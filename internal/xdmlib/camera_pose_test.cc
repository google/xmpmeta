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

#include "xdmlib/camera_pose.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/port.h"
#include "xdmlib/const.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xmp_const.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/utils.h"

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

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/camerapose/";
const char kFakeHref[] = "http://notarealh.ref";

const char kTestDataFull[] = "xdm/camera_pose_testdata_full.txt";
const char kTestDataPosition[] = "xdm/camera_pose_testdata_position.txt";
const char kTestDataOrientation[] =
    "xdm/camera_pose_testdata_orientation.txt";

// Same as NormalizeAxisAngle in camera_pose.cc
const std::vector<double>
NormalizeAxisAngle(const std::vector<double>& coords) {
  if (coords.size() < 4) {
    return std::vector<double>();
  }
  double length = sqrt((coords[0] * coords[0]) +
                       (coords[1] * coords[1]) +
                       (coords[2] * coords[2]));
  const std::vector<double> normalized =
      { coords[0] / length, coords[1] / length, coords[2] / length, coords[3] };
  return normalized;
}

std::unordered_map<string, xmlNsPtr> SetupNamespaces() {
  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(XdmConst::Camera(), xmlNewNs(nullptr, ToXmlChar(kFakeHref),
                                           ToXmlChar(XdmConst::Camera())));
  namespaces.emplace(XdmConst::CameraPose(),
                     xmlNewNs(nullptr, ToXmlChar(kFakeHref),
                              ToXmlChar(XdmConst::CameraPose())));
  return namespaces;
}

void FreeNamespaces(std::unordered_map<string, xmlNsPtr> xml_ns_map) {
  for (const auto& entry : xml_ns_map) {
    xmlFreeNs(entry.second);
  }
}

TEST(CameraPose, GetNamespaces) {
  std::unordered_map<string, string> ns_name_href_map;
  string prefix(XdmConst::CameraPose());

  double x = -85.32;
  double y = -135.20341;
  double z = 1.203;
  std::vector<double> init_position { x, y, z };
  std::unique_ptr<CameraPose> pose =
      CameraPose::FromData(init_position, std::vector<double>(), 0);
  ASSERT_NE(nullptr, pose);

  ASSERT_TRUE(ns_name_href_map.empty());
  pose->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(1, ns_name_href_map.size());
  EXPECT_EQ(kNamespaceHref, ns_name_href_map[prefix]);
}

TEST(CameraPose, FromDataAllEmpty) {
  std::unique_ptr<CameraPose> pose =
      CameraPose::FromData(std::vector<double>(), std::vector<double>(), 0);
  ASSERT_EQ(nullptr, pose);
}

TEST(CameraPose, FromDataPosition) {
  double x = -85.32;
  double y = -135.20341;
  double z = 1.203;
  std::vector<double> init_position = { x, y, z };

  std::unique_ptr<CameraPose> pose =
      CameraPose::FromData(init_position, std::vector<double>(), 0);
  ASSERT_NE(nullptr, pose);

  std::vector<double> position = pose->GetPositionXYZ();
  ASSERT_TRUE(pose->HasPosition());
  ASSERT_EQ(3, position.size());
  EXPECT_EQ(init_position.at(0), position.at(0));
  EXPECT_EQ(init_position.at(1), position.at(1));
  EXPECT_EQ(init_position.at(2), position.at(2));

  std::vector<double> orientation = pose->GetOrientationRotationXYZAngle();
  ASSERT_FALSE(pose->HasOrientation());
  ASSERT_EQ(0, orientation.size());

  ASSERT_EQ(0, pose->GetTimestamp());
}

TEST(CameraPose, FromOrientation) {
  double axis_x = 0;
  double axis_y = 0;
  double axis_z = 1;
  double angle = M_PI_2;
  int64 timestamp = 1455818790;
  std::vector<double> init_orientation = { axis_x, axis_y, axis_z, angle };
  std::unique_ptr<CameraPose> pose =
      CameraPose::FromData(std::vector<double>(), init_orientation,
                              timestamp);
  ASSERT_NE(nullptr, pose);

  std::vector<double> orientation = pose->GetOrientationRotationXYZAngle();
  ASSERT_EQ(4, orientation.size());
  const std::vector<double> normalized = NormalizeAxisAngle(init_orientation);
  EXPECT_EQ(normalized.at(0), orientation.at(0));
  EXPECT_EQ(normalized.at(1), orientation.at(1));
  EXPECT_EQ(normalized.at(2), orientation.at(2));
  EXPECT_EQ(angle, orientation.at(3));

  std::vector<double> position = pose->GetPositionXYZ();
  ASSERT_FALSE(pose->HasPosition());
  ASSERT_EQ(0, position.size());

  ASSERT_EQ(timestamp, pose->GetTimestamp());
}

