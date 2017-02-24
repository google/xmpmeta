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

#include "xdmlib/device.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "gmock/gmock.h"
#include "xdmlib/audio.h"
#include "xdmlib/camera.h"
#include "xdmlib/cameras.h"
#include "xdmlib/const.h"
#include "xdmlib/device_pose.h"
#include "xdmlib/profile.h"
#include "xdmlib/profiles.h"
#include "xmpmeta/base64.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"

using testing::ElementsAreArray;
using xmpmeta::CreateXmpData;
using xmpmeta::xml::Deserializer;
using xmpmeta::xml::DeserializerImpl;
using xmpmeta::xml::GetFirstDescriptionElement;
using xmpmeta::xml::Serializer;
using xmpmeta::xml::ToXmlChar;
using xmpmeta::xml::XmlConst;
using xmpmeta::xml::XmlDocToString;

namespace xmpmeta {
namespace xdm {
namespace {

// Test data constants.
const char kDeviceDataPath[] = "xdm/device_testdata.txt";
const char kMediaData[] = "123ABC456DEF";

const double kLat = -85.32;
const double kLon = -135.20341;
const double kAlt = 1.203;

// Convenience function for creating an XML node.
xmlNodePtr NewNode(const xmlNsPtr xml_ns, const string& node_name) {
  return xmlNewNode(xml_ns, ToXmlChar(node_name.data()));
}

// Convenience function for creating an XML namespace.
xmlNsPtr NewNs(const string& href, const string& ns_name) {
  return xmlNewNs(nullptr,
                  href.empty() ? nullptr : ToXmlChar(href.data()),
                  ToXmlChar(ns_name.data()));
}

std::unique_ptr<Audio> CreateAudio() {
  return Audio::FromData(kMediaData, "audio/mp4");
}

std::unique_ptr<Camera> CreateCamera() {
  std::unique_ptr<Audio> audio = CreateAudio();
  return Camera::FromData(std::move(audio), nullptr, nullptr);
}

std::unique_ptr<DevicePose> CreateDevicePose() {
  return DevicePose::FromData({kLat, kLon, kAlt}, std::vector<double>(), 0);
}

std::unique_ptr<Profile> CreateVrPhotoProfile() {
  return Profile::FromData("VRPhoto", {0, 1});
}

std::unique_ptr<Profiles> CreateProfiles() {
  std::vector<std::unique_ptr<Profile>> profile_list;
  profile_list.emplace_back(CreateVrPhotoProfile());
  return Profiles::FromProfileArray(&profile_list);
}

// Returns an XML rdf:Seq node with a list of the given values.
xmlNodePtr SetupRdfSeqOfIndices(const std::vector<int>& values,
                                const xmlNsPtr rdf_ns) {
  xmlNodePtr rdf_seq_node =
      xmlNewNode(rdf_ns, ToXmlChar(XmlConst::RdfSeq()));
  for (int value : values) {
    xmlNodePtr rdf_li_node =
        xmlNewNode(rdf_ns, ToXmlChar(XmlConst::RdfLi()));
    xmlNodeSetContent(
        rdf_li_node, ToXmlChar(std::to_string(value).data()));
    xmlAddChild(rdf_seq_node, rdf_li_node);
  }
  return rdf_seq_node;
}

void ExpectProfileEquals(const Profile& profile_1, const Profile& profile_2) {
  EXPECT_EQ(profile_1.GetType(), profile_2.GetType());
  const std::vector<int>& indices_1 = profile_1.GetCameraIndices();
  const std::vector<int>& indices_2 = profile_2.GetCameraIndices();
  EXPECT_THAT(indices_1, ElementsAreArray(indices_2));
}

TEST(Device, FromData) {
  std::vector<std::unique_ptr<Camera>> camera_list;
  int num_cameras = 3;
  for (int i = 0; i < num_cameras; i++) {
    camera_list.emplace_back(CreateCamera());
  }

  std::unique_ptr<Cameras> cameras = Cameras::FromCameraArray(&camera_list);
  ASSERT_NE(nullptr, cameras);

  std::unique_ptr<DevicePose> device_pose = CreateDevicePose();
  ASSERT_NE(nullptr, device_pose);

  std::unique_ptr<Profiles> profiles = CreateProfiles();
  ASSERT_NE(nullptr, profiles);

  std::unique_ptr<Device> device =
      Device::FromData("1.01", std::move(device_pose), std::move(profiles),
                       std::move(cameras));
  ASSERT_NE(nullptr, device);

  const DevicePose* read_device_pose = device->GetDevicePose();
  ASSERT_NE(nullptr, read_device_pose);
  const std::vector<double>& position = read_device_pose->GetPosition();
  EXPECT_EQ(3, position.size());
  EXPECT_DOUBLE_EQ(kLat, position[0]);
  EXPECT_DOUBLE_EQ(kLon, position[1]);
  EXPECT_DOUBLE_EQ(kAlt, position[2]);
  EXPECT_EQ(0, read_device_pose->GetOrientationRotationXYZAngle().size());

  const Profiles* read_profiles = device->GetProfiles();
  ASSERT_NE(nullptr, read_profiles);
  const std::vector<const Profile*> profile_list = read_profiles->GetProfiles();
  ASSERT_EQ(1, profile_list.size());
  ExpectProfileEquals(*CreateVrPhotoProfile(), *profile_list[0]);

  const std::vector<const Camera*>& read_camera_list =
      device->GetCameras()->GetCameras();
  EXPECT_EQ(num_cameras, read_camera_list.size());
  const std::unique_ptr<Audio> audio = CreateAudio();
  for (int i = 0; i < num_cameras; i++) {
    const Camera* camera = read_camera_list[i];
    ASSERT_NE(nullptr, camera);

    // Check that child elements are equal.
    const Audio* read_audio = camera->GetAudio();
    ASSERT_NE(nullptr, audio);
    ASSERT_NE(nullptr, read_audio);
    EXPECT_EQ(audio->GetMime(), read_audio->GetMime());
    EXPECT_EQ(audio->GetData(), read_audio->GetData());
  }
}

TEST(Device, FromDataEmptyRevision) {
  std::unique_ptr<DevicePose> device_pose = CreateDevicePose();
  std::unique_ptr<Device> device =
      Device::FromData("", std::move(device_pose), nullptr, nullptr);
  ASSERT_EQ(nullptr, device);
}

TEST(Device, FromDataNoCameras) {
  std::unique_ptr<Device> device =
      Device::FromData("1.01", nullptr, nullptr, nullptr);
  ASSERT_NE(nullptr, device);
  EXPECT_EQ(nullptr, device->GetCameras());
  EXPECT_EQ(nullptr, device->GetDevicePose());
}

TEST(Device, Serialize) {
  std::vector<std::unique_ptr<Camera>> camera_list;
  int num_cameras = 3;
  for (int i = 0; i < num_cameras; i++) {
    std::unique_ptr<Camera> camera = CreateCamera();
    camera_list.emplace_back(std::move(camera));
  }

  std::unique_ptr<Cameras> cameras = Cameras::FromCameraArray(&camera_list);
  ASSERT_NE(nullptr, cameras);

  std::unique_ptr<DevicePose> device_pose = CreateDevicePose();
  ASSERT_NE(nullptr, device_pose);

  std::unique_ptr<Profiles> profiles = CreateProfiles();
  ASSERT_NE(nullptr, profiles);

  std::unique_ptr<Device> device =
      Device::FromData("1.01", std::move(device_pose), std::move(profiles),
                       std::move(cameras));
  ASSERT_NE(nullptr, device);

  // Create serializer.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  ASSERT_TRUE(device->SerializeToXmp(xmp_data.get()));

  const string testdata_path = TestFileAbsolutePath(kDeviceDataPath);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(testdata_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xmp_data->ExtendedSection()));
}

TEST(Device, ReadMetadata) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // XDM Device node.
  xmlNsPtr device_ns = NewNs("http://fakeh.ref", XdmConst::Device());
  xmlNodePtr device_node = NewNode(device_ns, XdmConst::Device());
  xmlAddChild(description_node, device_node);
  xmlSetNsProp(device_node, device_ns, ToXmlChar("Revision"),
               ToXmlChar("1.01"));

