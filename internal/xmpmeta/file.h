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

#ifndef XMPMETA_FILE_H_
#define XMPMETA_FILE_H_

#include <string>

namespace xmpmeta {

void WriteStringToFileOrDie(const std::string &data,
                            const std::string &filename);
void ReadFileToStringOrDie(const std::string &filename, std::string *data);

// Join two path components, adding a slash if necessary.  If basename is an
// absolute path then JoinPath ignores dirname and simply returns basename.
std::string JoinPath(const std::string& dirname, const std::string& basename);

}  // namespace xmpmeta

#endif  // XMPMETA_FILE_H_
