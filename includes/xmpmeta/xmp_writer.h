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

#ifndef XMPMETA_XMP_WRITER_H_
#define XMPMETA_XMP_WRITER_H_

#include <iostream>
#include <string>
#include <memory>

#include "base/port.h"
#include "xmpmeta/xmp_data.h"

namespace xmpmeta {

// Creates a new XmpData object and initializes the boilerplate for the
// standard XMP section.
// The extended section is initialized only if create_extended is true.
std::unique_ptr<XmpData> CreateXmpData(bool create_extended);

// Writes  XMP data to an existing JPEG image file.
// This is equivalent to writeXMPMeta in geo/lightfield/metadata/XmpUtil.java.
// If the extended section is not null, this will modify the given XmpData by
// setting a property in the standard section that links it with the
// extended section.
bool WriteLeftEyeAndXmpMeta(const string& left_data, const string& filename,
                            const XmpData& xmp_data);

// Updates a JPEG input stream with new XMP data and writes it to an
// output stream.
// This is equivalent to writeXMPMeta in geo/lightfield/metadata/XmpUtil.java.
bool AddXmpMetaToJpegStream(std::istream* input_jpeg_stream,
                            const XmpData& xmp_data,
                            std::ostream* output_jpeg_stream);

}  // namespace xmpmeta

#endif  // XMPMETA_XMP_WRITER_H_
