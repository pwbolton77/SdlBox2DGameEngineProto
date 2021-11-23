#pragma once

//// GLU Matrix and Vector lib
// Manual: http://glm.g-truc.net/0.9.5/glm-0.9.5.pdf
// http://glm.g-truc.net/0.9.5/code.html
//
// Example #1: Usage with opengl ...
//   {
//      buf::Vec4 v(0.0f);
//      buf::Mat4 m(1.0f);
//      ...
//      glVertex3fv(&v[0]);
//      glLoadMatrixfv(&m[0][0]);
//   }

// Example #2: Transform 
//    {
//       glm::vec4 Position = glm::vec4(glm::vec3(0.0f), 1.0f);
//       glm::mat4 Model = glm::translate( glm::mat4(1.0f), glm::vec3(1.0f) );
//       glm::vec4 Transformed = Model * Position;
//    }

// Example #3: Using buf namespace with transform and printing
//#include "bolt_buf_matrix_print.h"
//{
//   buf::Vec4 v(0.0f);
//   std::cout << v << std::endl;
//
//   buf::Vec2 v2d(1.0f);
//   dbgln(v2d);
//
//   buf::Vec4 position = buf::Vec4(buf::Vec3(3.0f), 1.0f);
//   dbgln(position);
//
//   buf::Mat4 model = buf::translate(buf::Mat4(1.0f), buf::Vec3(1.0f));
//   dbgln(model);
//   std::cout << "model: " << buf::matrix_print::printMatrixAsLines(model);
//
//   buf::Vec4 transformed = model * position;
//   dbgln(transformed);
//}

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4, glm::ivec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptrvoid 

#include <string>
#include <ostream>
#include <sstream>
#include <cassert>

// Namespace for buf (Bolt Utilities Functionality)
namespace buf
{
   using namespace glm; // Bring all of glm into buf.  Note: Maybe not the best practice but it is convenient to just use buf::translate(...) etc.

   using Vec2 = glm::vec2;
   using Vec3 = glm::vec3;
   using Vec4 = glm::vec4;

   using Mat2 = glm::mat2;
   using Mat3 = glm::mat3;
   using Mat4 = glm::mat4;

   // Infrequently used for casting ... 
   using Vec2fArray = float[2];  // infrequently used for casting
   using Vec3fArray = float[3];  // infrequently used for casting
   using Vec4fArray = float[4];  // infrequently used for casting

   using Mat4x4fArray = float[16];  // infrequently used for casting
}


