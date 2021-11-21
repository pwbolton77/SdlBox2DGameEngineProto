#pragma once

#include "expected.h"
#include "vector_math.h"

#include <string>

// Alias bvec so it is the same as the longer bolt::vector_math
namespace bvec = bolt::vector_math;

// namespace for bolt utilities files
namespace buf
{
   // bring the following into the buf namespace
   using tl::expected;  
   using tl::unexpected;
}


///////////////////////// Result ///////////////////
// Result is a frequently used result type that uses a string to return an error.
//    Usage:
//       - To initialize or set to error use:
//             using namespace std::string_literals;
//             Result res {buf::unexpected("something went wrong!"s)};

template <typename T>
using Result = buf::expected<T, std::string>;

