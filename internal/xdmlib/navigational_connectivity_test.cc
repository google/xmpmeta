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

#include "xdmlib/navigational_connectivity.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "gmock/gmock.h"
#include "xdmlib/const.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"
#include "xmpmeta/xmp_const.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"

using testing::UnorderedElementsAre;
using xmpmeta::CreateXmpData;
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

const char kNamespaceHref[] =
    "http://ns.xdm.org/photos/1.0/navigationalconnectivity/";
const char kTestDataPath[] = "xdm/navigational_connectivity_testdata.txt";
const std::vector<int> kNavigationalEdges{0, 1, 1, 0};

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
    xmlNodeSetContent(rdf_li_node, ToXmlChar(std::to_string(values[i]).data()));
    xmlAddChild(rdf_seq_node, rdf_li_node);
  }
  return rdf_seq_node;
}

TEST(NavigationalConnectivity, GetNamespaces) {
  std::unique_ptr<NavigationalConnectivity> navigational_connectivity =
      NavigationalConnectivity::FromData({0, 1});
  ASSERT_NE(nullptr, navigational_connectivity);

  std::unordered_map<string, string> ns_name_href_map;
  navigational_connectivity->GetNamespaces(&ns_name_href_map);
  EXPECT_THAT(ns_name_href_map,
              UnorderedElementsAre(std::make_pair(
                  XdmConst::NavigationalConnectivity(), kNamespaceHref)));
}

TEST(NavigationalConnectivity, FromDataEmptyNavigationalEdges) {
  ASSERT_EQ(nullptr, NavigationalConnectivity::FromData({}));
}

TEST(NavigationalConnectivity, FromDataOddNumberNavigationalEdges) {
  ASSERT_EQ(nullptr, NavigationalConnectivity::FromData({1, 2, 3}));
}

TEST(NavigationalConnectivity, FromDataNegativeNavigationalEdges) {
  ASSERT_EQ(nullptr, NavigationalConnectivity::FromData({-1, 2}));
}

TEST(NavigationalConnectivity, FromDataCorrectNavigationalEdges) {
  std::unique_ptr<NavigationalConnectivity> navigational_connectivity =
      NavigationalConnectivity::FromData(kNavigationalEdges);
  ASSERT_NE(nullptr, navigational_connectivity);
  EXPECT_EQ(kNavigationalEdges,
            navigational_connectivity->GetNavigationalEdges());
}

TEST(NavigationalConnectivity, SerializeWithoutRdfPrefix) {
  std::unique_ptr<NavigationalConnectivity> navigational_connectivity =
      NavigationalConnectivity::FromData(kNavigationalEdges);

  // Create XML serializer.
  const char* device_name = XdmConst::Device();
  const char* navigational_connectivity_name =
      XdmConst::NavigationalConnectivity();
  const char* namespaceHref = "http://notarealh.ref";
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, NewNs(namespaceHref, device_name));
  namespaces.emplace(navigational_connectivity_name,
                     NewNs(namespaceHref, navigational_connectivity_name));

  xmlNodePtr device_node = NewNode(device_name);
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  std::unique_ptr<Serializer> navigational_connectivity_serializer =
      serializer.CreateSerializer(
          XdmConst::Namespace(navigational_connectivity_name),
          navigational_connectivity_name);
  ASSERT_NE(nullptr, navigational_connectivity_serializer);

  ASSERT_FALSE(navigational_connectivity->Serialize(
      navigational_connectivity_serializer.get()));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(NavigationalConnectivity, Serialize) {
  std::unique_ptr<NavigationalConnectivity> navigational_connectivity =
      NavigationalConnectivity::FromData(kNavigationalEdges);

  // Create XML serializer.
  const char* device_name = XdmConst::Device();
  const char* navigational_connectivity_name =
      XdmConst::NavigationalConnectivity();
  const char* namespaceHref = "http://notarealh.ref";

  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, NewNs(namespaceHref, device_name));
  namespaces.emplace(navigational_connectivity_name,
                     NewNs(namespaceHref, navigational_connectivity_name));
  namespaces.emplace(XmlConst::RdfPrefix(), NewNs("", XmlConst::RdfPrefix()));

  xmlNodePtr device_node = NewNode(device_name);

  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  std::unique_ptr<Serializer> navigational_connectivity_serializer =
      serializer.CreateSerializer(
          XdmConst::Namespace(navigational_connectivity_name),
          navigational_connectivity_name);
  ASSERT_NE(nullptr, navigational_connectivity_serializer);

  ASSERT_TRUE(navigational_connectivity->Serialize(
      navigational_connectivity_serializer.get()));

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

TEST(NavigationalConnectivity, ReadMetadataMissingRequiredField) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Device node.
  xmlNodePtr device_node = NewNode("Device");
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = NewNs(kNamespaceHref, XdmConst::Device());
  xmlNodePtr navigational_connectivity_node =
      xmlNewNode(device_ns, ToXmlChar(XdmConst::NavigationalConnectivity()));
  xmlAddChild(device_node, navigational_connectivity_node);

  // Create a NavigationalConnectivity from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<NavigationalConnectivity> navigational_connectivity =
      NavigationalConnectivity::FromDeserializer(deserializer);
  ASSERT_EQ(nullptr, navigational_connectivity.get());

  xmlFreeNs(device_ns);
}

TEST(NavigationalConnectivity, ReadMetadata) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Device node.
  xmlNodePtr device_node = NewNode("Device");
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = NewNs(kNamespaceHref, XdmConst::Device());
  xmlNodePtr navigational_connectivity_node =
      xmlNewNode(device_ns, ToXmlChar(XdmConst::NavigationalConnectivity()));
  xmlAddChild(device_node, navigational_connectivity_node);

  xmlNsPtr navigational_connectivity_ns =
      NewNs(kNamespaceHref, XdmConst::NavigationalConnectivity());
  xmlNodePtr navigational_edges_node =
      xmlNewNode(navigational_connectivity_ns, ToXmlChar("NavigationalEdges"));
  xmlAddChild(navigational_connectivity_node, navigational_edges_node);

  xmlNodePtr rdf_seq_node = SetupRdfSeqOfIndices(kNavigationalEdges);
  xmlAddChild(navigational_edges_node, rdf_seq_node);

  // Create a NavigationalConnectivity from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<NavigationalConnectivity> navigational_connectivity =
      NavigationalConnectivity::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, navigational_connectivity.get());
  EXPECT_EQ(kNavigationalEdges,
            navigational_connectivity->GetNavigationalEdges());

  xmlFreeNs(device_ns);
  xmlFreeNs(navigational_connectivity_ns);
  xmlFreeNs(rdf_seq_node->ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
