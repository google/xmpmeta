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

#include "xdmlib/cameras.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "xdmlib/audio.h"
#include "xdmlib/camera.h"
#include "xdmlib/const.h"
#include "xmpmeta/base64.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"

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

const char kNodeName[] = "Cameras";
const char kAudioPrefix[] = "Audio";
const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/camera/";
const char kAudioNamespaceHref[] = "http://ns.xdm.org/photos/1.0/audio/";

// Test data constants.
const char kCamerasDataPath[] = "xdm/cameras_testdata.txt";
const char kMediaData[] = "123ABC456DEF";

// Convenience function for creating an XML node.
xmlNodePtr NewNode(xmlNsPtr xml_ns, const string& node_name) {
  return xmlNewNode(xml_ns, ToXmlChar(node_name.data()));
}

// Convenience function for creating an XML namespace.
xmlNsPtr NewNs(const string& href, const string& ns_name) {
  return xmlNewNs(nullptr,
                  href.empty() ? nullptr : ToXmlChar(href.data()),
                  ToXmlChar(ns_name.data()));
}

std::unique_ptr<Camera> CreateCamera() {
  std::unique_ptr<Audio> audio = Audio::FromData(kMediaData, "audio/mp4");
  return Camera::FromData(std::move(audio), nullptr, nullptr);
}

TEST(Cameras, GetNamespaces) {
  std::unique_ptr<Camera> camera = CreateCamera();
  std::vector<std::unique_ptr<Camera>> camera_list;
  camera_list.emplace_back(std::move(camera));
  std::unique_ptr<Cameras> cameras = Cameras::FromCameraArray(&camera_list);
  ASSERT_NE(nullptr, cameras);

  string prefix("Camera");
  std::unordered_map<string, string> ns_name_href_map;
  ASSERT_TRUE(ns_name_href_map.empty());
  cameras->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(2, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));
  EXPECT_EQ(string(kAudioNamespaceHref),
            string(ns_name_href_map[kAudioPrefix]));

  // Same namespace will not be added again.
  cameras->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(2, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));
  EXPECT_EQ(string(kAudioNamespaceHref),
            string(ns_name_href_map[kAudioPrefix]));
}

TEST(Cameras, FromCameraArray) {
  std::vector<std::unique_ptr<Camera>> camera_list;
  int num_cameras = 3;
  for (int i = 0; i < num_cameras; i++) {
    std::unique_ptr<Camera> camera = CreateCamera();
    camera_list.emplace_back(std::move(camera));
  }

  std::unique_ptr<Cameras> cameras = Cameras::FromCameraArray(&camera_list);
  ASSERT_NE(nullptr, cameras);
  const std::vector<const Camera*> read_camera_list = cameras->GetCameras();
  const std::unique_ptr<Camera> camera = CreateCamera();
  EXPECT_EQ(num_cameras, read_camera_list.size());
  for (int i = 0; i < num_cameras; i++) {
    const Camera* read_camera = read_camera_list[i];
    ASSERT_NE(nullptr, read_camera);

    // Check that child elements are equal.
    const Audio* audio_1 = camera->GetAudio();
    const Audio* audio_2 = read_camera->GetAudio();
    ASSERT_NE(nullptr, audio_1);
    ASSERT_NE(nullptr, audio_2);
    EXPECT_EQ(audio_1->GetMime(), audio_2->GetMime());
    EXPECT_EQ(audio_1->GetData(), audio_2->GetData());
  }
}

TEST(Cameras, FromEmptyCameraList) {
  std::vector<std::unique_ptr<Camera>> camera_list;
  std::unique_ptr<Cameras> cameras = Cameras::FromCameraArray(&camera_list);
  ASSERT_EQ(nullptr, cameras);
}

