// Copyright 2016 The XMPMeta Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef XMPMETA_EXTERNAL_STRINGS_NUMBERS_H_
#define XMPMETA_EXTERNAL_STRINGS_NUMBERS_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <functional>
#include <limits>
#include <string>

#include "base/integral_types.h"
#include "base/port.h"
#include "strings/ascii_ctype.h"

namespace xmpmeta {

// Convert strings to numeric values, with strict error checking.
// Leading and trailing spaces are allowed.
// Negative inputs are not allowed for unsigned ints (unlike strtoul).
//
// Base must be [0, 2-36].
// Base 0:
//   auto-select base from first two chars:
//    "0x" -> hex
//    "0" -> octal
//    else -> decimal
// Base 16:
//   Number can start with "0x"
//
// On error, returns false, and sets *value to:
//   std::numeric_limits<T>::max() on overflow
//   std::numeric_limits<T>::min() on underflow
//   conversion of leading substring if available ("123@@@" -> 123)
//   0 if no leading substring available
// The effect on errno is unspecified.
// Do not depend on testing errno.
bool safe_strto32_base(const string& text, int32* value, int base);
bool safe_strto64_base(const string& text, int64* value, int base);
bool safe_strtou32_base(const string& text, uint32* value, int base);
bool safe_strtou64_base(const string& text, uint64* value, int base);
bool safe_strtosize_t_base(const string& text, size_t* value, int base);

// Convenience functions with base == 10.
inline bool safe_strto32(const string& text, int32* value) {
  return safe_strto32_base(text, value, 10);
}

inline bool safe_strto64(const string& text, int64* value) {
  return safe_strto64_base(text, value, 10);
}

inline bool safe_strtou32(const string& text, uint32* value) {
  return safe_strtou32_base(text, value, 10);
}

inline bool safe_strtou64(const string& text, uint64* value) {
  return safe_strtou64_base(text, value, 10);
}

// Convert strings to floating point values.
// Leading and trailing spaces are allowed.
// Values may be rounded on over- and underflow.
bool safe_strtof(const string& str, float* value);

bool safe_strtod(const string& str, double* value);

// Previously documented minimums -- the buffers provided must be at least this
// long, though these numbers are subject to change:
//     Int32, UInt32:                   12 bytes
//     Int64, UInt64, Int, Uint:        22 bytes
//     Time:                            30 bytes
// Use kFastToBufferSize rather than hardcoding constants.
static const int kFastToBufferSize = 32;

// ----------------------------------------------------------------------
// FastInt32ToBufferLeft()
// FastUInt32ToBufferLeft()
// FastInt64ToBufferLeft()
// FastUInt64ToBufferLeft()
//
// Like the Fast*ToBuffer() functions above, these are intended for speed.
// Unlike the Fast*ToBuffer() functions, however, these functions write
// their output to the beginning of the buffer (hence the name, as the
// output is left-aligned).  The caller is responsible for ensuring that
// the buffer has enough space to hold the output.
//
// Returns a pointer to the end of the string (i.e. the null character
// terminating the string).
// ----------------------------------------------------------------------

char* FastInt32ToBufferLeft(int32 i, char* buffer);    // at least 12 bytes
char* FastUInt32ToBufferLeft(uint32 i, char* buffer);  // at least 12 bytes
char* FastInt64ToBufferLeft(int64 i, char* buffer);    // at least 22 bytes
char* FastUInt64ToBufferLeft(uint64 i, char* buffer);  // at least 22 bytes

// SimpleAtoi converts a string to an integer.
// Uses safe_strto?() for actual parsing, so strict checking is
// applied, which is to say, the string must be a base-10 integer, optionally
// followed or preceded by whitespace, and value has to be in the range of
// the corresponding integer type.
//
// Returns true if parsing was successful.
template <typename int_type>
MUST_USE_RESULT bool SimpleAtoi(const string& s, int_type* out) {
  static_assert(sizeof(*out) == 4 || sizeof(*out) == 8,
                "SimpleAtoi works only with 32-bit or 64-bit integers.");
  static_assert(!std::is_floating_point<int_type>::value,
                "Use safe_strtof or safe_strtod instead.");
  bool parsed;
  // TODO(jorg): This signed-ness check is used because it works correctly
  // with enums, and it also serves to check that int_type is not a pointer.
  // If one day something like std::is_signed<enum E> works, switch to it.
  if (static_cast<int_type>(0) > -1) {  // Signed
    if (sizeof(*out) == 64 / 8) {  // 64-bit
      int64 val;
      parsed = safe_strto64(s, &val);
      *out = static_cast<int_type>(val);
    } else {  // 32-bit
      int32 val;
      parsed = safe_strto32(s, &val);
      *out = static_cast<int_type>(val);
    }
  } else {  // Unsigned
    if (sizeof(*out) == 64 / 8) {  // 64-bit
      uint64 val;
      parsed = safe_strtou64(s, &val);
      *out = static_cast<int_type>(val);
    } else {  // 32-bit
      uint32 val;
      parsed = safe_strtou32(s, &val);
      *out = static_cast<int_type>(val);
    }
  }
  return parsed;
}

// ----------------------------------------------------------------------
// SimpleFtoa()
//    Description: converts a double or float to a string which, if passed to
//    strtod() or strtof() respectively, will produce the exact same original
//    double or float.  Exception: for NaN values, strtod(SimpleDtoa(NaN)) or
//    strtof(SimpleFtoa(NaN)) may produce any NaN value, not necessarily the
//    exact same original NaN value.
//
//    The output string is not guaranteed to be as short as possible.
//
//    The output string, including terminating NUL, will have length
//    less than or equal to kFastToBufferSize defined above.  Of course,
//    we would prefer that your code not depend on this property of
//    the output string.  This guarantee derives from a similar guarantee
//    from the previous generation of char-buffer-based functions.
//    We had to carry it forward to preserve compatibility.
// ----------------------------------------------------------------------
string SimpleFtoa(float value);

// ----------------------------------------------------------------------
// SimpleItoa()
//    Description: converts an integer to a string.
//    Faster than printf("%d").
//
//    Return value: string
// ----------------------------------------------------------------------
inline string SimpleItoa(int32 i) {
  char buf[16];  // Longest is -2147483648
  return string(buf, FastInt32ToBufferLeft(i, buf));
}

// We need this overload because otherwise SimpleItoa(5U) wouldn't compile.
inline string SimpleItoa(uint32 i) {
  char buf[16];  // Longest is 4294967295
  return string(buf, FastUInt32ToBufferLeft(i, buf));
}

inline string SimpleItoa(int64 i) {
  char buf[32];  // Longest is -9223372036854775808
  return string(buf, FastInt64ToBufferLeft(i, buf));
}

// We need this overload because otherwise SimpleItoa(5ULL) wouldn't compile.
inline string SimpleItoa(uint64 i) {
  char buf[32];  // Longest is 18446744073709551615
  return string(buf, FastUInt64ToBufferLeft(i, buf));
}

inline string SimpleItoa(long i) {  // NOLINT long is OK here
  if (sizeof(i) == 64 / 8) {
    return SimpleItoa(static_cast<int64>(i));
  } else if (sizeof(i) == 32 / 8) {
    return SimpleItoa(static_cast<int32>(i));
  }
}

inline string SimpleItoa(unsigned long i) {  // NOLINT long is OK here
  if (sizeof(i) == 64 / 8) {
    return SimpleItoa(static_cast<uint64>(i));
  } else if (sizeof(i) == 32 / 8) {
    return SimpleItoa(static_cast<uint32>(i));
  }
}

// Required buffer size for FloatToBuffer is kFastToBufferSize.
char* FloatToBuffer(float i, char* buffer);

}  // namespace xmpmeta

#endif  // XMPMETA_EXTERNAL_STRINGS_NUMBERS_H_
