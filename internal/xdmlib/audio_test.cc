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

#include "xdmlib/audio.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/port.h"
#include "gtest/gtest.h"
#include "xdmlib/const.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/base64.h"
#include "xmpmeta/xmp_const.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::CreateXmpData;
using xmpmeta::ReadFileToStringOrDie;
using xmpmeta::TestFileAbsolutePath;
using xmpmeta::xml::Deserializer;
using xmpmeta::xml::DeserializerImpl;
using xmpmeta::xml::GetFirstDescriptionElement;
using xmpmeta::xml::Serializer;
using xmpmeta::xml::SerializerImpl;
using xmpmeta::xml::ToXmlChar;
using xmpmeta::xml::XmlConst;
using xmpmeta::xml::XmlDocToString;

namespace xmpmeta {
namespace xdm {
namespace {

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/audio/";
const char kAudioDataPath[] = "xdm/audio_testdata.txt";

TEST(Audio, GetNamespaces) {
  std::unordered_map<string, string> ns_name_href_map;
  string prefix(XdmConst::Audio());

  string data("123ABC456DEF");
  string mime("audio/mp4");
  std::unique_ptr<Audio> audio = Audio::FromData(data, mime);
  ASSERT_NE(nullptr, audio);

  ASSERT_TRUE(ns_name_href_map.empty());
  audio->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(1, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));

  // Same namespace will not be added again.
  audio->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(1, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));
}

TEST(Audio, FromData) {
  string data("123ABC456DEF");
  string mime("audio/mp4");
  std::unique_ptr<Audio> audio = Audio::FromData(data, mime);
  ASSERT_NE(nullptr, audio);
  // Set to default mimetype.
  EXPECT_EQ("audio/mp4", audio->GetMime());
  EXPECT_EQ(data, audio->GetData());
}

TEST(Audio, FromEmptyData) {
  string data("123ABC456DEF");
  string mime("audio/mp4");
  std::unique_ptr<Audio> audio = Audio::FromData(string(""), string(""));
  EXPECT_EQ(nullptr, audio);
  audio = Audio::FromData(data, string(""));
  EXPECT_EQ(nullptr, audio);
  audio = Audio::FromData(string(""), mime);
  EXPECT_EQ(nullptr, audio);
}

TEST(Audio, Serialize) {
  string data("123ABC456DEF");
  string mime("image/wav");

  std::unique_ptr<Audio> audio = Audio::FromData(data, mime);

  // Create XML serializer.
  const char* device_name = XdmConst::Device();
  const char* camera_name = XdmConst::Camera();
  const char* audio_name = XdmConst::Audio();
  const char* namespaceHref = "http://notarealh.ref";
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                           ToXmlChar(device_name)));
  namespaces.emplace(camera_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                           ToXmlChar(camera_name)));
  namespaces.emplace(audio_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                          ToXmlChar(audio_name)));
  xmlNodePtr device_node = xmlNewNode(nullptr, ToXmlChar(device_name));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  std::unique_ptr<Serializer> camera_serializer =
      serializer.CreateSerializer(XdmConst::Namespace(camera_name),
                                  camera_name);
  ASSERT_NE(nullptr, camera_serializer);
  std::unique_ptr<Serializer> audio_serializer =
      camera_serializer->CreateSerializer(XdmConst::Namespace(audio_name),
                                          audio_name);
  ASSERT_NE(nullptr, audio_serializer);

  ASSERT_TRUE(audio->Serialize(audio_serializer.get()));

  const string xdm_audio_data_path =
      TestFileAbsolutePath(kAudioDataPath);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(xdm_audio_data_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xml_doc));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Audio, ReadMetadata) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock an XdmCamera node.
  xmlNodePtr camera_node = xmlNewNode(nullptr, ToXmlChar("Camera"));
  xmlAddChild(description_node, camera_node);

  string prefix(XdmConst::Audio());
  xmlNsPtr camera_ns =
      xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
               ToXmlChar(XdmConst::Namespace(XdmConst::Audio()).data()));
  xmlNodePtr audio_node = xmlNewNode(camera_ns, ToXmlChar(prefix.data()));
  xmlAddChild(camera_node, audio_node);

  string data("123ABC456DEF");
  string base64_encoded;
  ASSERT_TRUE(EncodeBase64(data, &base64_encoded));
  string mime("audio/wav");
  xmlNsPtr audio_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                               ToXmlChar(prefix.data()));
  xmlSetNsProp(audio_node, audio_ns, ToXmlChar("Mime"),
               ToXmlChar(mime.data()));
  xmlSetNsProp(audio_node, audio_ns, ToXmlChar("Data"),
               ToXmlChar(base64_encoded.data()));

  // Create an Audio from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Audio> audio = Audio::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, audio.get());
  EXPECT_EQ(mime, audio->GetMime());
  EXPECT_EQ(data, audio->GetData());

  xmlFreeNs(audio_ns);
  xmlFreeNs(camera_ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
