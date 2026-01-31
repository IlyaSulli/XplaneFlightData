// JSF AV C++ Coding Standard - Type Definitions
// 
// AV Rule 209: The basic types of int, short, long, float and double shall 
// not be used, but specific-length equivalents should be typedef'd accordingly 
// for each compiler, and these type names used in the code.
// 
// This header provides portable, fixed-width type definitions compliant with
// the Joint Strike Fighter Air Vehicle C++ Coding Standards.
//
// AV Rule 50: The first word of a typedef will begin with an uppercase letter.
// AV Rule 126: Only C++ style comments (//) shall be used.

#ifndef JSF_TYPES_H
#define JSF_TYPES_H

#include <cstdint>

// Signed integer types (AV Rule 50: uppercase first letter)
typedef int8_t   Int8;
typedef int16_t  Int16;
typedef int32_t  Int32;
typedef int64_t  Int64;

// Unsigned integer types
typedef uint8_t  Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef uint64_t Uint64;

// Floating-point types
typedef float    Float32;
typedef double   Float64;

// Boolean type (built-in)
// bool is a built-in type in C++ and is allowed per AV Rule 209 Appendix A

// Character type (built-in)
// char is a built-in type in C++ and is allowed per AV Rule 209 Appendix A

#endif // JSF_TYPES_H