TEST(Cameras, SerializeWithoutRdfPrefix) {
  std::vector<std::unique_ptr<Camera>> camera_list;
  int num_cameras = 3;
  for (int i = 0; i < num_cameras; i++) {
    std::unique_ptr<Camera> camera = CreateCamera();
    camera_list.emplace_back(std::move(camera));
  }

  std::unique_ptr<Cameras> cameras = Cameras::FromCameraArray(&camera_list);
  ASSERT_NE(nullptr, cameras);

  // Create XML serializer.
  const char device_name[] = "Device";
  const char camera_name[] = "Camera";
  const char audio_name[] = "Audio";
  const char namespaceHref[] = "http://notarealh.ref";

  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, NewNs(namespaceHref, device_name));
  namespaces.emplace(camera_name, NewNs(kNamespaceHref, camera_name));
  namespaces.emplace(audio_name, NewNs(namespaceHref, audio_name));

  xmlNodePtr device_node = NewNode(nullptr, device_name);
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  ASSERT_FALSE(cameras->Serialize(&serializer));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Cameras, Serialize) {
  std::vector<std::unique_ptr<Camera>> camera_list;
  int num_cameras = 3;
  for (int i = 0; i < num_cameras; i++) {
    std::unique_ptr<Camera> camera = CreateCamera();
    camera_list.emplace_back(std::move(camera));
  }

  std::unique_ptr<Cameras> cameras = Cameras::FromCameraArray(&camera_list);
  ASSERT_NE(nullptr, cameras);

  // Create XML serializer.
  const char device_name[] = "Device";
  const char camera_name[] = "Camera";
  const char audio_name[] = "Audio";
  const char namespaceHref[] = "http://notarealh.ref";

  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, NewNs(namespaceHref, device_name));
  namespaces.emplace(camera_name, NewNs(kNamespaceHref, camera_name));
  namespaces.emplace(audio_name, NewNs(namespaceHref, audio_name));
  namespaces.emplace(XmlConst::RdfPrefix(),
                     NewNs(XmlConst::RdfNodeNs(), XmlConst::RdfPrefix()));

  xmlNodePtr device_node = NewNode(nullptr, device_name);
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  ASSERT_TRUE(cameras->Serialize(&serializer));

  const string testdata_path = TestFileAbsolutePath(kCamerasDataPath);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(testdata_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xml_doc));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Cameras, ReadMetadata) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // XDM Device node.
  xmlNodePtr device_node = NewNode(nullptr, XdmConst::Device());
  xmlAddChild(description_node, device_node);

  // Device:Cameras node.
  xmlNsPtr device_ns = xmlNewNs(nullptr, ToXmlChar("http:fakeh.ref"),
                                ToXmlChar(XdmConst::Device()));
  xmlNodePtr cameras_node = NewNode(device_ns, kNodeName);
  xmlAddChild(device_node, cameras_node);

  // rdf:Seq node.
  xmlNsPtr rdf_ns = NewNs("http://fakeh.ref", XmlConst::RdfPrefix());
  xmlNodePtr rdf_seq_node =
      xmlNewNode(rdf_ns, ToXmlChar(XmlConst::RdfSeq()));
  xmlAddChild(cameras_node, rdf_seq_node);

  // Set up minimal Camera nodes.
  const char audio_ns_href[] = "http://ns.xdm.org/photos/1.0/audio/";
  const char audio_mime[] = "audio/mp4";
  string base64_encoded;
  ASSERT_TRUE(EncodeBase64(kMediaData, &base64_encoded));

  int num_cameras = 3;
  xmlNsPtr audio_ns = NewNs(audio_ns_href, "Audio");
  xmlNsPtr camera_ns = NewNs("http://fakeh.ref", "Camera");
  for (int i = 0; i < num_cameras; i++) {
    xmlNodePtr camera_node = NewNode(device_ns, "Camera");
    xmlNodePtr audio_node = NewNode(camera_ns, "Audio");
    xmlSetNsProp(audio_node, audio_ns, ToXmlChar("Mime"),
                 ToXmlChar(audio_mime));
    xmlSetNsProp(audio_node, audio_ns, ToXmlChar("Data"),
                 ToXmlChar(base64_encoded.data()));
    xmlNodePtr rdf_li_node =
        xmlNewNode(rdf_ns, ToXmlChar(XmlConst::RdfLi()));

    xmlAddChild(camera_node, audio_node);
    xmlAddChild(rdf_li_node, camera_node);
    xmlAddChild(rdf_seq_node, rdf_li_node);
  }

  // Create a Cameras object from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Cameras> cameras = Cameras::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, cameras.get());

  const std::vector<const Camera*> read_camera_list = cameras->GetCameras();
  EXPECT_EQ(num_cameras, read_camera_list.size());
  for (int i = 0; i < num_cameras; i++) {
    const Camera* read_camera = read_camera_list[i];
    ASSERT_NE(nullptr, read_camera);

    const Audio* audio = read_camera->GetAudio();
    ASSERT_NE(nullptr, audio);
    EXPECT_EQ(audio_mime, audio->GetMime());
    EXPECT_EQ(kMediaData, audio->GetData());
  }

  xmlFreeNs(audio_ns);
  xmlFreeNs(camera_ns);
  xmlFreeNs(device_ns);
  xmlFreeNs(rdf_ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
