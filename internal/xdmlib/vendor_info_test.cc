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

#include "xdmlib/vendor_info.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/vendorinfo/";
const char kAllOptionalFieldsFile[] = "xdm/vendor_info_testdata_full.txt";
const char kMinimumFieldsFile[] = "xdm/vendor_info_testdata.txt";

TEST(VendorInfo, GetNamespaces) {
  std::unordered_map<string, string> ns_name_href_map;
  string prefix(XdmConst::VendorInfo());

  const string manufacturer("manufacturer_1");
  std::unique_ptr<VendorInfo> vendor_info =
      VendorInfo::FromData(manufacturer, "", "");
  ASSERT_NE(nullptr, vendor_info);

  EXPECT_TRUE(ns_name_href_map.empty());
  vendor_info->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(1, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));

  // Same namespace will not be added again.
  vendor_info->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(1, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));
}

TEST(VendorInfo, FromData) {
  const string manufacturer("manufacturer_1");
  const string model("model_1");
  const string notes("notes_1");
  std::unique_ptr<VendorInfo> vendor_info =
      VendorInfo::FromData(manufacturer, model, notes);
  ASSERT_NE(nullptr, vendor_info);
  EXPECT_EQ(manufacturer, vendor_info->GetManufacturer());
  EXPECT_EQ(model, vendor_info->GetModel());
  EXPECT_EQ(notes, vendor_info->GetNotes());
}

TEST(VendorInfo, FromDataEmptyManufacturer) {
  const string model("model_1");
  std::unique_ptr<VendorInfo> vendor_info = VendorInfo::FromData("", model, "");
  ASSERT_EQ(nullptr, vendor_info);
}

TEST(VendorInfo, FromDataRequiredFieldOnly) {
  const string manufacturer("manufacturer_1");
  std::unique_ptr<VendorInfo> vendor_info =
      VendorInfo::FromData(manufacturer, "", "");
  ASSERT_NE(nullptr, vendor_info);
  EXPECT_EQ(manufacturer, vendor_info->GetManufacturer());
  EXPECT_TRUE(vendor_info->GetModel().empty());
  EXPECT_TRUE(vendor_info->GetNotes().empty());
}

TEST(VendorInfo, SerializeWithMinimumFields) {
  const string manufacturer("manufacturer_1");
  std::unique_ptr<VendorInfo> vendor_info =
      VendorInfo::FromData(manufacturer, "", "");
  ASSERT_NE(nullptr, vendor_info);

  // Set up XML nodes.
  const char* camera_name = "Camera";
  const char* namespaceHref = "http://notarealh.ref";
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(camera_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                           ToXmlChar(camera_name)));
  namespaces.emplace(XdmConst::VendorInfo(),
                     xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                              ToXmlChar(XdmConst::VendorInfo())));

  xmlNodePtr camera_node = xmlNewNode(nullptr, ToXmlChar(camera_name));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, camera_node);

  // Create serializer
  SerializerImpl serializer(namespaces, camera_node);
  std::unique_ptr<Serializer> vendor_info_serializer =
      serializer.CreateSerializer(XdmConst::Camera(), XdmConst::VendorInfo());
  ASSERT_NE(nullptr, vendor_info_serializer);
  ASSERT_TRUE(vendor_info->Serialize(vendor_info_serializer.get()));

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

TEST(VendorInfo, SerializeWithAllOptionalFields) {
  const string manufacturer("manufacturer_1");
  const string model("model_1");
  const string notes("notes_1");
  std::unique_ptr<VendorInfo> vendor_info =
      VendorInfo::FromData(manufacturer, model, notes);
  ASSERT_NE(nullptr, vendor_info);

  // Set up XML nodes.
  const char* camera_name = "Camera";
  const char* namespaceHref = "http://notarealh.ref";
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(camera_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                           ToXmlChar(camera_name)));
  namespaces.emplace(XdmConst::VendorInfo(),
                     xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                              ToXmlChar(XdmConst::VendorInfo())));

  xmlNodePtr camera_node = xmlNewNode(nullptr, ToXmlChar(camera_name));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, camera_node);

  // Create serializer
  SerializerImpl serializer(namespaces, camera_node);
  std::unique_ptr<Serializer> vendor_info_serializer =
      serializer.CreateSerializer(XdmConst::Camera(), XdmConst::VendorInfo());
  ASSERT_NE(nullptr, vendor_info_serializer);
  ASSERT_TRUE(vendor_info->Serialize(vendor_info_serializer.get()));

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

