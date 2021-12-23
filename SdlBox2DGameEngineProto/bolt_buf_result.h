#pragma once

#include "expected.h"

// buf: Namespace for Bolton Utility Functions
namespace buf
{
   // Bring the following into the buf namespace
   using tl::expected;
   using tl::unexpected;

   //// Result ////
   // Result is a frequently used result type that uses a string to return an error.
   //    Usage:
   //       - To initialize or set to error use:
   //             using namespace std::string_literals;
   //             Result res {buf::unexpected("something went wrong!"s)};
   //
   template <typename T>
   using Result = buf::expected<T, std::string>;
}

