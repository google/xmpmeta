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

#include "xdmlib/camera.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/port.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "xdmlib/audio.h"
#include "xdmlib/camera_pose.h"
#include "xdmlib/const.h"
#include "xdmlib/equirect_model.h"
#include "xdmlib/image.h"
#include "xdmlib/vendor_info.h"
#include "xmpmeta/base64.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"
#include "xmpmeta/xmp_const.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"

using testing::Pair;
using testing::UnorderedElementsAre;
using testing::StrEq;
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

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/camera/";
const char kAudioNamespaceHref[] = "http://ns.xdm.org/photos/1.0/audio/";
const char kCameraDataPath[] = "xdm/camera_testdata.txt";

const char kMediaData[] = "123ABC456DEF";
const string manufacturer = "manufacturer_1";
const string model = "model_1";
const string notes = "notes_1";
const int cropped_origin_x = 0;
const int cropped_origin_y = 1530;
const int cropped_size_width = 3476;
const int cropped_size_height = 1355;
const int full_size_width = 8192;
const int full_size_height = 4096;

// Convenience function for creating an XML node.
xmlNodePtr NewNode(xmlNsPtr xml_ns, const string& node_name) {
  return xmlNewNode(xml_ns, ToXmlChar(node_name.data()));
}

// Convenience function for creating an XML namespace.
xmlNsPtr NewNs(const string& href, const string& ns_name) {
  return xmlNewNs(nullptr, href.empty() ? nullptr : ToXmlChar(href.data()),
                  ToXmlChar(ns_name.data()));
}

std::unique_ptr<Audio> CreateAudio() {
  return Audio::FromData(kMediaData, "audio/mp4");
}

std::unique_ptr<Image> CreateImage() {
  return Image::FromData(kMediaData, "image/jpeg", "");
}

std::unique_ptr<CameraPose> CreateCameraPose() {
  double axis_x = 1;
  double axis_y = 2;
  double axis_z = 1.5;
  double angle = 1.57;
  std::vector<double> orientation = {axis_x, axis_y, axis_z, angle};
  return CameraPose::FromData(std::vector<double>(), orientation, -1);
}

std::unique_ptr<VendorInfo> CreateVendorInfo() {
  return VendorInfo::FromData(manufacturer, model, notes);
}

std::unique_ptr<EquirectModel> CreateEquirectModel() {
  const Point cropped_origin(cropped_origin_x, cropped_origin_y);
  const Dimension cropped_size(cropped_size_width, cropped_size_height);
  const Dimension full_size(full_size_width, full_size_height);
  return EquirectModel::FromData(cropped_origin, cropped_size, full_size);
}

TEST(Camera, GetNamespaces) {
  std::unordered_map<string, string> ns_name_href_map;
  string prefix(XdmConst::Camera());

  std::unique_ptr<Audio> audio = CreateAudio();
  std::unique_ptr<Camera> camera =
      Camera::FromData(std::move(audio), nullptr, nullptr, nullptr, nullptr);
  ASSERT_NE(nullptr, camera);

  ASSERT_TRUE(ns_name_href_map.empty());
  camera->GetNamespaces(&ns_name_href_map);
  EXPECT_THAT(ns_name_href_map,
              UnorderedElementsAre(
                  Pair(StrEq(prefix), StrEq(kNamespaceHref)),
                  Pair(StrEq(XdmConst::Audio()), StrEq(kAudioNamespaceHref))));
}

