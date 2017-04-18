// Copyright 2017 The XMPMeta Authors. All Rights Reserved.
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

#include "xdmlib/mesh.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "strings/numbers.h"
#include "xdmlib/const.h"
#include "xmpmeta/base64.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"

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

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/mesh/";
const char kAllOptionalFieldsFile[] = "xdm/mesh_testdata_full.txt";
const char kMinimumFieldsFile[] = "xdm/mesh_testdata.txt";
// Mesh with 4 vertices and 4 faces.
const int kVertexCount = 4;
const std::vector<float> kVertexPosition{0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                                         0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
const int kFaceCount = 4;
const std::vector<int> kFaceIndices{0, 1, 2, 0, 1, 3, 0, 3, 2, 1, 2, 3};
const char kSoftware[] = "software";

TEST(Mesh, GetNamespaces) {
  std::unordered_map<string, string> ns_name_href_map;
  string prefix(XdmConst::Mesh());

  std::unique_ptr<Mesh> mesh = Mesh::FromData(
      kVertexCount, kVertexPosition, kFaceCount, kFaceIndices, false, "");
  ASSERT_NE(nullptr, mesh);

  EXPECT_TRUE(ns_name_href_map.empty());
  mesh->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(1, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));

  // Same namespace will not be added again.
  mesh->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(1, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));
}

TEST(Mesh, FromData) {
  std::unique_ptr<Mesh> mesh =
      Mesh::FromData(kVertexCount, kVertexPosition, kFaceCount, kFaceIndices,
                     false, kSoftware);
  ASSERT_NE(nullptr, mesh);
  EXPECT_EQ(kVertexCount, mesh->GetVertexCount());
  EXPECT_EQ(kVertexPosition, mesh->GetVertexPosition());
  EXPECT_EQ(kFaceCount, mesh->GetFaceCount());
  EXPECT_EQ(kFaceIndices, mesh->GetFaceIndices());
  EXPECT_FALSE(mesh->GetMetric());
  EXPECT_EQ(kSoftware, mesh->GetSoftware());
}

TEST(Mesh, FromDataEmptyVertexPosition) {
  std::unique_ptr<Mesh> mesh = Mesh::FromData(kVertexCount, {}, kFaceCount,
                                              kFaceIndices, false, kSoftware);
  ASSERT_EQ(nullptr, mesh);
}

TEST(Mesh, FromDataRequiredFieldsOnly) {
  std::unique_ptr<Mesh> mesh = Mesh::FromData(
      kVertexCount, kVertexPosition, kFaceCount, kFaceIndices, false, "");
  ASSERT_NE(nullptr, mesh);
  EXPECT_EQ(kVertexCount, mesh->GetVertexCount());
  EXPECT_EQ(kVertexPosition, mesh->GetVertexPosition());
  EXPECT_EQ(kFaceCount, mesh->GetFaceCount());
  EXPECT_EQ(kFaceIndices, mesh->GetFaceIndices());
  EXPECT_FALSE(mesh->GetMetric());
  EXPECT_TRUE(mesh->GetSoftware().empty());
}

TEST(Mesh, SerializeWithMinimumFields) {
  std::unique_ptr<Mesh> mesh = Mesh::FromData(
      kVertexCount, kVertexPosition, kFaceCount, kFaceIndices, false, "");
  ASSERT_NE(nullptr, mesh);

  // Set up XML nodes.
  const char* device_name = "Device";
  const char* namespaceHref = "http://notarealh.ref";
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                           ToXmlChar(device_name)));
  namespaces.emplace(
      XdmConst::Mesh(),
      xmlNewNs(nullptr, ToXmlChar(namespaceHref), ToXmlChar(XdmConst::Mesh())));

  xmlNodePtr device_node = xmlNewNode(nullptr, ToXmlChar(device_name));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer
  SerializerImpl serializer(namespaces, device_node);
  std::unique_ptr<Serializer> mesh_serializer = serializer.CreateSerializer(
      XdmConst::Namespace(XdmConst::Mesh()), XdmConst::Mesh());
  ASSERT_NE(nullptr, mesh_serializer);
  ASSERT_TRUE(mesh->Serialize(mesh_serializer.get()));

  // Check the XML string.
  const string testdata_path = TestFileAbsolutePath(kMinimumFieldsFile);
  std::string expected_data;
  ReadFileToStringOrDie(testdata_path, &expected_data);
  ASSERT_EQ(expected_data, XmlDocToString(xml_doc));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Mesh, SerializeWithAllOptionalFields) {
  std::unique_ptr<Mesh> mesh =
      Mesh::FromData(kVertexCount, kVertexPosition, kFaceCount, kFaceIndices,
                     false, kSoftware);
  ASSERT_NE(nullptr, mesh);

  // Set up XML nodes.
  const char* device_name = "Device";
  const char* namespaceHref = "http://notarealh.ref";
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                           ToXmlChar(device_name)));
  namespaces.emplace(
      XdmConst::Mesh(),
      xmlNewNs(nullptr, ToXmlChar(namespaceHref), ToXmlChar(XdmConst::Mesh())));

  xmlNodePtr device_node = xmlNewNode(nullptr, ToXmlChar(device_name));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer
  SerializerImpl serializer(namespaces, device_node);
  std::unique_ptr<Serializer> mesh_serializer = serializer.CreateSerializer(
      XdmConst::Namespace(XdmConst::Mesh()), XdmConst::Mesh());
  ASSERT_NE(nullptr, mesh_serializer);
  ASSERT_TRUE(mesh->Serialize(mesh_serializer.get()));

  // Check the XML string.
  const string testdata_path = TestFileAbsolutePath(kAllOptionalFieldsFile);
  std::string expected_data;
  ReadFileToStringOrDie(testdata_path, &expected_data);
  ASSERT_EQ(expected_data, XmlDocToString(xml_doc));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Mesh, ReadMetadataMissingRequiredField) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock an Device node.
  xmlNodePtr device_node = xmlNewNode(nullptr, ToXmlChar("Device"));
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                                ToXmlChar(XdmConst::Device()));
  xmlNodePtr mesh_node = xmlNewNode(device_ns, ToXmlChar(XdmConst::Mesh()));
  xmlAddChild(device_node, mesh_node);

  string base64_encoded_vertex_position;
  ASSERT_TRUE(
      EncodeFloatArrayBase64(kVertexPosition, &base64_encoded_vertex_position));

  xmlNsPtr mesh_ns =
      xmlNewNs(nullptr, ToXmlChar(kNamespaceHref), ToXmlChar(XdmConst::Mesh()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("VertexPosition"),
               ToXmlChar(base64_encoded_vertex_position.data()));

  // Create an Mesh from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Mesh> mesh = Mesh::FromDeserializer(deserializer);
  ASSERT_EQ(nullptr, mesh.get());

  xmlFreeNs(device_ns);
  xmlFreeNs(mesh_ns);
}

