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

#ifndef XMPMETA_TEST_XMP_CREATOR_H_
#define XMPMETA_TEST_XMP_CREATOR_H_

#include <string>
#include <vector>

#include "base/port.h"

namespace xmpmeta {

// Creates XMP strings and files to aid testing.
class TestXmpCreator {
 public:
  // Combines the xmp header and body to form a string to be used in the jpeg.
  static string CreateStandardXmpString(string xmp_body);

  // Splits up the xmp extension body into multiple strings with headers, each
  // of which is to be used as a section in the jpeg.
  // extension_header_part_2 is the GUID of the extended section's contents.
  static std::vector<string>
      CreateExtensionXmpStrings(const int num_sections,
                                const char* extension_header_part_2,
                                const char* extension_body);

  // Returns a string containing fake JPEG data.
  static string GetFakeJpegPayload();

  // Writes XMP metadata into a new JPEG file.
  static void WriteJPEGFile(const string& filename,
                            const std::vector<string>& xmp_sections);

  // Returns the contents of a JPEG file with the given XMP metadata.
  static string MakeJPEGFileContents(const std::vector<string>& xmp_sections);
};

}  // namespace xmpmeta

#endif  // XMPMETA_TEST_XMP_CREATOR_H_
