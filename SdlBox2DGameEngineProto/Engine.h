#pragma once
// Purpose: The 2D game engine.  NOTE: All static function/data members (need to deal with Glut/OpenGL framework and callbacks)! 

#include "bolt_utilities.h"
#include "ContactListener.h"

#include <Box2D/Box2D.h>

namespace bolt::game_engine
{
   class Engine
   {
   public:
      enum class ScreenMode { FullScreen, NonFullScreen };

      // Configure the engine before starting it
      static Result<void> configureEngine(ScreenMode screen_mode = ScreenMode::FullScreen);
      // Start running the game engine 
      static Result<void> runEngine();
      // Get the result of configuration
      static Result<void> getConfigureResult() { return config_result; };

   private:
      // Configure the graphics 
      static Result<void> configureGraphics(ScreenMode screen_mode);

      // Add a new rectangle to the (Box2D) world of object.
      static b2Body* addRectToWorld(float x, float y, float width, float height, bool dynamic_object);
      // Draw a square. Assumes 4 vertex points using OpenGl
      static void drawSquare(b2Vec2* points, b2Vec2 center, float angle);
      // Render the graphics to hidden display buffer, and then swap buffers to show the new display
      static void render();
      // Initialize the Box2D world and create/place the static objects.
      static void initBox2DWorld();
      // Sets up an orthographic view.  
      static void reshapeOrtho(int w, int h);
      // Update the position of objects/bodies in the world
      static void update();
      // Run the main render loop
      static void runMainLoop(int val);

      ///////// Callbacks /////
      // Callback when a mouse event occurs (assuming it was registered with glutMouseFunc())
      static void mouseEventCallback(int button, int state, int screen_x, int screen_y);
      // Callback when a key is pressed
      static void keyboardEventCallback(unsigned char key, int where_mouse_is_x, int where_mouse_is_y);

      // @@@ TODO: Fix StaticType and DynamicType 
      // #1 The tutorial used void pointers to "user data" with user data being int's.  So we need a couple ints to point to, where
      //    the value of the int indicates our custom type - i.e. it is either a "static" box (the platform where boxes land),
      //    or it is a "dynamic" box that we may want to delete.
      // static const int StaticType = 0;     // We make either static boxes
      // static const int DynamicType = 1;    // or dynamic boxes

      static ContactListener contact_listener;   // #1 Only need ONE instance of the contact listener to receive all collision callbacks
      static b2World* world;                     // The Box2D world of objects

      // Record the results of a configuration attempt. Contains an error string if not (successfully) configured.
      static Result<void> config_result;
   };
} // End namespace bolt::engine

