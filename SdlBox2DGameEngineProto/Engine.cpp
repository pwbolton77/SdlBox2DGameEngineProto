// Site References ...
// My custom install process for libraries: https://github.com/pwbolton77/SdlBox2DInstallInstructions
// Box2D Core Concepts: https://box2d.org/documentation/
// FreeGlut Manual: http://freeglut.sourceforge.net/docs/api.php
// Glut manual: https://www.opengl.org/resources/libraries/glut/spec3/node113.html
// Collision callback tutorial: https://www.youtube.com/watch?v=34suqmxL-ts&ab_channel=thecplusplusguy
// Complex shapes tutorial (for next iteration): https://www.youtube.com/watch?v=V95dzuDw0Jg&ab_channel=TheCodingTrain
// Making Box2D Circles (for some other iteration): https://stackoverflow.com/questions/10264012/how-to-create-circles-in-box2d
//
//
// Debugging: To print out the model view matrix
//    GLfloat model_view_matrix[16];
//    glGetFloatv(GL_MODELVIEW_MATRIX, model_view_matrix);
//    std::cout << "model: " << printMatrixAsLines(model_view_matrix) << std::endl;

#include "ContactListener.h"  // #1 We need custom class for collision callbacks
#include "bolt_buf.h"
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

#include "bolt_util_debug_macros.h" // Should be last include and ONLY in *.cpp files

using namespace std::string_literals;

#undef TEST_GET_MODEL_VIEW_MATRIX // Define this to test and print model view matrix after some simple manipulation 


namespace bolt::game_engine
{
   using namespace buf;
   Engine::ScreenMode Engine::screen_mode = Engine::ScreenMode::None;

   std::int16_t Engine::window_width = screen_width_default;    // Current window in pixels
   std::int16_t Engine::window_height = screen_height_default;  // Current window in pixels

   // Show less of the world as the window gets smaller ...
   float Engine::x_world_display_max = (x_world_display_max_nominal * window_width) / screen_width_default;
   float Engine::y_world_display_max = (y_world_display_max_nominal * window_height) / screen_height_default;

   ContactListener Engine::contact_listener{};   // #1 Only need ONE instance of the contact listener to receive all collision callbacks
   b2World* Engine::world{ nullptr };                     // The Box2D world of objects

   // Record the results of a configuration attempt. Contains an error string if not configured 
   Result<void> Engine::config_result{ buf::unexpected("There was no attempt to configure the engine."s) };

   // #1 The tutorial used void pointers to "user data" with user data being int's.  So we need a couple ints to point to, where
   //    the value of the int indicates our custom type - i.e. it is either a "static" box (the platform where boxes land),
   //    or it is a "dynamic" box that we may want to delete.
   // const int Engine::StaticType = 0;     // We make either static boxes
   // const int Engine::DynamicType = 1;    // or dynamic boxes

      // Purpose: Configure the graphics 
   Result<void> Engine::configureGraphics(ScreenMode _screen_mode)
   {
      Result<void> result; // Defaults to successful result

      assert(_screen_mode != ScreenMode::None);
      screen_mode = _screen_mode;

      int dummy_command_lines_args = 0;
      glutInit(&dummy_command_lines_args, nullptr); // Init with no arguments: *OR* you can pass in command line arguments via "glutInit(&argc, args);"

      // Using glut to init window:  https://stackoverflow.com/questions/15891856/full-screen-in-opengl/15893255
      // glutGameModeString("1024x768:32@75"); <--- From some other example
      glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // enabling double buffering and RGBA

      if (screen_mode == ScreenMode::FullScreen)
      {
         // glutGameModeString("1024x768:32@75"); <--- From some other example
         auto resolution_str = std::format("{}x{}", screen_width_default, screen_height_default);
         glutGameModeString(resolution_str.c_str());
         glutEnterGameMode(); // set glut to full screen using the settings in the line above
      }
      else
      {
         glutInitWindowSize((int)screen_width_default, (int)screen_height_default);
         glutCreateWindow("Bolt Game Engine"); // creating the window
      }

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

      return result;
   }

   // Purpose: Configure the engine before starting it
   Result<void> Engine::configureEngine(ScreenMode _screen_mode)
   {
      // Configure the graphics.  If there is an err, return the (error) result
      if (config_result = configureGraphics(_screen_mode); !config_result)
         return config_result;

      // Initialize the Box2D world and create/place the static objects.
      initBox2DWorld();

      return config_result;
   }

