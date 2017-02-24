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

#include "xdmlib/profile.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "gmock/gmock.h"
#include "xdmlib/const.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xmp_const.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"
#include "xmpmeta/xml/const.h"

using testing::ElementsAre;
using testing::UnorderedElementsAre;
using xmpmeta::CreateXmpData;
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

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/profile/";
const char kTestDataPath[] = "xdm/profile_testdata.txt";

// Convenience function for creating an XML node.
xmlNodePtr NewNode(const string& node_name) {
  return xmlNewNode(nullptr, ToXmlChar(node_name.data()));
}

// Convenience function for creating an XML namespace.
xmlNsPtr NewNs(const string& href, const string& ns_name) {
  return xmlNewNs(nullptr, href.empty() ? nullptr : ToXmlChar(href.data()),
                  ToXmlChar(ns_name.data()));
}

// Returns an XML rdf:Seq node with a list of the given values.
xmlNodePtr SetupRdfSeqOfIndices(const std::vector<int>& values) {
  xmlNsPtr rdf_ns = NewNs("http://fakeh.ref", XmlConst::RdfPrefix());
  xmlNodePtr rdf_seq_node = xmlNewNode(rdf_ns, ToXmlChar(XmlConst::RdfSeq()));
  for (int i = 0; i < values.size(); i++) {
    xmlNodePtr rdf_li_node = xmlNewNode(rdf_ns, ToXmlChar(XmlConst::RdfLi()));
    xmlNodeSetContent(rdf_li_node,
                      ToXmlChar(std::to_string(values[i]).data()));
    xmlAddChild(rdf_seq_node, rdf_li_node);
  }
  return rdf_seq_node;
}

TEST(Profile, GetNamespaces) {
  std::unique_ptr<Profile> profile = Profile::FromData("VRPhoto", {0, 1});
  ASSERT_NE(nullptr, profile);

  std::unordered_map<string, string> ns_name_href_map;
  profile->GetNamespaces(&ns_name_href_map);
  EXPECT_THAT(ns_name_href_map,
              UnorderedElementsAre(std::make_pair(XdmConst::Profile(),
                                                  kNamespaceHref)));
}

TEST(Profile, FromDataEmptyType) {
  ASSERT_EQ(nullptr, Profile::FromData("", {0, 1}));
}

TEST(Profile, FromDataSupportedTypesVRPhotoEmptyIndices) {
  // Bad indices for VR photo type.
  ASSERT_EQ(nullptr, Profile::FromData("VRPhoto", {}));
}

TEST(Profile, FromDataSupportedTypesVRPhotoIndicesTooShort) {
  ASSERT_EQ(nullptr, Profile::FromData("VRPhoto", {0}));
}

TEST(Profile, FromDataSupportedTypesVRPhotoWrongCase) {
  // Case-sensitive, so the wrong casing is treated as a unsupported type.
  std::unique_ptr<Profile> profile = Profile::FromData("VrPhoto", {0});
  ASSERT_NE(nullptr, profile);
  EXPECT_EQ(string("VrPhoto"), profile->GetType());
  EXPECT_EQ(1, profile->GetCameraIndices().size());
  EXPECT_THAT(profile->GetCameraIndices(), ElementsAre(0));
}

TEST(Profile, FromDataSupportedTypesDepthPhotoEmptyIndices) {
  // Empty indices for Depth photo type.
  ASSERT_EQ(nullptr, Profile::FromData("DepthPhoto", {}));
}

TEST(Profile, FromDataVRPhotoExcessIndices) {
  // Excess indices have no effect on profile validity.
  // VR photo.
  std::vector<int> indices = {0, 1, 2, 3};
  std::unique_ptr<Profile> profile = Profile::FromData("VRPhoto", indices);
  ASSERT_NE(nullptr, profile);
  EXPECT_EQ("VRPhoto", profile->GetType());
  EXPECT_EQ(indices, profile->GetCameraIndices());
}


TEST(Profile, FromDataDepthPhotoExcessIndices) {
  std::vector<int> indices = {0, 1, 2, 3};
  std::unique_ptr<Profile> profile = Profile::FromData("DepthPhoto", indices);
  ASSERT_NE(nullptr, profile);
  EXPECT_EQ("DepthPhoto", profile->GetType());
  EXPECT_EQ(indices, profile->GetCameraIndices());
}

