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
//
////////////////////////////////////////////////////////////////////////////////
//
// Useful string functions and so forth.  This is a grab-bag file.
//
// You might also want to look at memutil.h, which holds mem*()
// equivalents of a lot of the str*() functions in string.h,
// eg memstr, mempbrk, etc.
//
// These functions work fine for UTF-8 strings as long as you can
// consider them to be just byte strings.  For example, due to the
// design of UTF-8 you do not need to worry about accidental matches,
// as long as all your inputs are valid UTF-8 (use \uHHHH, not \xHH or \oOOO).
//
// Caveats:
// * all the lengths in these routines refer to byte counts,
//   not character counts.
// * case-insensitivity in these routines assumes that all the letters
//   in question are in the range A-Z or a-z.
//
// If you need Unicode specific processing (for example being aware of
// Unicode character boundaries, or knowledge of Unicode casing rules,
// or various forms of equivalence and normalization), take a look at
// files in i18n/utf8.

#ifndef XMPMETA_EXTERNAL_STRINGS_UTIL_H_
#define XMPMETA_EXTERNAL_STRINGS_UTIL_H_

#include <string>

#include "base/port.h"
#include "fastmem.h"

namespace xmpmeta {

// Returns whether str begins with prefix.
inline bool HasPrefixString(const string& str,
                            const string& prefix) {
  return str.length() >= prefix.length() &&
      strings::memeq(&str[0], &prefix[0], prefix.length());
}

// Returns whether str ends with suffix.
inline bool HasSuffixString(const string& str,
                            const string& suffix) {
  return str.length() >= suffix.length() &&
      strings::memeq(&str[0] + (str.length() - suffix.length()),
                     &suffix[0], suffix.length());
}

}  // namespace xmpmeta

#endif  // XMPMETA_EXTERNAL_STRINGS_UTIL_H_
