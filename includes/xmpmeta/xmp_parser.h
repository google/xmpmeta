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

#ifndef XMPMETA_XMP_PARSER_H_
#define XMPMETA_XMP_PARSER_H_

#include <fstream>
#include <string>

#include "base/port.h"
#include "xmpmeta/xmp_data.h"

namespace xmpmeta {

// Populates a XmpData from the header of the JPEG file.
bool ReadXmpHeader(const string& filename, bool skip_extended,
                   XmpData* xmp_data);

// Populates a XmpData from the header of JPEG file that has already been read
// into memory.
bool ReadXmpFromMemory(const string& jpeg_contents, bool skip_extended,
                       XmpData* xmp_data);

// Populates a XmpData from the header of the given stream (stream data is
// in JPEG format).
bool ReadXmpHeader(std::istream* input_stream, bool skip_extended,
                   XmpData* xmp_data);
}  // namespace xmpmeta

#endif  // XMPMETA_XMP_PARSER_H_