  // Device:DevicePose node.
  xmlNodePtr pose_node = NewNode(device_ns, "DevicePose");
  xmlAddChild(device_node, pose_node);
  xmlNsPtr pose_ns = NewNs("http://fakeh.ref", "DevicePose");
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("Latitude"), ToXmlChar("1.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("Longitude"), ToXmlChar("2.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("Altitude"), ToXmlChar("-1"));

  // Device:Cameras node.
  xmlNodePtr cameras_node = NewNode(device_ns, "Cameras");
  xmlAddChild(device_node, cameras_node);

  // rdf:Seq node.
  xmlNsPtr rdf_ns = NewNs("http://fakeh.ref", XmlConst::RdfPrefix());
  xmlNodePtr cameras_rdf_seq_node = NewNode(rdf_ns, XmlConst::RdfSeq());
  xmlAddChild(cameras_node, cameras_rdf_seq_node);

  // Set up minimal Camera nodes.
  const char audio_ns_href[] = "http://ns.xdm.org/photos/1.0/audio/";
  const char audio_mime[] = "audio/mp4";
  string base64_encoded;
  ASSERT_TRUE(EncodeBase64(kMediaData, &base64_encoded));

  int num_cameras = 3;
  xmlNsPtr audio_ns = NewNs(audio_ns_href, "Audio");
  xmlNsPtr camera_ns = NewNs("http://fakeh.ref", XdmConst::Camera());
  for (int i = 0; i < num_cameras; i++) {
    xmlNodePtr camera_node = NewNode(device_ns, "Camera");
    xmlNodePtr audio_node = NewNode(camera_ns, "Audio");
    xmlSetNsProp(audio_node, audio_ns, ToXmlChar("Mime"),
                 ToXmlChar(audio_mime));
    xmlSetNsProp(audio_node, audio_ns, ToXmlChar("Data"),
                 ToXmlChar(base64_encoded.data()));
    xmlNodePtr rdf_li_node = NewNode(rdf_ns, XmlConst::RdfLi());

    xmlAddChild(camera_node, audio_node);
    xmlAddChild(rdf_li_node, camera_node);
    xmlAddChild(cameras_rdf_seq_node, rdf_li_node);
  }

