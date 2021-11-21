#pragma once

#include <sstream>
#include <cmath>
#include <numbers>

// Converted from work here: https://github.com/datenwolf/linmath.h/blob/master/linmath.h
namespace bolt::vector_math
{
   template <typename T>
   T radiansToDegrees(T radians) { return (radians * 180 / std::numbers::pi_v<T>); }

   template <typename T>
   T degreesToRadians(T degrees) { return (degrees * std::numbers::pi_v<T>)/ static_cast<T>(180); }

   template <typename T>
   T radiansFromRatio(T x, T y) { return static_cast<T> (std::atan2(y, x)); };

   // NOTE: Next function was void linmath_vec4_set(...)
   template <typename T, std::size_t N>
   void vecSet(T(&v)[N], const T& value) {
      int i;
      for (i = 0; i < N; i++) {
         v[i] = value;
      }
   }

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


}  // End namespace bolt::vector_math


