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

#include <string>
#include "gtest/gtest.h"

#ifndef XMPMETA_TEST_UTIL_H_
#define XMPMETA_TEST_UTIL_H_

namespace xmpmeta {

// Construct a fully qualified path for the test file depending on the
// local build/testing environment.
std::string TestFileAbsolutePath(const std::string& filename);

// Construct a fully qualified path for a temporary file depending on the
// local build/testing environment.
std::string TempFileAbsolutePath(const std::string& filename);

}  // namespace xmpmeta

#endif  // XMPMETA_TEST_UTIL_H_
