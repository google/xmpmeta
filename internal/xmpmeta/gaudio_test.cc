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

#include "xmpmeta/gaudio.h"

#include <memory>
#include <string>

#include <libxml/tree.h>

#include "glog/logging.h"
#include "gtest/gtest.h"
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

// Test data constants.
const char kAudioFile[] = "audio_embedded.mp4";
const char kBadPath[] = "bad_path.jpg";
const char kLeftFile[] = "left_with_xmp.jpg";
const char kMime[] = "audio/mp4a-latm";

std::unique_ptr<SerializerImpl>
    CreateSerializerForTest(const XmpData& xmp_data, bool use_extended_section,
                            const xmlNsPtr xml_ns) {
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace("GAudio", xml_ns);
  std::unique_ptr<SerializerImpl> serializer =
      SerializerImpl::FromDataAndSerializeNamespaces(
          namespaces, xml::GetFirstDescriptionElement(
              use_extended_section ?
              xmp_data.ExtendedSection() :
              xmp_data.StandardSection()));
  return serializer;
}

TEST(GAudio, ParseVrPhotoXmp) {
  const string left_path = TestFileAbsolutePath(kLeftFile);
  const string audio_path = TestFileAbsolutePath(kAudioFile);

  std::unique_ptr<GAudio> gaudio = GAudio::FromJpegFile(left_path);
  ASSERT_NE(nullptr, gaudio);

  std::string expected_audio_data;
  ReadFileToStringOrDie(audio_path, &expected_audio_data);
  EXPECT_EQ(kMime, gaudio->GetMime());
  EXPECT_EQ(expected_audio_data, gaudio->GetData());
}

TEST(GAudio, BadPath) {
  std::unique_ptr<GAudio> gaudio = GAudio::FromJpegFile(kBadPath);
  ASSERT_EQ(nullptr, gaudio);
}

TEST(GAudio, IsPresent) {
  const string left_path = TestFileAbsolutePath(kLeftFile);
  EXPECT_TRUE(GAudio::IsPresent(left_path));
  EXPECT_FALSE(GAudio::IsPresent(kBadPath));
}

TEST(GAudio, ToVrPhotoXmpFromFile) {
  const string left_path = TestFileAbsolutePath(kLeftFile);
  const string audio_path = TestFileAbsolutePath(kAudioFile);

  std::unique_ptr<GAudio> gaudio = GAudio::FromJpegFile(left_path);
  ASSERT_NE(nullptr, gaudio);
  std::string decoded_audio_data;
  ReadFileToStringOrDie(audio_path, &decoded_audio_data);

  // Write to XmpData.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  ASSERT_NE(nullptr, xmp_data);
  EXPECT_EQ(kMime, gaudio->GetMime());
  EXPECT_EQ(string(decoded_audio_data), gaudio->GetData());

  // Need both namespaces because libxml moves ownership of the xmlNsPtr
  // to the standard and extended sections' documents, respectively.
  xmlNsPtr main_xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"), ToXmlChar("GAudio"));
  xmlNsPtr ext_xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"), ToXmlChar("GAudio"));
  std::unique_ptr<SerializerImpl> std_serializer =
      CreateSerializerForTest(*xmp_data, false, main_xml_ns);
  std::unique_ptr<SerializerImpl> ext_serializer =
      CreateSerializerForTest(*xmp_data, true, ext_xml_ns);
  ASSERT_TRUE(gaudio->Serialize(std_serializer.get(), ext_serializer.get()));

  // Get back from XmpData and check values.
  std::unique_ptr<GAudio> gaudio_from_xmp = GAudio::FromXmp(*xmp_data);
  ASSERT_NE(nullptr, gaudio_from_xmp);
  EXPECT_EQ(kMime, gaudio_from_xmp->GetMime());
  EXPECT_EQ(decoded_audio_data, gaudio_from_xmp->GetData());
}


TEST(GAudio, ToVrPhotoXmpFromData) {
  // Get data from test file.
  const string left_path = TestFileAbsolutePath(kLeftFile);
  const string audio_path = TestFileAbsolutePath(kAudioFile);

  // Not base64 encoded.
  std::string decoded_audio_data;
  ReadFileToStringOrDie(audio_path, &decoded_audio_data);

  // Write to XmpData.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  ASSERT_NE(nullptr, xmp_data);
  std::unique_ptr<GAudio> new_gaudio =
      GAudio::CreateFromData(decoded_audio_data, kMime);
  ASSERT_NE(nullptr, new_gaudio);
  EXPECT_EQ(kMime, new_gaudio->GetMime());
  EXPECT_EQ(string(decoded_audio_data), new_gaudio->GetData());

  // Need both namespaces because libxml moves ownership of the xmlNsPtr
  // to the standard and extended sections' documents, respectively.
  xmlNsPtr main_xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"), ToXmlChar("GAudio"));
  xmlNsPtr ext_xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"), ToXmlChar("GAudio"));

  std::unique_ptr<SerializerImpl> std_serializer =
      CreateSerializerForTest(*xmp_data, false, main_xml_ns);
  std::unique_ptr<SerializerImpl> ext_serializer =
      CreateSerializerForTest(*xmp_data, true, ext_xml_ns);
  ASSERT_TRUE(new_gaudio->Serialize(std_serializer.get(),
                                    ext_serializer.get()));

  // Get back from XmpData and check values.
  std::unique_ptr<GAudio> gaudio_from_xmp = GAudio::FromXmp(*xmp_data);
  ASSERT_NE(nullptr, gaudio_from_xmp);
  EXPECT_EQ(kMime, gaudio_from_xmp->GetMime());
  EXPECT_EQ(decoded_audio_data, gaudio_from_xmp->GetData());
}

TEST(GAudio, ToVrPhotoXmpWithNullStandardSectionSerializer) {
  const string audio_path = TestFileAbsolutePath(kAudioFile);

  // Not base64 encoded.
  std::string decoded_audio_data;
  ReadFileToStringOrDie(audio_path, &decoded_audio_data);
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  ASSERT_NE(nullptr, xmp_data);

  xmlNsPtr xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"),
               ToXmlChar("GAudio"));
  std::unique_ptr<SerializerImpl> ext_serializer =
      CreateSerializerForTest(*xmp_data, true, xml_ns);

  std::unique_ptr<GAudio> new_gaudio =
      GAudio::CreateFromData(decoded_audio_data, kMime);
  ASSERT_FALSE(new_gaudio->Serialize(nullptr, ext_serializer.get()));
}

TEST(GAudio, ToVrPhotoXmpWithNullExtendedSectionSerializer) {
  const string audio_path = TestFileAbsolutePath(kAudioFile);

  // Not base64 encoded.
  std::string decoded_audio_data;
  ReadFileToStringOrDie(audio_path, &decoded_audio_data);
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(false);
  ASSERT_EQ(nullptr, xmp_data->ExtendedSection());

  xmlNsPtr xml_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakehref.com/"),
               ToXmlChar("GAudio"));
  std::unique_ptr<SerializerImpl> std_serializer =
      CreateSerializerForTest(*xmp_data, false, xml_ns);

  std::unique_ptr<GAudio> new_gaudio =
      GAudio::CreateFromData(decoded_audio_data, kMime);
  ASSERT_FALSE(new_gaudio->Serialize(std_serializer.get(), nullptr));
}

}  // namespace
}  // namespace xmpmeta