TEST(CameraPose, FromPositionAndOrientation) {
  double x = -85.32;
  double y = -135.20341;
  double z = 1.203;
  double axis_x = 0;
  double axis_y = 0;
  double axis_z = 1;
  double angle = M_PI_2;
  int64 timestamp = 1455818790;

  std::vector<double> init_position = { x, y, z };
  std::vector<double> init_orientation = { axis_x, axis_y, axis_z, angle };
  std::unique_ptr<CameraPose> pose =
      CameraPose::FromData(init_position, init_orientation, timestamp);
  ASSERT_NE(nullptr, pose);
  std::vector<double> orientation = pose->GetOrientationRotationXYZAngle();
  ASSERT_TRUE(pose->HasPosition());
  ASSERT_EQ(4, orientation.size());
  std::vector<double> normalized = NormalizeAxisAngle(init_orientation);
  EXPECT_EQ(normalized.at(0), orientation.at(0));
  EXPECT_EQ(normalized.at(1), orientation.at(1));
  EXPECT_EQ(normalized.at(2), orientation.at(2));
  EXPECT_EQ(angle, orientation.at(3));

  std::vector<double> position = pose->GetPositionXYZ();
  ASSERT_TRUE(pose->HasOrientation());
  ASSERT_EQ(3, position.size());
  EXPECT_EQ(init_position.at(0), position.at(0));
  EXPECT_EQ(init_position.at(1), position.at(1));
  EXPECT_EQ(init_position.at(2), position.at(2));

  ASSERT_EQ(timestamp, pose->GetTimestamp());
}

TEST(CameraPose, SerializePosition) {
  double x = -85.32;
  double y = -135.20341;
  double z = 1.203;
  int64 timestamp = 1455818790;
  std::vector<double> init_position = { x, y, z };

  std::unique_ptr<CameraPose> pose =
      CameraPose::FromData(init_position, std::vector<double>(), timestamp);
  ASSERT_NE(nullptr, pose);

  // Set up XML structure.
  xmlNodePtr camera_node = xmlNewNode(nullptr, ToXmlChar(XdmConst::Camera()));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, camera_node);

  // Create serializer.
  std::unordered_map<string, xmlNsPtr> namespaces = SetupNamespaces();
  SerializerImpl serializer(namespaces, camera_node);
  std::unique_ptr<Serializer> pose_serializer =
      serializer.CreateSerializer(
          XdmConst::Namespace(XdmConst::CameraPose()), XdmConst::CameraPose());
  ASSERT_NE(nullptr, pose_serializer);

  ASSERT_TRUE(pose->Serialize(pose_serializer.get()));

  const string testdata_path = TestFileAbsolutePath(kTestDataPosition);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(testdata_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xml_doc));

  // Clean up.
  FreeNamespaces(namespaces);
  xmlFreeDoc(xml_doc);
}

TEST(CameraPose, SerializeOrientation) {
  double axis_x = 1;
  double axis_y = 2;
  double axis_z = 1.5;
  double angle = 1.57;
  std::vector<double> init_orientation = { axis_x, axis_y, axis_z, angle };

  std::unique_ptr<CameraPose> pose =
      CameraPose::FromData(std::vector<double>(), init_orientation);
  ASSERT_NE(nullptr, pose);

  // Set up XML structure.
  xmlNodePtr camera_node = xmlNewNode(nullptr, ToXmlChar(XdmConst::Camera()));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, camera_node);

  // Create serializer.
  std::unordered_map<string, xmlNsPtr> namespaces = SetupNamespaces();
  SerializerImpl serializer(namespaces, camera_node);
  std::unique_ptr<Serializer> pose_serializer =
      serializer.CreateSerializer(
          XdmConst::Namespace(XdmConst::CameraPose()), XdmConst::CameraPose());
  ASSERT_NE(nullptr, pose_serializer);

  ASSERT_TRUE(pose->Serialize(pose_serializer.get()));

  const string testdata_path = TestFileAbsolutePath(kTestDataOrientation);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(testdata_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xml_doc));

  // Clean up.
  FreeNamespaces(namespaces);
  xmlFreeDoc(xml_doc);
}


