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

#include "xmpmeta/gpano.h"

#include <memory>
#include <string>

#include <libxml/tree.h>

#include "glog/logging.h"
#include "xmpmeta/file.h"
#include "xmpmeta/pano_meta_data.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/test_xmp_creator.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_parser.h"
#include "xmpmeta/xmp_writer.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::PanoMetaData;
using xmpmeta::xml::SerializerImpl;
using xmpmeta::xml::ToXmlChar;

namespace xmpmeta {
namespace {

const int kCroppedLeft = 2420;
const int kCroppedTop = 1396;
const int kCroppedWidth = 3782;
const int kCroppedHeight = 1566;
const int kFullWidth = 8192;
const int kFullHeight = 4096;
const int kInitialHeadingDegrees = 189;

std::unique_ptr<SerializerImpl>
    CreateSerializerForTest(const XmpData& xmp_data, const xmlNsPtr xml_ns) {
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace("GPano", xml_ns);
  std::unique_ptr<SerializerImpl> serializer =
      SerializerImpl::FromDataAndSerializeNamespaces(
          namespaces,
          xml::GetFirstDescriptionElement(xmp_data.StandardSection()));
  return serializer;
}

TEST(GPano, ParseVrPhotoXmp) {
  const string left_path = TestFileAbsolutePath("left_with_xmp.jpg");
  std::unique_ptr<GPano> gpano = GPano::FromJpegFile(left_path);
  ASSERT_NE(nullptr, gpano);

  const PanoMetaData& meta_data = gpano->GetPanoMetaData();
  EXPECT_EQ(kCroppedLeft, meta_data.cropped_left);
  EXPECT_EQ(kCroppedTop, meta_data.cropped_top);
  EXPECT_EQ(kCroppedWidth, meta_data.cropped_width);
  EXPECT_EQ(kCroppedHeight, meta_data.cropped_height);
  EXPECT_EQ(kFullWidth, meta_data.full_width);
  EXPECT_EQ(kFullHeight, meta_data.full_height);
  EXPECT_EQ(kInitialHeadingDegrees, meta_data.initial_heading_degrees);
}

TEST(GPano, BadPath) {
  std::unique_ptr<GPano> gpano = GPano::FromJpegFile("bad_path.jpg");
  ASSERT_EQ(nullptr, gpano);
}

TEST(GPano, ToVrPhotoXmp) {
  PanoMetaData new_meta_data;
  new_meta_data.cropped_left = kCroppedLeft;
  new_meta_data.cropped_top = kCroppedTop;
  new_meta_data.cropped_width = kCroppedWidth;
  new_meta_data.cropped_height = kCroppedHeight;
  new_meta_data.full_width = kFullWidth;
  new_meta_data.full_height = kFullHeight;
  new_meta_data.initial_heading_degrees = kInitialHeadingDegrees;

  // Doesn't matter whether there's an extended section or not.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(false);
  std::unique_ptr<GPano> new_gpano = GPano::CreateFromData(new_meta_data);
  ASSERT_NE(nullptr, new_gpano);

  xmlNsPtr xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"), ToXmlChar("GPano"));
  std::unique_ptr<SerializerImpl> serializer =
      CreateSerializerForTest(*xmp_data, xml_ns);
  ASSERT_TRUE(new_gpano->Serialize(serializer.get()));

  std::unique_ptr<GPano> gpano_from_xmp = GPano::FromXmp(*xmp_data);
  ASSERT_NE(nullptr, gpano_from_xmp);

  const PanoMetaData& meta_data = gpano_from_xmp->GetPanoMetaData();
  EXPECT_EQ(kCroppedLeft, meta_data.cropped_left);
  EXPECT_EQ(kCroppedTop, meta_data.cropped_top);
  EXPECT_EQ(kCroppedWidth, meta_data.cropped_width);
  EXPECT_EQ(kCroppedHeight, meta_data.cropped_height);
  EXPECT_EQ(kFullWidth, meta_data.full_width);
  EXPECT_EQ(kFullHeight, meta_data.full_height);
  EXPECT_EQ(kInitialHeadingDegrees, meta_data.initial_heading_degrees);
}

TEST(GPano, ToVrPhotoXmpWithNullSerializer) {
  PanoMetaData new_meta_data;
  std::unique_ptr<GPano> new_gpano = GPano::CreateFromData(new_meta_data);
  ASSERT_FALSE(new_gpano->Serialize(nullptr));
}

TEST(GPano, ParseXmpTest) {
  const char kStandardSectionWithAudioData[] =
      "vr_photo_with_audio_std_section_data.txt";
  const char kStandardSectionWithoutAudioData[] =
      "vr_photo_no_audio_std_section_data.txt";
  const char kPhotoSphereStdSectionData[] = "photo_sphere_std_section_data.txt";

  const std::vector<string> data_paths = {
      TestFileAbsolutePath(kStandardSectionWithAudioData),
      TestFileAbsolutePath(kStandardSectionWithoutAudioData),
      TestFileAbsolutePath(kPhotoSphereStdSectionData)};

  for (const string& data_path : data_paths) {
    std::string xmp_body;
    ReadFileToStringOrDie(data_path, &xmp_body);

    const string filename = TempFileAbsolutePath("test.jpg");
    std::vector<string> standard_xmp;
    standard_xmp.push_back(TestXmpCreator::CreateStandardXmpString(xmp_body));
    TestXmpCreator::WriteJPEGFile(filename, standard_xmp);

    XmpData xmp_data;
    ASSERT_TRUE(ReadXmpHeader(filename, true, &xmp_data));
    std::unique_ptr<GPano> gpano_from_xmp = GPano::FromXmp(xmp_data);
    EXPECT_NE(nullptr, gpano_from_xmp) << data_path;
  }
}

}  // namespace
}  // namespace xmpmeta
