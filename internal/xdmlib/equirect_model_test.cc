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

#include "xdmlib/equirect_model.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "gmock/gmock.h"
#include "xdmlib/const.h"
#include "xdmlib/dimension.h"
#include "xdmlib/point.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/utils.h"

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

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/equirectmodel/";
const char kNodeNamespaceHref[] = "http://ns.xdm.org/photos/1.0/imagingmodel/";
const char kEquirectModelDataPath[] = "xdm/equirect_model_testdata.txt";

TEST(EquirectModel, GetNamespaces) {
  std::unordered_map<string, string> ns_name_href_map;
  string prefix(XdmConst::EquirectModel());
  const Point cropped_origin(0, 1530);
  const Dimension cropped_size(3476, 1355);
  const Dimension full_size(8192, 4096);

  std::unique_ptr<EquirectModel> model =
      EquirectModel::FromData(cropped_origin, cropped_size, full_size);
  ASSERT_NE(nullptr, model);

  ASSERT_TRUE(ns_name_href_map.empty());
  model->GetNamespaces(&ns_name_href_map);
  EXPECT_THAT(ns_name_href_map,
              UnorderedElementsAre(std::make_pair(XdmConst::ImagingModel(),
                                                  kNodeNamespaceHref),
                                   std::make_pair(XdmConst::EquirectModel(),
                                                  kNamespaceHref)));
}

TEST(EquirectModel, FromData) {
  const Point cropped_origin(0, 1530);
  const Dimension cropped_size(3476, 1355);
  const Dimension full_size(8192, 4096);

  std::unique_ptr<EquirectModel> model =
      EquirectModel::FromData(cropped_origin, cropped_size, full_size);
  ASSERT_NE(nullptr, model);

  Point values = model->GetCroppedOrigin();
  EXPECT_EQ(0, values.x);
  EXPECT_EQ(1530, values.y);

  Dimension dimension = model->GetCroppedSize();
  EXPECT_EQ(cropped_size.width, dimension.width);
  EXPECT_EQ(cropped_size.height, dimension.height);

  dimension = model->GetFullSize();
  EXPECT_EQ(full_size.width, dimension.width);
  EXPECT_EQ(full_size.height, dimension.height);
}

TEST(EquirectModel, Serialize) {
  const Point cropped_origin(0, 1530);
  const Dimension cropped_size(3476, 1355);
  const Dimension full_size(8192, 4096);

  std::unique_ptr<EquirectModel> model =
      EquirectModel::FromData(cropped_origin, cropped_size, full_size);
  ASSERT_NE(nullptr, model);

  // Create XML serializer.
  const char* device_name = XdmConst::Device();
  const char* camera_name = XdmConst::Camera();
  const char* imaging_model_name = XdmConst::ImagingModel();
  const char* model_name = XdmConst::EquirectModel();
  const char* namespaceHref = "http://notarealh.ref";
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                           ToXmlChar(device_name)));
  namespaces.emplace(camera_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                           ToXmlChar(camera_name)));
  namespaces.emplace(model_name, xmlNewNs(nullptr, ToXmlChar(namespaceHref),
                                          ToXmlChar(model_name)));
  xmlNodePtr device_node = xmlNewNode(nullptr, ToXmlChar(device_name));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  std::unique_ptr<Serializer> camera_serializer =
      serializer.CreateSerializer(XdmConst::Namespace(camera_name),
                                  camera_name);
  ASSERT_NE(nullptr, camera_serializer);

  std::unique_ptr<Serializer> imaging_model_serializer =
      camera_serializer->CreateSerializer(
          XdmConst::Namespace(imaging_model_name), imaging_model_name);;
  ASSERT_NE(nullptr, imaging_model_serializer);

  std::unique_ptr<Serializer> model_serializer =
      imaging_model_serializer->CreateSerializer(
          XdmConst::Namespace(XdmConst::EquirectModel()),
          XdmConst::EquirectModel());
  ASSERT_NE(nullptr, model_serializer);

  ASSERT_TRUE(model->Serialize(model_serializer.get()));

  const string xdm_model_data_path =
      TestFileAbsolutePath(kEquirectModelDataPath);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(xdm_model_data_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xml_doc));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(EquirectModel, ReadMetadata) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Mock an XdmCamera node.
  xmlNodePtr camera_node = xmlNewNode(nullptr, ToXmlChar(XdmConst::Camera()));
  xmlAddChild(description_node, camera_node);

  string prefix(XdmConst::EquirectModel());
  xmlNsPtr camera_ns = xmlNewNs(nullptr, ToXmlChar("http://fakeh.ref"),
                                ToXmlChar(XdmConst::Camera()));
  xmlNodePtr model_node = xmlNewNode(camera_ns, ToXmlChar(prefix.data()));
  xmlAddChild(camera_node, model_node);

  xmlNsPtr model_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                               ToXmlChar(prefix.data()));
  xmlSetNsProp(model_node, model_ns, ToXmlChar("CroppedAreaLeftPixels"),
               ToXmlChar("0"));
  xmlSetNsProp(model_node, model_ns, ToXmlChar("CroppedAreaTopPixels"),
               ToXmlChar("1530"));
  xmlSetNsProp(model_node, model_ns, ToXmlChar("CroppedAreaImageWidthPixels"),
               ToXmlChar("3476"));
  xmlSetNsProp(model_node, model_ns, ToXmlChar("CroppedAreaImageHeightPixels"),
               ToXmlChar("1355"));
  xmlSetNsProp(model_node, model_ns, ToXmlChar("FullImageWidthPixels"),
               ToXmlChar("8192"));
  xmlSetNsProp(model_node, model_ns, ToXmlChar("FullImageHeightPixels"),
               ToXmlChar("4096"));

  // Create an EquirectModel from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<EquirectModel> model =
      EquirectModel::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, model.get());

  Point values = model->GetCroppedOrigin();
  EXPECT_EQ(0, values.x);
  EXPECT_EQ(1530, values.y);

  Dimension dimension = model->GetCroppedSize();
  EXPECT_EQ(3476, dimension.width);
  EXPECT_EQ(1355, dimension.height);

  dimension = model->GetFullSize();
  EXPECT_EQ(8192, dimension.width);
  EXPECT_EQ(4096, dimension.height);

  xmlFreeNs(camera_ns);
  xmlFreeNs(model_ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
