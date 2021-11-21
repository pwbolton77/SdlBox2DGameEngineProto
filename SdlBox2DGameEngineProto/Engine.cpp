
// Site References ...
// My custom install process for libraries: https://github.com/pwbolton77/SdlBox2DInstallInstructions
// Box2D Core Concepts: https://box2d.org/documentation/
// FreeGlut Manual: http://freeglut.sourceforge.net/docs/api.php
// Glut manual: https://www.opengl.org/resources/libraries/glut/spec3/node113.html
// Collision callback tutorial: https://www.youtube.com/watch?v=34suqmxL-ts&ab_channel=thecplusplusguy
// Complex shapes tutorial (for next iteration): https://www.youtube.com/watch?v=V95dzuDw0Jg&ab_channel=TheCodingTrain
// Making Box2D Circles (for some other iteration): https://stackoverflow.com/questions/10264012/how-to-create-circles-in-box2d

#include "ContactListener.h"  // #1 We need custom class for collision callbacks
#include "bolt_utilities.h"
#include "Engine.h"

#include <SDL.h>
#include <Box2D/Box2D.h>

#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <memory>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

using namespace std::string_literals;

#define dbg(x) std::cout << #x << ": " << (x) << "   "
#define dbgln(x) std::cout << #x << ": " << (x) << std::endl

#undef TEST_GET_MODEL_VIEW_MATRIX // Define this to test and print model view matrix after some simple manipulation 

// const int SCREEN_WIDTH = 640;
// const int SCREEN_HEIGHT = 480;

// "1280x1024"); //the settings for full screen mode
static const int SCREEN_WIDTH = 1280;
static const int SCREEN_HEIGHT = 1024;

static const int SCREEN_FRAMES_PER_SECOND = 60;
static const float MetersToPixels = 40.0f;		// Meters to pixels
static const float PixelsToMeters = 1.0f / MetersToPixels;	// Pixels to meters 


namespace bolt::game_engine
{

   ContactListener Engine::contact_listener{};   // #1 Only need ONE instance of the contact listener to receive all collision callbacks
   b2World* Engine::world{nullptr};                     // The Box2D world of objects

   // Record the results of a configuration attempt. Contains an error string if not configured 
   Result<void> Engine::config_result{ buf::unexpected("There was no attempt to configure the engine."s) };

   // #1 The tutorial used void pointers to "user data" with user data being int's.  So we need a couple ints to point to, where
   //    the value of the int indicates our custom type - i.e. it is either a "static" box (the platform where boxes land),
   //    or it is a "dynamic" box that we may want to delete.
   // const int Engine::StaticType = 0;     // We make either static boxes
   // const int Engine::DynamicType = 1;    // or dynamic boxes

      // Purpose: Configure the graphics 
   Result<void> Engine::configureGraphics(ScreenMode screen_mode)
   {
      Result<void> result; // Defaults to successful result

      int dummy_command_lines_args = 0;
      glutInit(&dummy_command_lines_args, nullptr); // Init with no arguments: *OR* you can pass in command line arguments via "glutInit(&argc, args);"
      glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH); //set the display to Double buffer, with depth

      //// Setup for full screen mode
      // glutGameModeString("1024x768:32@75"); <--- This was the example for the settings for full screen mode
      glutGameModeString("1280x1024"); // A compatible settings for full screen mode for many monitors
      glutEnterGameMode(); // set glut to full screen using the settings in the line above

      //// Initialize Modelview Matrix
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      //// Initialize clear color (i.e. clear the hidden buffer to black when we render the next display buffer)
      glClearColor(0.f, 0.f, 0.f, 1.f);   // RGB to black, with an Alpha of 1.0 so non-transparent

      //// Enable texturing
      glEnable(GL_TEXTURE_2D);   // Maybe useful later 

      //// Setup opengl blending
      glEnable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      //// Try to enable smoothing (does not work well)
      // glEnable(GL_LINE_SMOOTH);
      // glEnable(GL_POLYGON_SMOOTH);
      // glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      // glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

      //// TODO: @@@ Figure a better way to setup callbacks
      auto render_lamda = []() { render();  };
      glutDisplayFunc(render_lamda); //use the display function to draw everything
      glutIdleFunc(render_lamda); //update any variables in display, display can be changed to anything, as long as you move the variables to be updated, in this case, angle++;
      glutReshapeFunc(reshapeOrtho); //reshape the window accordingly

