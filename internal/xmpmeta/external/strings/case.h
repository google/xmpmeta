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

// This file contains string processing functions related to
// uppercase, lowercase, etc.
//
// These functions are for ASCII only. If you need to process UTF8 strings,
// take a look at files in i18n/utf8.

#ifndef XMPMETA_EXTERNAL_STRINGS_CASE_H_
#define XMPMETA_EXTERNAL_STRINGS_CASE_H_

#include <string>

#include "base/port.h"

namespace xmpmeta {

// Returns true if the two strings are equal, case-insensitively speaking.
// Uses C/POSIX locale.
inline bool StringCaseEqual(const string& s1, const string& s2) {
  return strcasecmp(s1.c_str(), s2.c_str()) == 0;
}

// ----------------------------------------------------------------------
// LowerString()
// LowerStringToBuf()
//    Convert the characters in "s" to lowercase.
//    Works only with ASCII strings; for UTF8, see ToLower in
//    util/utf8/public/unilib.h
//    Changes contents of "s".  LowerStringToBuf copies at most
//    "n" characters (including the terminating '\0')  from "s"
//    to another buffer.
// ----------------------------------------------------------------------
void LowerString(string* s);

namespace strings {
inline string ToLower(const string& s) {
  string out(s);
  LowerString(&out);
  return out;
}

}  // namespace strings
}  // namespace xmpmeta

#endif  // XMPMETA_EXTERNAL_STRINGS_CASE_H_
