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

#include "xmpmeta/test_util.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "xmpmeta/file.h"

DECLARE_string(test_srcdir);
DECLARE_string(test_tmpdir);

// This macro is used to inject additional path information specific
// to the build system.

#ifndef XMPMETA_TEST_SRCDIR_SUFFIX
#define XMPMETA_TEST_SRCDIR_SUFFIX ""
#endif

namespace xmpmeta {

std::string TestFileAbsolutePath(const std::string& filename) {
  return JoinPath(FLAGS_test_srcdir + XMPMETA_TEST_SRCDIR_SUFFIX,
                  filename);
}

std::string TempFileAbsolutePath(const std::string& filename) {
  return JoinPath(FLAGS_test_tmpdir, filename);
}

}  // namespace xmpmeta
