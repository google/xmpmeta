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

#include "xmpmeta/gimage.h"

#include <memory>
#include <string>

#include <libxml/tree.h>

#include "glog/logging.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::xml::ToXmlChar;
using xmpmeta::xml::SerializerImpl;

namespace xmpmeta {
namespace {

const char kBadPath[] = "bad_path.jpg";
const char kLeftFile[] = "left_with_xmp.jpg";
const char kMime[] = "image/jpeg";
const char kRightFile[] = "right_embedded.jpg";

std::unique_ptr<SerializerImpl>
    CreateSerializerForTest(const XmpData& xmp_data, bool use_extended_section,
                            const xmlNsPtr xml_ns) {
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace("GImage", xml_ns);
  std::unique_ptr<SerializerImpl> serializer =
      SerializerImpl::FromDataAndSerializeNamespaces(
          namespaces, xml::GetFirstDescriptionElement(
              use_extended_section ?
              xmp_data.ExtendedSection() :
              xmp_data.StandardSection()));
  return serializer;
}

TEST(GImage, ParseVrPhotoXmp) {
  const string left_path = TestFileAbsolutePath(kLeftFile);
  const string right_path = TestFileAbsolutePath(kRightFile);

  std::unique_ptr<GImage> gimage = GImage::FromJpegFile(left_path);
  ASSERT_NE(nullptr, gimage);

  std::string expected_image_data;
  ReadFileToStringOrDie(right_path, &expected_image_data);
  EXPECT_EQ(kMime, gimage->GetMime());
  EXPECT_EQ(expected_image_data, gimage->GetData());
}

TEST(GImage, BadPath) {
  std::unique_ptr<GImage> gimage = GImage::FromJpegFile(kBadPath);
  ASSERT_EQ(nullptr, gimage);
}

TEST(GImage, IsPresent) {
  const string left_path = TestFileAbsolutePath(kLeftFile);
  EXPECT_TRUE(GImage::IsPresent(left_path));
  EXPECT_FALSE(GImage::IsPresent(kBadPath));
}

TEST(GImage, ToVrPhotoXmp) {
  // Get image data from test file.
  const string left_path = TestFileAbsolutePath(kLeftFile);
  const string right_path = TestFileAbsolutePath(kRightFile);

  std::unique_ptr<GImage> gimage = GImage::FromJpegFile(left_path);
  ASSERT_NE(nullptr, gimage);

  // Write to XmpData.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  ASSERT_NE(nullptr, xmp_data);

  // Need both namespaces because libxml moves ownership of the xmlNsPtr
  // to the standard and extended sections' documents, respectively.
  xmlNsPtr main_xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"), ToXmlChar("GImage"));
  xmlNsPtr ext_xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"), ToXmlChar("GImage"));

  std::unique_ptr<SerializerImpl> std_serializer =
      CreateSerializerForTest(*xmp_data, false, main_xml_ns);
  std::unique_ptr<SerializerImpl> ext_serializer =
      CreateSerializerForTest(*xmp_data, true, ext_xml_ns);
  ASSERT_TRUE(gimage->Serialize(std_serializer.get(), ext_serializer.get()));

  // Get back from XmpData and check values.
  std::string expected_image_data;
  ReadFileToStringOrDie(right_path, &expected_image_data);
  std::unique_ptr<GImage> gimage_from_xmp = GImage::FromXmp(*xmp_data);
  ASSERT_NE(nullptr, gimage_from_xmp);
  EXPECT_EQ(kMime, gimage_from_xmp->GetMime());
  EXPECT_EQ(expected_image_data, gimage_from_xmp->GetData());
}

TEST(GImage, ToVrPhotoXmpFromData) {
  // Get image data from test file.
  const string right_path = TestFileAbsolutePath(kRightFile);

  std::string expected_image_data;
  ReadFileToStringOrDie(right_path, &expected_image_data);
  std::unique_ptr<GImage> gimage =
      GImage::CreateFromData(expected_image_data, kMime);
  ASSERT_NE(nullptr, gimage);

  // Write to XmpData.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  ASSERT_NE(nullptr, xmp_data);

  // Need both namespaces because libxml moves ownership of the xmlNsPtr
  // to the standard and extended sections' documents, respectively.
  xmlNsPtr main_xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"), ToXmlChar("GImage"));
  xmlNsPtr ext_xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"), ToXmlChar("GImage"));

  std::unique_ptr<SerializerImpl> std_serializer =
      CreateSerializerForTest(*xmp_data, false, main_xml_ns);
  std::unique_ptr<SerializerImpl> ext_serializer =
      CreateSerializerForTest(*xmp_data, true, ext_xml_ns);
  ASSERT_TRUE(gimage->Serialize(std_serializer.get(), ext_serializer.get()));

  // Get back from XmpData and check values.
  std::unique_ptr<GImage> gimage_from_xmp = GImage::FromXmp(*xmp_data);
  ASSERT_NE(nullptr, gimage_from_xmp);
  EXPECT_EQ(kMime, gimage_from_xmp->GetMime());
  EXPECT_EQ(expected_image_data, gimage_from_xmp->GetData());
}

TEST(GImage, ToVrPhotoXmpWithNullXmpStandardSectionSerializer) {
  // Get image data from test file.
  const string right_path = TestFileAbsolutePath(kRightFile);

  std::string expected_image_data;
  ReadFileToStringOrDie(right_path, &expected_image_data);
  std::unique_ptr<GImage> gimage =
      GImage::CreateFromData(expected_image_data, kMime);
  ASSERT_NE(nullptr, gimage);

  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNsPtr xml_ns = xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"),
                             ToXmlChar("GImage"));
  std::unique_ptr<SerializerImpl> ext_serializer =
      CreateSerializerForTest(*xmp_data, true, xml_ns);

  std::unique_ptr<GImage> new_gimage =
      GImage::CreateFromData(expected_image_data, kMime);
  ASSERT_FALSE(gimage->Serialize(nullptr, ext_serializer.get()));
}

TEST(GImage, ToVrPhotoXmpWithNullXmpExtendedSectionSerializer) {
  // Get image data from test file.
  const string right_path = TestFileAbsolutePath(kRightFile);

  std::string expected_image_data;
  ReadFileToStringOrDie(right_path, &expected_image_data);
  std::unique_ptr<GImage> gimage =
      GImage::CreateFromData(expected_image_data, kMime);
  ASSERT_NE(nullptr, gimage);

  xmlNsPtr xml_ns = xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"),
                             ToXmlChar("GImage"));

  std::unique_ptr<XmpData> xmp_data = CreateXmpData(false);
  std::unique_ptr<SerializerImpl> std_serializer =
      CreateSerializerForTest(*xmp_data, false, xml_ns);

  std::unique_ptr<GImage> new_gimage =
      GImage::CreateFromData(expected_image_data, kMime);
  ASSERT_FALSE(new_gimage->Serialize(std_serializer.get(), nullptr));
}

}  // namespace
}  // namespace xmpmeta