// TODO(miraleung): Add corner case tests when the rest of the elements are
// checked in.
TEST(Camera, FromData) {
  std::unique_ptr<Audio> audio = CreateAudio();
  std::unique_ptr<Image> image = CreateImage();
  std::unique_ptr<CameraPose> camera_pose = CreateCameraPose();
  std::unique_ptr<VendorInfo> vendor_info = CreateVendorInfo();
  std::unique_ptr<EquirectModel> equirect_model = CreateEquirectModel();

  const string audio_mime = audio->GetMime();
  const string image_mime = image->GetMime();

  std::unique_ptr<Camera> camera = Camera::FromData(
      std::move(audio), std::move(image), std::move(camera_pose),
      std::move(vendor_info), std::move(equirect_model));
  ASSERT_NE(nullptr, camera);

  const Audio* read_audio = camera->GetAudio();
  ASSERT_NE(nullptr, read_audio);
  EXPECT_EQ(kMediaData, read_audio->GetData());
  EXPECT_EQ(audio_mime, read_audio->GetMime());

  const Image* read_image = camera->GetImage();
  ASSERT_NE(nullptr, read_image);
  EXPECT_EQ(kMediaData, read_image->GetData());
  EXPECT_EQ(image_mime, read_image->GetMime());

  const CameraPose* read_pose = camera->GetCameraPose();
  ASSERT_NE(nullptr, read_pose);

  const VendorInfo* read_vendor_info = camera->GetVendorInfo();
  ASSERT_NE(nullptr, read_vendor_info);
  EXPECT_EQ(manufacturer, read_vendor_info->GetManufacturer());
  EXPECT_EQ(model, read_vendor_info->GetModel());
  EXPECT_EQ(notes, read_vendor_info->GetNotes());

  const EquirectModel* read_equirect_model =
      static_cast<const EquirectModel*>(camera->GetImagingModel());
  ASSERT_NE(nullptr, read_equirect_model);
  Point cropped_origin = read_equirect_model->GetCroppedOrigin();
  EXPECT_EQ(cropped_origin_x, cropped_origin.x);
  EXPECT_EQ(cropped_origin_y, cropped_origin.y);

  Dimension cropped_size = read_equirect_model->GetCroppedSize();
  EXPECT_EQ(cropped_size_width, cropped_size.width);
  EXPECT_EQ(cropped_size_height, cropped_size.height);

  Dimension full_size = read_equirect_model->GetFullSize();
  EXPECT_EQ(full_size_width, full_size.width);
  EXPECT_EQ(full_size_height, full_size.height);
}

TEST(Camera, FromDataWithNullElements) {
  std::unique_ptr<Image> image = CreateImage();
  const string image_mime = image->GetMime();

  std::unique_ptr<Camera> camera =
      Camera::FromData(nullptr, std::move(image), nullptr, nullptr, nullptr);
  ASSERT_NE(nullptr, camera);

  const Audio* read_audio = camera->GetAudio();
  ASSERT_EQ(nullptr, read_audio);

  const Image* read_image = camera->GetImage();
  EXPECT_EQ(kMediaData, read_image->GetData());
  EXPECT_EQ(image_mime, read_image->GetMime());

  EXPECT_EQ(nullptr, camera->GetCameraPose());
  EXPECT_EQ(nullptr, camera->GetVendorInfo());
  EXPECT_EQ(nullptr, camera->GetImagingModel());
}

