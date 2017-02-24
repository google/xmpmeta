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

#include "xmpmeta/jpeg_io.h"

#include <fstream>
#include <string>
#include <vector>

#include "base/port.h"
#include "glog/logging.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/test_xmp_creator.h"
#include "xmpmeta/xmp_const.h"

namespace xmpmeta {
namespace {

// Test file paths.
const char* kJpegTestDataPath = "left_with_xmp.jpg";

// XMP standard test data.
const char* kXmpBody =
    "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP\">\n"
    "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
    "    <rdf:Description rdf:about=\"\"\n"
    "      xmlns:GImage=\"http://ns.google.com/photos/1.0/image/\"\n"
    "      xmlns:xmpNote=\"http://ns.adobe.com/xmp/note/\"\n"
    "      GImage:Mime=\"image/jpeg\"\n"
    "      xmpNote:HasExtendedXMP=\"123ABC\"/>\n"
    "  </rdf:RDF>\n"
    "</x:xmpmeta>\n";

const char* kXmpMalformedBody =
    "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP\">\n"
    "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
    "    <rdf:Description rdf:about=\"\"\n"
    "      xmlns:GImage=\"http://ns.google.com/photos/1.0/image/\"\n"
    "      xmlns:xmpNote=\"http://ns.adobe.com/xmp/note/\"\n"
    "      GImage:Mime=\"image/jpeg\"\n"
    "      xmpNote:HasExtendedXMP=\"123ABC\"/>\n"
    "</x:xmpmeta>\n";

// XMP extension test data.
const char* kXmpExtensionHeaderPart2 = "123ABCxxxxxxxx";
const char* kXmpExtensionBody =
    "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP\">\n"
    "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
    "    <rdf:Description rdf:about=\"\"\n"
    "      xmlns:GImage=\"http://ns.google.com/photos/1.0/image/\"\n"
    "      GImage:Data=\"9865\"/>\n"
    "  </rdf:RDF>\n"
    "</x:xmpmeta>\n";

TEST(JpegIO, ParseStandardXmp) {
  const string filename = TempFileAbsolutePath("test.jpg");
  std::vector<string> standard_xmp;
  standard_xmp.push_back(TestXmpCreator::CreateStandardXmpString(kXmpBody));
  TestXmpCreator::WriteJPEGFile(filename, standard_xmp);

  std::ifstream file(filename.c_str(), std::ios::binary);
  ASSERT_TRUE(file.is_open());
  ParseOptions parse_options;
  parse_options.read_meta_only = true;
  const std::vector<Section> sections = Parse(parse_options, &file);
  EXPECT_FALSE(sections.empty());
  for (const auto& section : sections) {
    EXPECT_FALSE(section.is_image_section);
  }
}

TEST(JpegIO, ParseMalformedStandardXmp) {
  const string filename = TempFileAbsolutePath("test.jpg");
  std::vector<string> standard_xmp;
  standard_xmp.push_back(
      TestXmpCreator::CreateStandardXmpString(kXmpMalformedBody));
  TestXmpCreator::WriteJPEGFile(filename, standard_xmp);

  std::ifstream file(filename.c_str(), std::ios::binary);
  ASSERT_TRUE(file.is_open());
  ParseOptions parse_options;
  parse_options.read_meta_only = true;
  const std::vector<Section> sections = Parse(parse_options, &file);
  EXPECT_FALSE(sections.empty());
  for (const auto& section : sections) {
    EXPECT_FALSE(section.is_image_section);
  }
}

TEST(JpegIO, ParseExtendedXmp) {
  const string filename = TempFileAbsolutePath("test.jpg");
  std::vector<string> xmp_sections =
      TestXmpCreator::CreateExtensionXmpStrings(2, kXmpExtensionHeaderPart2,
                                                kXmpExtensionBody);
  xmp_sections.insert(xmp_sections.begin(),
                      TestXmpCreator::CreateStandardXmpString(kXmpBody));
  TestXmpCreator::WriteJPEGFile(filename, xmp_sections);

  std::ifstream file(filename.c_str(), std::ios::binary);
  ASSERT_TRUE(file.is_open());
  ParseOptions parse_options;
  parse_options.read_meta_only = true;
  const std::vector<Section> sections = Parse(parse_options, &file);
  EXPECT_EQ(3, sections.size());
  for (const auto& section : sections) {
    EXPECT_FALSE(section.is_image_section);
  }
}

TEST(JpegIO, ParseOnlyStandardSectionInExtendedXmp) {
  const string filename = TempFileAbsolutePath("test.jpg");
  std::vector<string> xmp_sections =
      TestXmpCreator::CreateExtensionXmpStrings(2, kXmpExtensionHeaderPart2,
                                                kXmpExtensionBody);
  xmp_sections.insert(xmp_sections.begin(),
                      TestXmpCreator::CreateStandardXmpString(kXmpBody));
  TestXmpCreator::WriteJPEGFile(filename, xmp_sections);

  std::ifstream file(filename.c_str(), std::ios::binary);
  ASSERT_TRUE(file.is_open());
  ParseOptions parse_options;
  parse_options.read_meta_only = true;
  parse_options.section_header = XmpConst::Header();
  const std::vector<Section> sections = Parse(parse_options, &file);
  EXPECT_EQ(1, sections.size());
  for (const auto& section : sections) {
    EXPECT_FALSE(section.is_image_section);
  }
}

TEST(JpegIO, ParseAllExtendedSections) {
  const string filename = TempFileAbsolutePath("test.jpg");
  std::vector<string> xmp_sections =
      TestXmpCreator::CreateExtensionXmpStrings(2, kXmpExtensionHeaderPart2,
                                                kXmpExtensionBody);
  xmp_sections.insert(xmp_sections.begin(),
                      TestXmpCreator::CreateStandardXmpString(kXmpBody));
  TestXmpCreator::WriteJPEGFile(filename, xmp_sections);

  std::ifstream file(filename.c_str(), std::ios::binary);
  ASSERT_TRUE(file.is_open());
  ParseOptions parse_options;
  parse_options.read_meta_only = true;
  parse_options.section_header = XmpConst::ExtensionHeader();
  EXPECT_EQ(2, Parse(parse_options, &file).size());
}

TEST(JpegIO, ParseFirstExtendedSection) {
  const string filename = TempFileAbsolutePath("test.jpg");
  std::vector<string> xmp_sections =
      TestXmpCreator::CreateExtensionXmpStrings(2, kXmpExtensionHeaderPart2,
                                                kXmpExtensionBody);
  xmp_sections.insert(xmp_sections.begin(),
                      TestXmpCreator::CreateStandardXmpString(kXmpBody));
  TestXmpCreator::WriteJPEGFile(filename, xmp_sections);

  std::ifstream file(filename.c_str(), std::ios::binary);
  ASSERT_TRUE(file.is_open());
  ParseOptions parse_options;
  parse_options.read_meta_only = true;
  parse_options.section_header = XmpConst::ExtensionHeader();
  parse_options.section_header_return_first = true;
  EXPECT_EQ(1, Parse(parse_options, &file).size());
}

TEST(JpegIO, ParseJpegWithExtendedXmpReadMetaOnly) {
  const string filename = TestFileAbsolutePath(kJpegTestDataPath);
  std::ifstream file(filename.c_str(), std::ios::binary);

  ASSERT_TRUE(file.is_open());
  ParseOptions parse_options;
  parse_options.read_meta_only = true;
  const std::vector<Section> sections = Parse(parse_options, &file);
  EXPECT_FALSE(sections.empty());
  for (const auto& section : sections) {
    EXPECT_FALSE(section.is_image_section);
  }
}

TEST(JpegIO, ParseJpegWithExtendedXmpReadEverything) {
  const string filename = TestFileAbsolutePath(kJpegTestDataPath);
  std::ifstream file(filename.c_str(), std::ios::binary);

  ASSERT_TRUE(file.is_open());
  ParseOptions parse_options;
  const std::vector<Section> sections = Parse(parse_options, &file);
  EXPECT_FALSE(sections.empty());
  for (int i = 0; i < sections.size() - 1; i++) {
    EXPECT_FALSE(sections.at(i).is_image_section);
  }
  EXPECT_TRUE(sections.at(sections.size() - 1).is_image_section);
}

}  // namespace
}  // namespace xmpmeta
