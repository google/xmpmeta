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

#include "xmpmeta/base64.h"

#include "strings/escaping.h"

namespace xmpmeta {

// Decodes the base64-encoded input range.
bool DecodeBase64(const string& data, string* output) {
  // Support decoding of both web-safe and regular base64.
  // "Web-safe" base-64 replaces + with - and / with _, and omits
  // trailing = padding characters.
  if (strings::Base64Unescape(data, output)) {
    return true;
  }
  return strings::WebSafeBase64Unescape(data, output);
}

// Base64-encodes the given data.
bool EncodeBase64(const string& data, string* output) {
  strings::Base64Escape(reinterpret_cast<const uint8*>(data.c_str()),
                        data.length(), output, false);
  return output->length() > 0;
}

// Base64-encodes the given int array.
bool EncodeIntArrayBase64(const std::vector<int>& data, string* output) {
  string bytes;
  for (const auto& item : data) {
    char buffer[sizeof(item)];
    memcpy(buffer, &item, sizeof(item));
    bytes.append(buffer, sizeof(buffer));
  }
  return EncodeBase64(bytes, output);
}

// Base64-decodes the given base64-encoded string.
bool DecodeIntArrayBase64(const string& data, std::vector<int>& output) {
  string bytes;
  if (!DecodeBase64(data, &bytes)) {
    return false;
  }
  const int count = bytes.size() / sizeof(int);
  output.clear();
  for (int i = 0; i < count; ++i) {
    const int result = *reinterpret_cast<const int*>(
        bytes.substr(i * sizeof(result), sizeof(result)).c_str());
    output.push_back(result);
  }
  return !output.empty();
}

// Base64-encodes the given float array.
bool EncodeFloatArrayBase64(const std::vector<float>& data, string* output) {
  string bytes;
  for (const auto& item : data) {
    char buffer[sizeof(item)];
    memcpy(buffer, &item, sizeof(item));
    bytes.append(buffer, sizeof(buffer));
  }
  return EncodeBase64(bytes, output);
}

// Base64-decodes the given base64-encoded string.
bool DecodeFloatArrayBase64(const string& data, std::vector<float>& output) {
  string bytes;
  if (!DecodeBase64(data, &bytes)) {
    return false;
  }
  const int count = bytes.size() / sizeof(float);
  output.clear();
  for (int i = 0; i < count; ++i) {
    const float result = *reinterpret_cast<const float*>(
        bytes.substr(i * sizeof(result), sizeof(result)).c_str());
    output.push_back(result);
  }
  return !output.empty();
}

}  // namespace xmpmeta
