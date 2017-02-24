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

#ifndef XMPMETA_BASE64_H_
#define XMPMETA_BASE64_H_

#include <string>

#include "base/port.h"

namespace xmpmeta {
// Decodes the base64-encoded input range. Supports decoding of both web-safe
// and regular base64."Web-safe" base-64 replaces + with - and / with _, and
// omits trailing = padding characters.
bool DecodeBase64(const string& data, string* output);

// Base64-encodes the given string.
bool EncodeBase64(const string& data, string* output);

}  // namespace meta

#endif  // XMPMETA_BASE64_H_