TEST(Profile, FromDataVRPhoto) {
  string type("VRPhoto");
  std::vector<int> indices = {0, 1};
  std::unique_ptr<Profile> profile = Profile::FromData(type, indices);
  ASSERT_NE(nullptr, profile);
  EXPECT_EQ(type, profile->GetType());
  EXPECT_EQ(indices, profile->GetCameraIndices());
}

TEST(Profile, FromDataDepthPhoto) {
  string type = "DepthPhoto";
  std::vector<int> indices = {0};
  std::unique_ptr<Profile> profile = Profile::FromData(type, indices);
  ASSERT_NE(nullptr, profile);
  EXPECT_EQ(type, profile->GetType());
  EXPECT_EQ(indices, profile->GetCameraIndices());
}

TEST(Profile, SerializeWithoutRdfPrefix) {
  const string type("VRPhoto");
  const std::vector<int> indices = {0, 1};
  std::unique_ptr<Profile> profile = Profile::FromData(type, indices);

  // Create XML serializer.
  const char* device_name = XdmConst::Device();
  const char* profile_name = XdmConst::Profile();
  const char* namespaceHref = "http://notarealh.ref";
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, NewNs(namespaceHref, device_name));
  namespaces.emplace(profile_name, NewNs(namespaceHref, profile_name));

  xmlNodePtr device_node = NewNode(device_name);
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  std::unique_ptr<Serializer> profile_serializer =
      serializer.CreateSerializer(XdmConst::Namespace(profile_name),
                                  profile_name);
  ASSERT_NE(nullptr, profile_serializer);

  ASSERT_FALSE(profile->Serialize(profile_serializer.get()));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Profile, Serialize) {
  const string type("VRPhoto");
  const std::vector<int> indices = {0, 1};
  std::unique_ptr<Profile> profile = Profile::FromData(type, indices);

  // Create XML serializer.
  const char* device_name = XdmConst::Device();
  const char* profile_name = XdmConst::Profile();
  const char* namespaceHref = "http://notarealh.ref";

  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, NewNs(namespaceHref, device_name));
  namespaces.emplace(profile_name, NewNs(namespaceHref, profile_name));
  namespaces.emplace(XmlConst::RdfPrefix(), NewNs("", XmlConst::RdfPrefix()));

  xmlNodePtr device_node = NewNode(device_name);

  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  std::unique_ptr<Serializer> profile_serializer =
      serializer.CreateSerializer(XdmConst::Namespace(profile_name),
                                  profile_name);
  ASSERT_NE(nullptr, profile_serializer);

  ASSERT_TRUE(profile->Serialize(profile_serializer.get()));

  const string testdata_path = TestFileAbsolutePath(kTestDataPath);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(testdata_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xml_doc));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Profile, ReadMetadataNoIndicesUnsupportedType) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Device node.
  xmlNodePtr device_node = NewNode("Device");
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = NewNs(kNamespaceHref, XdmConst::Device());
  xmlNodePtr profile_node =
      xmlNewNode(device_ns, ToXmlChar(XdmConst::Profile()));
  xmlAddChild(device_node, profile_node);

  xmlNodePtr camera_indices_node = NewNode("CameraIndices");
  xmlAddChild(profile_node, camera_indices_node);

  const string type("Supercalifragilisticexpialidocious");
  xmlNsPtr profile_ns = NewNs(kNamespaceHref, XdmConst::Profile());
  xmlSetNsProp(profile_node, profile_ns, ToXmlChar("Type"),
               ToXmlChar(type.data()));

  // Create a Profile from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Profile> profile = Profile::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, profile.get());
  EXPECT_EQ(type, profile->GetType());
  EXPECT_TRUE(profile->GetCameraIndices().empty());

  xmlFreeNs(device_ns);
  xmlFreeNs(profile_ns);
}

TEST(Profile, ReadMetadataEmptyIndicesForDepthPhoto) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Device node.
  xmlNodePtr device_node = NewNode("Device");
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = NewNs(kNamespaceHref, XdmConst::Device());
  xmlNodePtr profile_node =
      xmlNewNode(device_ns, ToXmlChar(XdmConst::Profile()));
  xmlAddChild(device_node, profile_node);

  xmlNsPtr profile_ns = NewNs(kNamespaceHref, XdmConst::Profile());
  xmlNodePtr camera_indices_node =
      xmlNewNode(profile_ns, ToXmlChar("CameraIndices"));
  xmlAddChild(profile_node, camera_indices_node);

  xmlNodePtr rdf_seq_node = SetupRdfSeqOfIndices(std::vector<int>());
  xmlAddChild(camera_indices_node, rdf_seq_node);

  xmlSetNsProp(profile_node, profile_ns, ToXmlChar("Type"),
               ToXmlChar("DepthPhoto"));

  // Create a Profile from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Profile> profile = Profile::FromDeserializer(deserializer);
  ASSERT_EQ(nullptr, profile.get());

  xmlFreeNs(device_ns);
  xmlFreeNs(profile_ns);
  xmlFreeNs(rdf_seq_node->ns);
}