   // Purpose: Start running the game engine 
   Result<void> Engine::runEngine()
   {
      Result<void> result; // Initialize to non-error 

      // Setup a timer (in milliseconds), then call the runMainLoop() function. 
      //  - val is just a user provided value so the user can (potentially) identify the reason a timer when off
      glutTimerFunc(1000 / ScreenFramesPerSecond, runMainLoop, 0 /*val*/);

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
   b2Body* Engine::addRectToWorld(float x_center_world, float y_center_world, float width, float height, bool dynamic_object = true)
   {
      b2BodyDef bodydef;
      bodydef.position.Set(x_center_world, y_center_world);
      bodydef.type = (dynamic_object) ? b2_dynamicBody : b2_staticBody;

      b2Body* body = world->CreateBody(&bodydef);

      b2PolygonShape shape;   // Polygons seem to be limited to vertices 
      shape.SetAsBox(width / 2, height / 2);

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
      glTranslatef(center.x, center.y, 0.0f);
      glRotatef(angle * 180.0f / (float)M_PI, 0.0f, 0.0f, 1.0f);
      glBegin(GL_QUADS);
      for (int i = 0; i < 4; ++i)
         glVertex2f(points[i].x, points[i].y);

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
      // world = new b2World(b2Vec2(0.0f, 0.0f)); // 0, 0 to removed all gravity: Was (0.0f, 9.81f) for gravity
      world = new b2World(b2Vec2(0.0f, -9.8f));

      world->SetContactListener(&contact_listener);

      // Add a static platform where boxes will land and stop.
      const auto& [world_x, world_y] = screenToWorldScaled(screen_width_default / 2, 50);

      addRectToWorld(x_world_display_max_nominal / 2.0f, 0.8f, 10.0f, 0.4f, false /*not dynamic, so static*/);
   }

   // Purpose: Sets up an orthographic view.  
   //    Note: Needs to be installed as a glutReshapeFunc() callback to support Window's window management
   void Engine::reshapeOrtho(int win_width, int win_height) 
   {
      dbgfuncln(win_width);
      dbgfuncln(win_height);
      window_width = win_width;
      window_height = win_height;
      glViewport(0, 0, (GLsizei)win_width, (GLsizei)win_height); //set the viewport to the current window specifications
      glMatrixMode(GL_PROJECTION); //set the matrix to projection

      glLoadIdentity();

      // Show less of the world as the window gets smaller ...
      x_world_display_max = (x_world_display_max_nominal * window_width) / screen_width_default;
      y_world_display_max = (y_world_display_max_nominal * window_height) / screen_height_default;

      // glOrtho(0.0, x_world_display_max_nominal, 0.0, y_world_display_max_nominal, 1.0, -1.0);
      glOrtho(0.0, x_world_display_max, 0.0, y_world_display_max, 1.0, -1.0);

      glMatrixMode(GL_MODELVIEW); //set the matrix back to model
   }

   // Purpose: Update the position of objects/bodies in the world
   void Engine::update()
   {
      world->Step(1.0f / ScreenFramesPerSecond /*amount of time that passed*/,
         5 /*magic number*/, 5 /*magic number*/);  // I guess these numbers affect accuracy and overhead of collision detection and position calculations.
   }

   // Purpose: Run the main render loop
   void Engine::runMainLoop(int val)
   {
      update();   // Update the position of objects/bodies in the world

      render();   // Render the next display/frame (and swap to the newly drawn frame)

      // Setup a timer (in milliseconds), then call the runMainLoop() function again. 
      //  val - is just a user provided value so the user can (potentially) identify the reason a timer when off
      glutTimerFunc(1000 / ScreenFramesPerSecond, runMainLoop, val); //Run frame one more time
   }

   // Purpose: Callback when a mouse event occurs (assuming it was registered with glutMouseFunc())
   void Engine::mouseEventCallback(int button, int state, int screen_x, int screen_y)
   {
      if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
      {
         const auto& [world_x, world_y] = screenToWorldScaled(screen_x, screen_y);
         addRectToWorld(world_x, world_y, 0.4f, 0.4f, true /*not dynamic, so static*/);
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
#ifdef PRINT_WHERE_MOUSE_IS
      dbg(__func__); dbg(where_mouse_is_x); dbg(where_mouse_is_y); dbgln(key);   // Debug: Just print out the key

      const auto& [world_mouse_x, world_mouse_y] = screenToWorldScaled(where_mouse_is_x, where_mouse_is_y);
      dbg(__func__); dbg(world_mouse_x); dbg(world_mouse_y); dbgln(key);   // Debug: Just print out the key

      const auto& [mouse_x_from_bot_left, mouse_y_from_bot_left] = screenToWorldUnscaled(where_mouse_is_x, where_mouse_is_y);
      dbg(__func__); dbg(mouse_x_from_bot_left); dbg(mouse_y_from_bot_left); dbgln(key);   // Debug: Just print out the key
#endif

      if (key == 27)
      {
         glutLeaveGameMode(); //set the resolution how it was
         exit(0); //quit the program
      }
   }

} // End namespace bolt::engine
