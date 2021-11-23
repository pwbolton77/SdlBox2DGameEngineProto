// Purpose: The start of a 2d game engine that uses SDL, OpenGL, and Box2D for collision detection.
//    pwb 11/21/2001
// 
// Site References ...
// My custom install process for libraries: https://github.com/pwbolton77/SdlBox2DInstallInstructions
// Box2D Core Concepts: https://box2d.org/documentation/
// FreeGlut Manual: http://freeglut.sourceforge.net/docs/api.php
// Glut manual: https://www.opengl.org/resources/libraries/glut/spec3/node113.html
// Collision callback tutorial: https://www.youtube.com/watch?v=34suqmxL-ts&ab_channel=thecplusplusguy
// Complex shapes tutorial (for next iteration): https://www.youtube.com/watch?v=V95dzuDw0Jg&ab_channel=TheCodingTrain
// Making Box2D Circles (for some other iteration): https://stackoverflow.com/questions/10264012/how-to-create-circles-in-box2d

#include "Engine.h"

#include "bolt_util_debug_macros.h" // Should be last include and ONLY in *.cpp files

// #1 The tutorial used void pointers to "user data" with user data being int's.  So we need a couple ints to point to, where
//    the value of the int indicates our custom type - i.e. it is either a "static" box (the platform where boxes land),
//    or it is a "dynamic" box that we may want to delete.
const int StaticType = 0;     // We make either static boxes
const int DynamicType = 1;    // or dynamic boxes


// Purpose: The program's main()
int main(int argc, char* args[])
{
   namespace ben = bolt::game_engine;
   using Eng = ben::Engine;

   int error_code{ 0 }; // Return a non-zero error code from main() to indicate an error

   //// Configure the engine
   auto screen_mode = Eng::ScreenMode::NonFullScreen;
   auto startup_result = Eng::configureEngine(screen_mode);

   //// If config went okay
   if (startup_result)
   {
      // Provide some instructions to the player 
      std::cout << std::endl;
      std::cout << "Instructions:" << std::endl;
      std::cout << " - Click mouse in window to create a block that falls." << std::endl;
      std::cout << " - Press ESC to exit." << std::endl;

      //// Start running the engine
      // !! NOTE: This will not return until the engine exists
      startup_result = Eng::runEngine();
   }

   //// Handle errors
   if (!startup_result)
   {
      std::cerr << "Game engine failed: " << startup_result.error() << std::endl;
      error_code = 1;   // Return a (non-zero) error code from main, indicating an error
   }
   return error_code;
}