TEST(Camera, Serialize) {
  std::unique_ptr<Audio> audio = CreateAudio();
  std::unique_ptr<Image> image = CreateImage();
  std::unique_ptr<CameraPose> camera_pose = CreateCameraPose();
  std::unique_ptr<VendorInfo> vendor_info = CreateVendorInfo();
  std::unique_ptr<EquirectModel> equirect_model = CreateEquirectModel();
  std::unique_ptr<Camera> camera = Camera::FromData(
      std::move(audio), std::move(image), std::move(camera_pose),
      std::move(vendor_info), std::move(equirect_model));
  ASSERT_NE(nullptr, camera);

  // Create XML serializer.
  const char* device_name = XdmConst::Device();
  const char* camera_name = XdmConst::Camera();
  const char* audio_name = XdmConst::Audio();
  const char* image_name = XdmConst::Image();
  const char* camera_pose_name = XdmConst::CameraPose();
  const char* vendor_info_name = XdmConst::VendorInfo();
  const char* equirect_model_name = XdmConst::EquirectModel();
  const char* namespaceHref = "http://notarealh.ref";

  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, NewNs(namespaceHref, device_name));
  namespaces.emplace(camera_name, NewNs(kNamespaceHref, camera_name));
  namespaces.emplace(audio_name, NewNs(namespaceHref, audio_name));
  namespaces.emplace(image_name, NewNs(namespaceHref, image_name));
  namespaces.emplace(camera_pose_name, NewNs(namespaceHref, camera_pose_name));
  namespaces.emplace(vendor_info_name, NewNs(namespaceHref, vendor_info_name));
  namespaces.emplace(equirect_model_name,
                     NewNs(namespaceHref, equirect_model_name));

  xmlNodePtr device_node = NewNode(nullptr, device_name);
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  std::unique_ptr<Serializer> camera_serializer = serializer.CreateSerializer(
      XdmConst::Namespace(camera_name), camera_name);
  ASSERT_NE(nullptr, camera_serializer);

  ASSERT_TRUE(camera->Serialize(camera_serializer.get()));

  const string xdm_camera_data_path = TestFileAbsolutePath(kCameraDataPath);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(xdm_camera_data_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xml_doc));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Camera, ReadMetadata) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  xmlNsPtr device_ns = xmlNewNs(nullptr, ToXmlChar("http:somehref.com"),
                                ToXmlChar(XdmConst::Device()));
  xmlNodePtr camera_node = NewNode(device_ns, XdmConst::Camera());
  xmlAddChild(description_node, camera_node);

  // Set up Audio node.
  const char audio_name[] = "Audio";
  xmlNsPtr camera_ns = xmlNewNs(nullptr, ToXmlChar("http://somehref.com"),
                                ToXmlChar(XdmConst::Camera()));
  xmlNodePtr audio_node = NewNode(camera_ns, audio_name);
  xmlAddChild(camera_node, audio_node);

  // Set Audio properties.
  string base64_encoded;
  ASSERT_TRUE(EncodeBase64(kMediaData, &base64_encoded));
  const char namespaceHref[] = "http://notarealh.ref";
  const string audio_mime("audio/wav");
  xmlNsPtr audio_ns = NewNs(namespaceHref, audio_name);
  xmlSetNsProp(audio_node, audio_ns, ToXmlChar("Mime"),
               ToXmlChar(audio_mime.data()));
  xmlSetNsProp(audio_node, audio_ns, ToXmlChar("Data"),
               ToXmlChar(base64_encoded.data()));

  // Set up Image node.
  const char image_name[] = "Image";
  xmlNodePtr image_node = NewNode(camera_ns, image_name);
  xmlAddChild(camera_node, image_node);

  const string image_mime("image/jpeg");
  xmlNsPtr image_ns = NewNs(namespaceHref, "Image");
  xmlSetNsProp(image_node, image_ns, ToXmlChar("Mime"),
               ToXmlChar(image_mime.data()));
  xmlSetNsProp(image_node, image_ns, ToXmlChar("Data"),
               ToXmlChar(base64_encoded.data()));

  // Set up Camera Pose node.
  xmlNodePtr pose_node = NewNode(camera_ns, "CameraPose");
  xmlAddChild(camera_node, pose_node);
  xmlNsPtr pose_ns = NewNs(namespaceHref, "CameraPose");

  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAxisX"), ToXmlChar("1"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAxisY"), ToXmlChar("2"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAxisZ"),
               ToXmlChar("1.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("RotationAngle"),
               ToXmlChar("1.57"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("Timestamp"),
               ToXmlChar("1455818790"));

  // Set up VendorInfo node.
  xmlNodePtr vendor_info_node = NewNode(camera_ns, "VendorInfo");
  xmlAddChild(camera_node, vendor_info_node);
  xmlNsPtr vendor_info_ns = NewNs(namespaceHref, "VendorInfo");
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Manufacturer"),
               ToXmlChar(manufacturer.data()));
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Model"),
               ToXmlChar(model.data()));
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Notes"),
               ToXmlChar(notes.data()));

  // Mock an XdmCamera node.
  xmlNodePtr equirect_model_node = NewNode(camera_ns, "EquirectModel");
  xmlAddChild(camera_node, equirect_model_node);

  xmlNsPtr equirect_model_ns = NewNs(namespaceHref, "EquirectModel");
  xmlSetNsProp(equirect_model_node, equirect_model_ns,
               ToXmlChar("CroppedAreaLeftPixels"), ToXmlChar("0"));
  xmlSetNsProp(equirect_model_node, equirect_model_ns,
               ToXmlChar("CroppedAreaTopPixels"), ToXmlChar("1530"));
  xmlSetNsProp(equirect_model_node, equirect_model_ns,
               ToXmlChar("CroppedAreaImageWidthPixels"), ToXmlChar("3476"));
  xmlSetNsProp(equirect_model_node, equirect_model_ns,
               ToXmlChar("CroppedAreaImageHeightPixels"), ToXmlChar("1355"));
  xmlSetNsProp(equirect_model_node, equirect_model_ns,
               ToXmlChar("FullImageWidthPixels"), ToXmlChar("8192"));
  xmlSetNsProp(equirect_model_node, equirect_model_ns,
               ToXmlChar("FullImageHeightPixels"), ToXmlChar("4096"));

  // Create an Camera from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Camera> camera = Camera::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, camera.get());

  // Check elements.
  const Audio* read_audio = camera->GetAudio();
  ASSERT_NE(nullptr, read_audio);
  EXPECT_EQ(kMediaData, read_audio->GetData());
  EXPECT_EQ(audio_mime, read_audio->GetMime());

  const Image* read_image = camera->GetImage();
  ASSERT_NE(nullptr, read_image);
  EXPECT_EQ(kMediaData, read_image->GetData());
  EXPECT_EQ(image_mime, read_image->GetMime());

  const CameraPose* read_pose = camera->GetCameraPose();
  std::unique_ptr<CameraPose> pose = CreateCameraPose();
  ASSERT_NE(nullptr, read_pose);
  EXPECT_EQ(pose->GetOrientationRotationXYZAngle(),
            read_pose->GetOrientationRotationXYZAngle());
  EXPECT_EQ(pose->HasPosition(), read_pose->HasPosition());

  const VendorInfo* read_vendor_info = camera->GetVendorInfo();
  ASSERT_NE(nullptr, read_vendor_info);
  ASSERT_EQ(manufacturer, read_vendor_info->GetManufacturer());
  ASSERT_EQ(model, read_vendor_info->GetModel());
  ASSERT_EQ(notes, read_vendor_info->GetNotes());

  const EquirectModel* read_equirect_model =
      static_cast<const EquirectModel*>(camera->GetImagingModel());
  ASSERT_NE(nullptr, read_equirect_model);
  Point cropped_origin = read_equirect_model->GetCroppedOrigin();
  EXPECT_EQ(cropped_origin_x, cropped_origin.x);
  EXPECT_EQ(cropped_origin_y, cropped_origin.y);

  Dimension cropped_size = read_equirect_model->GetCroppedSize();
  EXPECT_EQ(cropped_size_width, cropped_size.width);
  EXPECT_EQ(cropped_size_height, cropped_size.height);

  Dimension full_size = read_equirect_model->GetFullSize();
  EXPECT_EQ(full_size_width, full_size.width);
  EXPECT_EQ(full_size_height, full_size.height);

  xmlFreeNs(audio_ns);
  xmlFreeNs(camera_ns);
  xmlFreeNs(device_ns);
  xmlFreeNs(image_ns);
  xmlFreeNs(pose_ns);
  xmlFreeNs(vendor_info_ns);
  xmlFreeNs(equirect_model_ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