TEST(Profile, ReadMetadataNoIndicesListForDepthPhoto) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Device node.
  xmlNodePtr device_node = NewNode("Device");
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = NewNs(kNamespaceHref, XdmConst::Device());
  xmlNodePtr profile_node =
      xmlNewNode(device_ns, ToXmlChar(XdmConst::Profile()));
  xmlAddChild(device_node, profile_node);

  xmlNsPtr profile_ns = NewNs(kNamespaceHref, XdmConst::Profile());
  xmlNodePtr camera_indices_node =
      xmlNewNode(profile_ns, ToXmlChar("CameraIndices"));
  xmlAddChild(profile_node, camera_indices_node);

  xmlSetNsProp(profile_node, profile_ns, ToXmlChar("Type"),
               ToXmlChar("DepthPhoto"));

  // Create a Profile from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Profile> profile = Profile::FromDeserializer(deserializer);
  ASSERT_EQ(nullptr, profile.get());

  xmlFreeNs(device_ns);
  xmlFreeNs(profile_ns);
}

TEST(Profile, ReadMetadataValidDepthPhotoTooMayIndices) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Device node.
  xmlNodePtr device_node = NewNode("Device");
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = NewNs(kNamespaceHref, XdmConst::Device());
  xmlNodePtr profile_node =
      xmlNewNode(device_ns, ToXmlChar(XdmConst::Profile()));
  xmlAddChild(device_node, profile_node);

  xmlNsPtr profile_ns = NewNs(kNamespaceHref, XdmConst::Profile());
  xmlNodePtr camera_indices_node =
      xmlNewNode(profile_ns, ToXmlChar("CameraIndices"));
  xmlAddChild(profile_node, camera_indices_node);

  const std::vector<int> indices = {0, 1};
  xmlNodePtr rdf_seq_node = SetupRdfSeqOfIndices(indices);
  xmlAddChild(camera_indices_node, rdf_seq_node);

  const string type("DepthPhoto");
  xmlSetNsProp(profile_node, profile_ns, ToXmlChar("Type"),
               ToXmlChar(type.data()));

  // Create a Profile from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Profile> profile = Profile::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, profile.get());
  EXPECT_EQ(type, profile->GetType());
  EXPECT_EQ(indices, profile->GetCameraIndices());

  xmlFreeNs(device_ns);
  xmlFreeNs(profile_ns);
  xmlFreeNs(rdf_seq_node->ns);
}

TEST(Profile, ReadMetadataValidVrPhoto) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Device node.
  xmlNodePtr device_node = NewNode("Device");
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = NewNs(kNamespaceHref, XdmConst::Device());
  xmlNodePtr profile_node =
      xmlNewNode(device_ns, ToXmlChar(XdmConst::Profile()));
  xmlAddChild(device_node, profile_node);

  xmlNsPtr profile_ns = NewNs(kNamespaceHref, XdmConst::Profile());
  xmlNodePtr camera_indices_node =
      xmlNewNode(profile_ns, ToXmlChar("CameraIndices"));
  xmlAddChild(profile_node, camera_indices_node);

  const std::vector<int> indices = {0, 1};
  xmlNodePtr rdf_seq_node = SetupRdfSeqOfIndices(indices);
  xmlAddChild(camera_indices_node, rdf_seq_node);

  const string type("VRPhoto");
  xmlSetNsProp(profile_node, profile_ns, ToXmlChar("Type"),
               ToXmlChar(type.data()));

  // Create a Profile from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Profile> profile = Profile::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, profile.get());
  EXPECT_EQ(type, profile->GetType());
  EXPECT_EQ(indices, profile->GetCameraIndices());

  xmlFreeNs(device_ns);
  xmlFreeNs(profile_ns);
  xmlFreeNs(rdf_seq_node->ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