      glutMouseFunc(mouseEventCallback);
      glutKeyboardFunc(keyboardEventCallback);

      // Check for any errors
      GLenum error = glGetError();
      if (error != GL_NO_ERROR)
      {
         result = buf::unexpected{ std::format("Error initializing OpenGL! {}\n", (const char*)gluErrorString(error)) };
      }

#ifdef TEST_GET_MODEL_VIEW_MATRIX
      testGetModelViewMatrix();  // Test and print model view matrix after some simple manipulation 
#endif

      return result;
   }

   // Purpose: Configure the engine before starting it
   Result<void> Engine::configureEngine(ScreenMode screen_mode)
   {
      // Configure the graphics.  If there is an err, return the (error) result
      if (config_result = configureGraphics(screen_mode); !config_result)
         return config_result;

      // Initialize the Box2D world and create/place the static objects.
      initBox2DWorld();

      return config_result;
   }

   // Purpose: Start running the game engine 
   Result<void> Engine::startEngine()
   {
      Result<void> result; // Initialize to non-error 

      // Setup a timer (in milliseconds), then call the runMainLoop() function. 
      //  - val is just a user provided value so the user can (potentially) identify the reason a timer when off
      glutTimerFunc(1000 / SCREEN_FRAMES_PER_SECOND, runMainLoop, 0 /*val*/);

      // Run the world.
      glutMainLoop(); //Start GLUT main loop

      glutLeaveGameMode(); //set the resolution how it was
      SDL_Quit(); //Quit/cleanup SDL subsystems

      return result;
   }

   // Purpose: Add a new rectangle to the (Box2D) world of object.
   //   dynamic_object: 
   //       - if true the object bounce around in the physical world.  
   //       - If false the object is "static" and acts like a rigid, fixed platform (that probably never moves in the scene).
   b2Body* Engine::addRectToWorld(float x, float y, float width, float height, bool dynamic_object = true)
   {
      b2BodyDef bodydef;
      bodydef.position.Set(x * PixelsToMeters, y * PixelsToMeters);
      bodydef.type = (dynamic_object) ? b2_dynamicBody : b2_staticBody;

      b2Body* body = world->CreateBody(&bodydef);

      b2PolygonShape shape;   // Polygons seem to be limited to vertices 
      shape.SetAsBox(PixelsToMeters * width / 2, PixelsToMeters * height / 2);

      b2FixtureDef fixture_def;
      fixture_def.shape = &shape;   // Note: "shape" is specifically documented to state that it will be cloned, so can be on stack.
      fixture_def.density = 1.0;

      body->CreateFixture(&fixture_def);

      auto& user_data = body->GetUserData();
      assert(sizeof(uintptr_t) == sizeof(&DynamicType)); // Make sure that we can save a pointer to an int in a uintptr_t type.

      // #1 When a collision happens we need to know the type of the box/body, so save the type via a user data pointer
      if (dynamic_object)
         user_data.pointer = (uintptr_t)&DynamicType;
      else
         user_data.pointer = (uintptr_t)&StaticType;

      return body;
   }

   // Purpose: Draw a square. Assumes 4 vertex points using OpenGl
   void Engine::drawSquare(b2Vec2* points, b2Vec2 center, float angle)
   {
      glColor3f(1.0, 1.0f, 1.0f);
      glPushMatrix();
      glTranslatef(center.x * MetersToPixels, center.y * MetersToPixels, 0.0f);
      glRotatef(angle * 180.0f / (float)M_PI, 0.0f, 0.0f, 1.0f);
      glBegin(GL_QUADS);
      for (int i = 0; i < 4; ++i)
         glVertex2f(points[i].x * MetersToPixels, points[i].y * MetersToPixels);

      glEnd();
      glPopMatrix();
   }

   // Purpose: Render the graphics to hidden display buffer, and then swap buffers to show the new display
   void Engine::render()
   {
      glClear(GL_COLOR_BUFFER_BIT); // Clear the hidden (color) buffer with the glClearColor() we setup at initGL() 

      b2Body* body_node_ptr = world->GetBodyList(); // Get the head of list of bodies in the Box2D world


      while (body_node_ptr != nullptr)
      {
         b2Vec2 points[4]; // Assumes a (4 vertex) rectangle!

         // Get points assuming a rectangle 
         for (int i = 0; i < 4; ++i)
            points[i] = ((b2PolygonShape*)body_node_ptr->GetFixtureList()->GetShape())->m_vertices[i];

         drawSquare(points, body_node_ptr->GetWorldCenter(), body_node_ptr->GetAngle());

         body_node_ptr = body_node_ptr->GetNext(); // Get the next body in the world
      }

      glutSwapBuffers();   // Swap the hidden buffer with the old to show the new display buffer
   }

   // Purpose: Initialize the Box2D world and create/place the static objects.
   void Engine::initBox2DWorld()
   {
      // world = new b2World(b2Vec2(0.0f, 0.0f)); // Removed all gravity: Was (0.0f, 9.81f) for gravity
      world = new b2World(b2Vec2(0.0f, 9.8f)); // Removed all gravity: Was (0.0f, 9.81f) for gravity

      world->SetContactListener(&contact_listener);

      // Add a static platform where boxes will land and stop.
      addRectToWorld(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, SCREEN_WIDTH, 30, false /*not dynamic, so static*/);
   }

   // Purpose: Sets up an orthographic view.  
   //    Note: Needs to be installed as a glutReshapeFunc() callback to support Window's window management
   void Engine::reshapeOrtho(int w, int h) {
      glViewport(0, 0, (GLsizei)w, (GLsizei)h); //set the viewport to the current window specifications
      glMatrixMode(GL_PROJECTION); //set the matrix to projection

      glLoadIdentity();

      glOrtho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, 1.0, -1.0);
      // TODO:  
      //    - @@ This will flip screen coord so origin is in bottom left: glOrtho(0.0, SCREEN_WIDTH, 0.0, SCREEN_HEIGHT, 1.0, -1.0);  // @@@ Temp
      //    - @@ But doing so means we need to translate mouse hit coord to world coords a little differently
      //    - @@ And gravity will have to be reversed 
      glMatrixMode(GL_MODELVIEW); //set the matrix back to model
   }

   // Purpose: Update the position of objects/bodies in the world
   void Engine::update()
   {
      world->Step(1.0f / SCREEN_FRAMES_PER_SECOND /*amount of time that passed*/,
         5 /*magic number*/, 5 /*magic number*/);  // I guess these numbers affect accuracy and overhead of collision detection and position calculations.
   }

   // Purpose: Run the main render loop
   void Engine::runMainLoop(int val)
   {
      update();   // Update the position of objects/bodies in the world

      render();   // Render the next display/frame (and swap to the newly drawn frame)

      // Setup a timer (in milliseconds), then call the runMainLoop() function again. 
      //  val - is just a user provided value so the user can (potentially) identify the reason a timer when off
      glutTimerFunc(1000 / SCREEN_FRAMES_PER_SECOND, runMainLoop, val); //Run frame one more time
   }

   // Purpose: Callback when a mouse event occurs (assuming it was registered with glutMouseFunc())
   void Engine::mouseEventCallback(int button, int state, int screen_x, int screen_y)
   {
      if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
      {
         addRectToWorld((float)screen_x, (float)screen_y, 20 /*width*/, 20 /*height*/, true);
      }

      // Other callbacks include
      //glutIdleFunc(animate);  // when there is nothing else to do
      //glutKeyboardFunc(something);    // ?
      //glutKeyboardUpFunc(keyboard_up); // when a key goes up 
      //glutPassiveMotionFunc(look);// when the mouse moved
      //glutMotionFunc(drag);// when the mouse drags around 
   }

   // Purpose: Callback when a key is pressed
   void Engine::keyboardEventCallback(unsigned char key, int where_mouse_is_x, int where_mouse_is_y)
   {
      dbg(__func__); dbg(where_mouse_is_x); dbg(where_mouse_is_y); dbgln(key);   // Debug: Just print out the key

      if (key == 27)
      {
         glutLeaveGameMode(); //set the resolution how it was
         exit(0); //quit the program
      }
   }


   // @@@@@@@@@@@@@@@@@@@@@

} // End namespace bolt::engine
