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

#include "xmpmeta/md5.h"

#include <sstream>
#include <string>

#include "gtest/gtest.h"

namespace xmpmeta {
namespace {

const int kExpectedHashLength = 32;

TEST(MD5, MD5Hash) {
  std::ostringstream data_stream;
  // Make a very large payload.
  for (int i = 0; i < 100000; i++) {
    for (int j = 0xff; j >= 0; j--) {
      data_stream.put(static_cast<char>(j));
    }
  }

  const string data = data_stream.str();
  string expected("e0fd444298d27b86d6bd865780dd71e0");
  string value =  MD5Hash(data);
  ASSERT_EQ(expected, value);
  ASSERT_EQ(kExpectedHashLength, value.length());
}

TEST(MD5, MD5HashNullString) {
  std::ostringstream data_stream;
  // Make a null payload.
  for (int i = 0; i < 100; i++) {
    data_stream.put(static_cast<char>(0));
  }

  const string data = data_stream.str();
  string expected("6d0bb00954ceb7fbee436bb55a8397a9");
  string value =  MD5Hash(data);
  ASSERT_EQ(expected, value);
  ASSERT_EQ(kExpectedHashLength, value.length());
}

}  // namespace
}  // namespace xmpmeta