  // Create a Device object from the XMP metadata.
  std::unique_ptr<Device> device = Device::FromXmp(*xmp_data);
  ASSERT_NE(nullptr, device);
  EXPECT_EQ("1.01", device->GetRevision());

  // Check DevicePose.
  const DevicePose* pose = device->GetDevicePose();
  ASSERT_NE(nullptr, pose);
  std::vector<double> values = pose->GetPosition();
  ASSERT_EQ(3, values.size());
  EXPECT_DOUBLE_EQ(1.5, values[0]);
  EXPECT_DOUBLE_EQ(2.5, values[1]);
  EXPECT_DOUBLE_EQ(-1, values[2]);
  EXPECT_EQ(0, pose->GetOrientationRotationXYZAngle().size());

  // Check Cameras.
  const Cameras* cameras = device->GetCameras();
  ASSERT_NE(nullptr, cameras);

  const std::vector<const Camera*>& read_camera_list = cameras->GetCameras();
  EXPECT_EQ(num_cameras, read_camera_list.size());
  for (int i = 0; i < num_cameras; i++) {
    const Camera* camera = read_camera_list[i];
    ASSERT_NE(nullptr, camera);

    const Audio* audio = camera->GetAudio();
    ASSERT_NE(nullptr, audio);
    EXPECT_EQ(audio_mime, audio->GetMime());
    EXPECT_EQ(kMediaData, audio->GetData());
  }

  xmlFreeNs(audio_ns);
  xmlFreeNs(camera_ns);
  xmlFreeNs(device_ns);
  xmlFreeNs(pose_ns);
  xmlFreeNs(rdf_ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
