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

#ifndef XMPMETA_JPEG_IO_H_
#define XMPMETA_JPEG_IO_H_

#include <string>
#include <vector>

#include "base/port.h"

namespace xmpmeta {

// Contains the data for a section in a JPEG file.
// A JPEG file contains many sections in addition to image data.
struct Section {
  // Constructors.
  Section() = default;
  explicit Section(const string& buffer);

  // Returns true if the section's marker matches an APP1 marker.
  bool IsMarkerApp1();

  int marker;
  bool is_image_section;
  string data;
};

struct ParseOptions {
  // If set to true, keeps only the EXIF and XMP sections (with
  // marker kApp1) and ignores others. Otherwise, keeps everything including
  // image data.
  bool read_meta_only = false;

  // If section_header is set, this boolean controls whether only the 1st
  // section matching the section_header will be returned. If not set
  // (the default), all the sections that math the section header will be
  // returned.
  bool section_header_return_first = false;

  // A filter that keeps all the sections whose data starts with the
  // given string. Ignored if empty.
  string section_header;
};


// Parses the JPEG image file.
std::vector<Section> Parse(const ParseOptions& options,
                           std::istream* input_stream);

// Writes JPEG data sections to a file.
void WriteSections(const std::vector<Section>& sections,
                   std::ostream* output_stream);

}  // namespace xmpmeta

#endif  // XMPMETA_JPEG_IO_H_
