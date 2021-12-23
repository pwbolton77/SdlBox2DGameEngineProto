#pragma once
// Purpose: The 2D game engine.  
//    pwb 11/25/2021
//    NOTE: This class consists of only static functions because (OpenGL) Glut libraries/framework 
//          does not support callbacks to objects (callbacks must be registered as static member or free functions).
//

#include "bolt_buf.h"
#include "ContactListener.h"
#include <tuple>
#include <span>

#include <Box2D/Box2D.h>

namespace bolt::game_engine
{
   class Engine
   {
   public:
      enum class ScreenMode { None, FullScreen, NonFullScreen };

      // Configure the engine before starting it
      static buf::Result<void> configureEngine(ScreenMode screen_mode = ScreenMode::NonFullScreen);
      // Start running the game engine 
      static buf::Result<void> runEngine();
      // Get the result of configuration
      static buf::Result<void> getConfigureResult() { return config_result; };
      // Return the nominal upper and lower world coordinates displayed on screen in meters. The term 
      // nominal means the (default) display area for which the engine was designed to show the player. This 
      // may vary from what is actually being displayed. Returned as <x_min, y_min, x_max, y_max>
      static std::tuple<float, float, float, float> getWorldDisplayedInMetersNominal() { return { 0.0f, 0.0f, x_world_display_max_nominal, y_world_display_max_nominal }; };

   private:
      static ScreenMode screen_mode;   // Full screen mode or not

      static constexpr int ScreenFramesPerSecond = 60;

      static constexpr float pixels_per_meter_nominal = 100.0f;	                  // Pixels per meter 
      static constexpr float meters_per_pixel_nominal = 1.0f / pixels_per_meter_nominal;   // Meters per pixel

      static constexpr std::int16_t screen_width_default = 1280;    // In pixels
      static constexpr std::int16_t screen_height_default = 1024;    // In pixels

      static std::int16_t window_width;   // Current window in pixels
      static std::int16_t window_height;  // Current window in pixels

      static constexpr float x_world_display_max_nominal = meters_per_pixel_nominal * screen_width_default;
      static constexpr float y_world_display_max_nominal = meters_per_pixel_nominal * screen_height_default;

      // Show less of the world as the window gets smaller ...
      static float x_world_display_max;
      static float y_world_display_max;

      // Return number of meters in physicals world, given pixels
      static float pixelsToMetersNominal(std::int16_t pixels) { return meters_per_pixel_nominal * pixels; }

      // Convert screen coordinates (top-left) to world coordinates (bottom-left and scaled).
      static float screenToWorldScaledX(std::int16_t x_screen) { return pixelsToMetersNominal(x_screen); }
      static float screenToWorldScaledY(std::int16_t y_screen) { return pixelsToMetersNominal(window_height - y_screen); };

      static std::pair<float, float> screenToWorldScaled(std::int16_t x_screen, std::int16_t y_screen) { return { screenToWorldScaledX(x_screen), screenToWorldScaledY(y_screen) }; };

      // Unscaled - Convert screen coordinates (top-left) to world coordinates (bottom-left and NON-scaled).
      static std::pair<std::int16_t, std::int16_t> screenToWorldUnscaled(std::int16_t  x_screen, std::int16_t y_screen) { return { x_screen, window_height - y_screen }; };

      // Configure the graphics 
      static buf::Result<void> configureGraphics(ScreenMode screen_mode);

      // Add a new polygon to the (Box2D) world of object.
      static b2Body* addPolyToWorld(float x_center_world, float y_center_world, const std::vector<buf::Vec2>& verts, bool dynamic_object);
      // Add a new rectangle to the (Box2D) world of object.
      static b2Body* addRectToWorld(float x, float y, float width, float height, bool dynamic_object);
      // Draw a polygon
      static void drawPoly(const std::span<buf::Vec2>& points, b2Vec2 center, float angle);
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
      static buf::Result<void> config_result;
   };
} // End namespace bolt::engine