TEST(Mesh, ReadMetadataMissingOptionalField) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock an Device node.
  xmlNodePtr device_node = xmlNewNode(nullptr, ToXmlChar("Device"));
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                                ToXmlChar(XdmConst::Device()));
  xmlNodePtr mesh_node = xmlNewNode(device_ns, ToXmlChar(XdmConst::Mesh()));
  xmlAddChild(device_node, mesh_node);

  string base64_encoded_vertex_position;
  ASSERT_TRUE(
      EncodeFloatArrayBase64(kVertexPosition, &base64_encoded_vertex_position));
  string base64_encoded_face_indices;
  ASSERT_TRUE(EncodeIntArrayBase64(kFaceIndices, &base64_encoded_face_indices));

  xmlNsPtr mesh_ns =
      xmlNewNs(nullptr, ToXmlChar(kNamespaceHref), ToXmlChar(XdmConst::Mesh()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("VertexCount"),
               ToXmlChar(SimpleItoa(kVertexCount).c_str()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("VertexPosition"),
               ToXmlChar(base64_encoded_vertex_position.data()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("FaceCount"),
               ToXmlChar(SimpleItoa(kFaceCount).c_str()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("FaceIndices"),
               ToXmlChar(base64_encoded_face_indices.data()));

  // Create an Mesh from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Mesh> mesh = Mesh::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, mesh.get());

  ASSERT_EQ(kVertexCount, mesh->GetVertexCount());
  ASSERT_EQ(kVertexPosition, mesh->GetVertexPosition());
  ASSERT_EQ(kFaceCount, mesh->GetFaceCount());
  ASSERT_EQ(kFaceIndices, mesh->GetFaceIndices());
  ASSERT_TRUE(mesh->GetSoftware().empty());

  // Parses to the default value when not set.
  ASSERT_FALSE(mesh->GetMetric());

  xmlFreeNs(device_ns);
  xmlFreeNs(mesh_ns);
}

TEST(Mesh, ReadMetadata) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock an Device node.
  xmlNodePtr device_node = xmlNewNode(nullptr, ToXmlChar("Device"));
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                                ToXmlChar(XdmConst::Device()));
  xmlNodePtr mesh_node = xmlNewNode(device_ns, ToXmlChar(XdmConst::Mesh()));
  xmlAddChild(device_node, mesh_node);

  string base64_encoded_vertex_position;
  ASSERT_TRUE(
      EncodeFloatArrayBase64(kVertexPosition, &base64_encoded_vertex_position));
  string base64_encoded_face_indices;
  ASSERT_TRUE(EncodeIntArrayBase64(kFaceIndices, &base64_encoded_face_indices));

  xmlNsPtr mesh_ns =
      xmlNewNs(nullptr, ToXmlChar(kNamespaceHref), ToXmlChar(XdmConst::Mesh()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("VertexCount"),
               ToXmlChar(SimpleItoa(kVertexCount).c_str()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("VertexPosition"),
               ToXmlChar(base64_encoded_vertex_position.data()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("FaceCount"),
               ToXmlChar(SimpleItoa(kFaceCount).c_str()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("FaceIndices"),
               ToXmlChar(base64_encoded_face_indices.data()));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("Metric"), ToXmlChar("true"));
  xmlSetNsProp(mesh_node, mesh_ns, ToXmlChar("Software"), ToXmlChar(kSoftware));

  // Create an Mesh from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Mesh> mesh = Mesh::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, mesh.get());

  ASSERT_EQ(kVertexCount, mesh->GetVertexCount());
  ASSERT_EQ(kVertexPosition, mesh->GetVertexPosition());
  ASSERT_EQ(kFaceCount, mesh->GetFaceCount());
  ASSERT_EQ(kFaceIndices, mesh->GetFaceIndices());
  ASSERT_EQ(kSoftware, mesh->GetSoftware());

  // Parses to the default value when not set.
  ASSERT_TRUE(mesh->GetMetric());

  xmlFreeNs(device_ns);
  xmlFreeNs(mesh_ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