TEST(CameraPose, SerializePositionAndOrientation) {
  double x = -85.32;
  double y = -135.20341;
  double z = 1.203;

  double axis_x = 1;
  double axis_y = 2;
  double axis_z = 1.5;
  double angle = 1.57;

  int64 timestamp = 1455818790;
  std::vector<double> init_position = { x, y, z };
  std::vector<double> init_orientation = { axis_x, axis_y, axis_z, angle };

  std::unique_ptr<CameraPose> pose =
      CameraPose::FromData(init_position, init_orientation, timestamp);
  ASSERT_NE(nullptr, pose);

  // Set up XML structure.
  xmlNodePtr camera_node = xmlNewNode(nullptr, ToXmlChar(XdmConst::Camera()));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, camera_node);

  // Create serializer.
  std::unordered_map<string, xmlNsPtr> namespaces = SetupNamespaces();
  SerializerImpl serializer(namespaces, camera_node);
  std::unique_ptr<Serializer> pose_serializer =
      serializer.CreateSerializer(
          XdmConst::Namespace(XdmConst::CameraPose()), XdmConst::CameraPose());
  ASSERT_NE(nullptr, pose_serializer);

  ASSERT_TRUE(pose->Serialize(pose_serializer.get()));

  const string testdata_path = TestFileAbsolutePath(kTestDataFull);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(testdata_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xml_doc));

  // Clean up.
  FreeNamespaces(namespaces);
  xmlFreeDoc(xml_doc);
}

TEST(CameraPose, ReadMetadataPositionOrientation) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Set up the XML structure.
  xmlNodePtr parent_node =
      xmlNewNode(nullptr, ToXmlChar(XdmConst::Camera()));
  xmlAddChild(description_node, parent_node);

  xmlNsPtr camera_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakeh.ref"),
               ToXmlChar(XdmConst::Namespace(XdmConst::CameraPose()).data()));
  xmlNodePtr pose_node =
      xmlNewNode(camera_ns, ToXmlChar(XdmConst::CameraPose()));
  xmlAddChild(parent_node, pose_node);
  xmlNsPtr pose_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                              ToXmlChar(XdmConst::CameraPose()));

  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("PositionX"), ToXmlChar("1.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("PositionY"), ToXmlChar("2.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("PositionZ"), ToXmlChar("-1"));

  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAxisX"),
               ToXmlChar("1.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAxisY"),
               ToXmlChar("2.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAxisZ"),
               ToXmlChar("1.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAngle"),
               ToXmlChar("1.57"));

  // Create an CameraPose from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<CameraPose> pose =
      CameraPose::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, pose.get());

  std::vector<double> values = pose->GetPositionXYZ();
  ASSERT_EQ(3, values.size());
  EXPECT_EQ(1.5, values[0]);
  EXPECT_EQ(2.5, values[1]);
  EXPECT_EQ(-1, values[2]);

  values = pose->GetOrientationRotationXYZAngle();
  ASSERT_EQ(4, values.size());
  std::vector<double> raw_values({1.5, 2.5, 1.5, 1.57});
  std::vector<double> normalized = NormalizeAxisAngle(raw_values);
  EXPECT_EQ(normalized[0], values[0]);
  EXPECT_EQ(normalized[1], values[1]);
  EXPECT_EQ(normalized[2], values[2]);
  EXPECT_EQ(normalized[3], values[3]);

  EXPECT_EQ(-1, pose->GetTimestamp());

  xmlFreeNs(camera_ns);
  xmlFreeNs(pose_ns);
}

TEST(CameraPose, ReadMetadataOrientation) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // Set up the XML structure.
  xmlNodePtr parent_node =
      xmlNewNode(nullptr, ToXmlChar(XdmConst::Camera()));
  xmlAddChild(description_node, parent_node);

  xmlNsPtr camera_ns =
      xmlNewNs(nullptr, ToXmlChar("http://fakeh.ref"),
               ToXmlChar(XdmConst::Namespace(XdmConst::CameraPose()).data()));
  xmlNodePtr pose_node =
      xmlNewNode(camera_ns, ToXmlChar(XdmConst::CameraPose()));
  xmlAddChild(parent_node, pose_node);
  xmlNsPtr pose_ns = xmlNewNs(nullptr, ToXmlChar(kNamespaceHref),
                              ToXmlChar(XdmConst::CameraPose()));

  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAxisX"),
               ToXmlChar("1.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAxisY"),
               ToXmlChar("2.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAxisZ"),
               ToXmlChar("1.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAngle"),
               ToXmlChar("1.57"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("Timestamp"),
               ToXmlChar("1455818790"));

  // Create an CameraPose from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<CameraPose> pose =
      CameraPose::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, pose.get());

  std::vector<double> values = pose->GetOrientationRotationXYZAngle();
  ASSERT_EQ(4, values.size());
  std::vector<double> raw_values({1.5, 2.5, 1.5, 1.57});
  std::vector<double> normalized = NormalizeAxisAngle(raw_values);
  EXPECT_EQ(normalized[0], values[0]);
  EXPECT_EQ(normalized[1], values[1]);
  EXPECT_EQ(normalized[2], values[2]);
  EXPECT_EQ(normalized[3], values[3]);

  ASSERT_EQ(1455818790, pose->GetTimestamp());

  xmlFreeNs(camera_ns);
  xmlFreeNs(pose_ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