TEST(VendorInfo, ReadMetadataMissingRequiredField) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Camera node.
  xmlNodePtr camera_node = xmlNewNode(nullptr, ToXmlChar("Camera"));
  xmlAddChild(description_node, camera_node);

  xmlNsPtr camera_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                                ToXmlChar(XdmConst::Camera()));
  xmlNodePtr vendor_info_node =
      xmlNewNode(camera_ns, ToXmlChar(XdmConst::VendorInfo()));
  xmlAddChild(camera_node, vendor_info_node);

  const string model("model_1");
  xmlNsPtr vendor_info_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                                     ToXmlChar(XdmConst::VendorInfo()));
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Model"),
               ToXmlChar(model.data()));

  // Create a VendorInfo from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<VendorInfo> vendor_info =
      VendorInfo::FromDeserializer(deserializer, XdmConst::Camera());
  ASSERT_EQ(nullptr, vendor_info.get());

  xmlFreeNs(camera_ns);
  xmlFreeNs(vendor_info_ns);
}

TEST(VendorInfo, ReadMetadataMissingOptionalField) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Camera node.
  xmlNodePtr camera_node = xmlNewNode(nullptr, ToXmlChar("Camera"));
  xmlAddChild(description_node, camera_node);

  xmlNsPtr camera_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                                ToXmlChar(XdmConst::Camera()));
  xmlNodePtr vendor_info_node =
      xmlNewNode(camera_ns, ToXmlChar(XdmConst::VendorInfo()));
  xmlAddChild(camera_node, vendor_info_node);

  const string manufacturer("manufacturer_1");
  xmlNsPtr vendor_info_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                                     ToXmlChar(XdmConst::VendorInfo()));
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Manufacturer"),
               ToXmlChar(manufacturer.data()));

  // Create a VendorInfo from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<VendorInfo> vendor_info =
      VendorInfo::FromDeserializer(deserializer, XdmConst::Camera());
  ASSERT_NE(nullptr, vendor_info.get());

  ASSERT_EQ(manufacturer, vendor_info->GetManufacturer());
  ASSERT_TRUE(vendor_info->GetModel().empty());
  ASSERT_TRUE(vendor_info->GetNotes().empty());

  xmlFreeNs(camera_ns);
  xmlFreeNs(vendor_info_ns);
}

TEST(VendorInfo, ReadMetadata) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock a Device node.
  xmlNodePtr device_node = xmlNewNode(nullptr, ToXmlChar("Device"));
  xmlAddChild(description_node, device_node);

  xmlNsPtr device_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                                ToXmlChar(XdmConst::Device()));
  xmlNodePtr vendor_info_node =
      xmlNewNode(device_ns, ToXmlChar(XdmConst::VendorInfo()));
  xmlAddChild(device_node, vendor_info_node);

  const string manufacturer("manufacturer_1");
  const string model("model_1");
  const string notes("notes_1");
  xmlNsPtr vendor_info_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                                     ToXmlChar(XdmConst::VendorInfo()));
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Manufacturer"),
               ToXmlChar(manufacturer.data()));
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Model"),
               ToXmlChar(model.data()));
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Notes"),
               ToXmlChar(notes.data()));

  // Create an VendorInfo from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<VendorInfo> vendor_info =
      VendorInfo::FromDeserializer(deserializer, XdmConst::Device());
  ASSERT_NE(nullptr, vendor_info.get());

  ASSERT_EQ(manufacturer, vendor_info->GetManufacturer());
  ASSERT_EQ(model, vendor_info->GetModel());
  ASSERT_EQ(notes, vendor_info->GetNotes());

  xmlFreeNs(device_ns);
  xmlFreeNs(vendor_info_ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
