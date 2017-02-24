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

#include "test_xmp_creator.h"

#include <sstream>

#include "xmpmeta/file.h"

namespace xmpmeta {
namespace {
// XMP standard test data.
const char* kXmpHeader = "http://ns.adobe.com/xap/1.0/";

// XMP extension test data.
const char* kXmpExtensionHeaderPart1 = "http://ns.adobe.com/xmp/extension/";

// JPEG boilerplate.
const char* kJPEGFileStart = "\xff\xd8";
const char* kJPEGSectionStart = "\xff\xe1";
const char* kJPEGRemainder =
    "\xff\xdaJpegPixelData\xff\xd9";  // SOS, data, EOI.

}  // namespace

string TestXmpCreator::CreateStandardXmpString(string xmp_body) {
  string xmp_string = kXmpHeader;
  xmp_string.push_back(0);
  xmp_string.append(xmp_body);
  return xmp_string;
}

std::vector<string>
TestXmpCreator::CreateExtensionXmpStrings(const int num_sections,
                                          const char* extension_header_part_2,
                                          const char* extension_body) {
  string body = extension_body;
  const int length = body.size();
  std::vector<string> sections;
  for (int i = 0; i < num_sections; ++i) {
    const int start = length * i / num_sections;
    const int end = length * (i + 1) / num_sections;
    string xmp_string = kXmpExtensionHeaderPart1;
    xmp_string.push_back(0);
    xmp_string.append(extension_header_part_2);
    xmp_string.append(body.substr(start, end - start));
    sections.emplace_back(xmp_string);
  }
  return sections;
}

string TestXmpCreator::GetFakeJpegPayload() {
  return kJPEGRemainder;
}

string TestXmpCreator::MakeJPEGFileContents(
    const std::vector<string>& xmp_sections) {
  // Inefficient way to construct a string, but not important here. Just being
  // careful to avoid string construction options that don't play nicely with
  // \0s in the middle.
  string file_contents = kJPEGFileStart;
  for (const string& section : xmp_sections) {
    const int length = section.size() + 2;
    const char lh = static_cast<char>(length >> 8);
    const char ll = static_cast<char>(length & 0xff);
    file_contents.append(kJPEGSectionStart);
    file_contents.push_back(lh);
    file_contents.push_back(ll);
    file_contents.append(section);
  }
  file_contents.append(kJPEGRemainder);
  return file_contents;
}

void TestXmpCreator::WriteJPEGFile(const string& filename,
                                   const std::vector<string>& xmp_sections) {
  string file_contents = MakeJPEGFileContents(xmp_sections);
  WriteStringToFileOrDie(file_contents, filename);
}

}  // namespace xmpmeta
