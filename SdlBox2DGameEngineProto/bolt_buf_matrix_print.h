#pragma once

#include "bolt_buf_matrix.h"

#include <string>
#include <ostream>
#include <sstream>
#include <cassert>

// buf: Namespace for Bolton Utility Functions
namespace buf::matrix_print
{
   template <typename T, std::size_t N>
   std::string printVec(const T(&v)[N])
   {
      std::ostringstream ss;
      ss << "[";

      for (std::size_t index = 0; index < N - 1; ++index)
         ss << v[index] << ", ";

      ss << v[N - 1] << "]";
      return ss.str();
   }

   template <typename T, std::size_t N>
   std::string printMatrix(const T(&v)[N])
   {
      std::size_t cols = static_cast<std::size_t>(std::sqrt(N));
      std::size_t rows = cols;
      if (rows * cols != N)   // Does not appear to be a square matrix, so exit
         return "[[<format error>]]";

      std::ostringstream ss;
      ss << "[";

      for (std::size_t row = 0; row < rows; ++row)
      {
         ss << "[";
         std::size_t col = 0;
         for (; col < cols - 1; ++col)
            ss << v[row * cols + col] << ", ";

         ss << v[row * cols + col] << "]";
      }

      ss << "]";
      return ss.str();
   }

   template <typename T, std::size_t N>
   std::string printMatrixAsLines(const T(&v)[N])
   {
      std::size_t cols = static_cast<std::size_t>(std::sqrt(N));
      std::size_t rows = cols;
      if (rows * cols != N)   // Does not appear to be a square matrix, so exit
         return "[[<format error>]]";

      std::ostringstream ss;
      ss << "\n";

      for (std::size_t row = 0; row < rows; ++row)
      {
         ss << "[";
         std::size_t col = 0;
         for (; col < cols - 1; ++col)
            ss << v[row * cols + col] << ", ";

         ss << v[row * cols + col] << "]\n";
      }

      return ss.str();
   }

   //// ostream operators
   inline std::ostream& operator<<(std::ostream& os, const buf::Vec4& vec)
   {
      os << printVec((const buf::Vec4fArray&)vec);
      return os;
   };

   inline std::ostream& operator<<(std::ostream& os, const buf::Vec3& vec)
   {
      os << printVec((const buf::Vec3fArray&)vec);
      return os;
   };

   inline std::ostream& operator<<(std::ostream& os, const buf::Vec2& vec)
   {
      os << printVec((const buf::Vec2fArray&)vec);
      return os;
   };

   inline std::ostream& operator<<(std::ostream& os, const buf::Mat4& mat)
   {
      os << printMatrix((const buf::Mat4x4fArray&)mat); return os;
   };

   inline std::string printMatrixAsLines (const buf::Mat4& mat)
   {
      return printMatrixAsLines ((const buf::Mat4x4fArray&)mat);
   }
}


