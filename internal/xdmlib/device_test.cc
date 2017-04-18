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
#include "strings/numbers.h"
#include "xdmlib/audio.h"
#include "xdmlib/camera.h"
#include "xdmlib/cameras.h"
#include "xdmlib/const.h"
#include "xdmlib/device_pose.h"
#include "xdmlib/mesh.h"
#include "xdmlib/navigational_connectivity.h"
#include "xdmlib/profile.h"
#include "xdmlib/profiles.h"
#include "xdmlib/vendor_info.h"
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
const char kDeviceDataPosedCollectionPath[] =
    "xdm/device_testdata_posedcollection.xml";
const char kMediaData[] = "123ABC456DEF";

// Device pose
const double kDeviceLat = -85.32;
const double kDeviceLon = -135.20341;
const double kDeviceAlt = 1.203;

// Vendor info.
const string manufacturer = "manufacturer_1";
const string model = "model_1";
const string notes = "notes_1";

// Camera pose.
const double kCameraRotationAxisX = 0.37139105566446701;
const double kCameraRotationAxisY = 0.7427811113287841;
const double kCameraRotationAxisZ = 0.55708608349662558;
const double kCameraRotationAngle = 1.57;

// Number of cameras.
const int kCameraNum = 3;

// Image
const char kImageMime[] = "image/jpeg";
const char kImageId[] = "unique_image_id";

// Mesh with 4 vertices and 4 faces.
const int kVertexCount = 4;
const std::vector<float> kVertexPosition{0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                                         0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
const int kFaceCount = 4;
const std::vector<int> kFaceIndices{0, 1, 2, 0, 1, 3, 0, 3, 2, 1, 2, 3};
const char kSoftware[] = "software";

// NavigationalConnectivity with 2 navigational edges from image 0 to image 1
// and from image 1 to image 0.
const std::vector<int> kNavigationalEdges{0, 1, 1, 0};

// Convenience function for creating an XML node.
xmlNodePtr NewNode(const xmlNsPtr xml_ns, const string& node_name) {
  return xmlNewNode(xml_ns, ToXmlChar(node_name.data()));
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

std::unique_ptr<Audio> CreateAudio() {
  return Audio::FromData(kMediaData, "audio/mp4");
}

std::unique_ptr<Camera> CreateCamera() {
  std::unique_ptr<Audio> audio = CreateAudio();
  return Camera::FromData(std::move(audio), nullptr, nullptr, nullptr, nullptr);
}

std::unique_ptr<DevicePose> CreateDevicePose() {
  return DevicePose::FromData({kDeviceLat, kDeviceLon, kDeviceAlt},
                              std::vector<double>(), 0);
}

std::unique_ptr<Profile> CreateVrPhotoProfile() {
  return Profile::FromData("VRPhoto", {0, 1});
}

std::unique_ptr<Profiles> CreateProfiles() {
  std::vector<std::unique_ptr<Profile>> profile_list;
  profile_list.emplace_back(CreateVrPhotoProfile());
  return Profiles::FromProfileArray(&profile_list);
}

std::unique_ptr<VendorInfo> CreateVendorInfo() {
  return VendorInfo::FromData(manufacturer, model, notes);
}

std::unique_ptr<Mesh> CreateMesh() {
  return Mesh::FromData(kVertexCount, kVertexPosition, kFaceCount, kFaceIndices,
                        false, kSoftware);
}

std::unique_ptr<NavigationalConnectivity> CreateNavigationalConnectivity() {
  return NavigationalConnectivity::FromData(kNavigationalEdges);
}

// Returns an XML rdf:Seq node with a list of the given values.
xmlNodePtr SetupRdfSeqOfIndices(const std::vector<int>& values,
                                const xmlNsPtr rdf_ns) {
  xmlNodePtr rdf_seq_node = xmlNewNode(rdf_ns, ToXmlChar(XmlConst::RdfSeq()));
  for (int value : values) {
    xmlNodePtr rdf_li_node = xmlNewNode(rdf_ns, ToXmlChar(XmlConst::RdfLi()));
    xmlNodeSetContent(rdf_li_node, ToXmlChar(std::to_string(value).data()));
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

  std::unique_ptr<VendorInfo> vendor_info = CreateVendorInfo();
  ASSERT_NE(nullptr, vendor_info);

  std::unique_ptr<Mesh> mesh = CreateMesh();
  ASSERT_NE(nullptr, mesh);

  std::unique_ptr<NavigationalConnectivity> navigational_connectivity =
      CreateNavigationalConnectivity();
  ASSERT_NE(nullptr, navigational_connectivity);

  std::unique_ptr<Device> device =
      Device::FromData("1.02", std::move(device_pose), std::move(profiles),
                       std::move(cameras), std::move(vendor_info),
                       std::move(mesh), std::move(navigational_connectivity));
  ASSERT_NE(nullptr, device);

  const DevicePose* read_device_pose = device->GetDevicePose();
  ASSERT_NE(nullptr, read_device_pose);
  const std::vector<double>& position = read_device_pose->GetPosition();
  EXPECT_EQ(3, position.size());
  EXPECT_DOUBLE_EQ(kDeviceLat, position[0]);
  EXPECT_DOUBLE_EQ(kDeviceLon, position[1]);
  EXPECT_DOUBLE_EQ(kDeviceAlt, position[2]);
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

  const VendorInfo* read_vendor_info = device->GetVendorInfo();
  ASSERT_NE(nullptr, read_vendor_info);
  EXPECT_EQ(manufacturer, read_vendor_info->GetManufacturer());
  EXPECT_EQ(model, read_vendor_info->GetModel());
  EXPECT_EQ(notes, read_vendor_info->GetNotes());

  const Mesh* read_mesh = device->GetMesh();
  ASSERT_NE(nullptr, read_mesh);
  EXPECT_EQ(kVertexCount, read_mesh->GetVertexCount());
  EXPECT_EQ(kVertexPosition, read_mesh->GetVertexPosition());
  EXPECT_EQ(kFaceCount, read_mesh->GetFaceCount());
  EXPECT_EQ(kFaceIndices, read_mesh->GetFaceIndices());
  EXPECT_FALSE(read_mesh->GetMetric());
  EXPECT_EQ(kSoftware, read_mesh->GetSoftware());

  const NavigationalConnectivity* read_navigational_connectivity =
      device->GetNavigationalConnectivity();
  ASSERT_NE(nullptr, read_navigational_connectivity);
  EXPECT_EQ(kNavigationalEdges,
            read_navigational_connectivity->GetNavigationalEdges());
}

TEST(Device, FromDataEmptyRevision) {
  std::unique_ptr<DevicePose> device_pose = CreateDevicePose();
  std::unique_ptr<Device> device = Device::FromData(
      "", std::move(device_pose), nullptr, nullptr, nullptr, nullptr, nullptr);
  ASSERT_EQ(nullptr, device);
}

TEST(Device, FromDataNoCameras) {
  std::unique_ptr<Device> device = Device::FromData(
      "1.02", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  ASSERT_NE(nullptr, device);
  EXPECT_EQ(nullptr, device->GetCameras());
  EXPECT_EQ(nullptr, device->GetDevicePose());
  EXPECT_EQ(nullptr, device->GetVendorInfo());
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

  std::unique_ptr<VendorInfo> vendor_info = CreateVendorInfo();
  ASSERT_NE(nullptr, vendor_info);

  std::unique_ptr<Mesh> mesh = CreateMesh();
  ASSERT_NE(nullptr, mesh);

  std::unique_ptr<NavigationalConnectivity> navigational_connectivity =
      CreateNavigationalConnectivity();
  ASSERT_NE(nullptr, navigational_connectivity);

  std::unique_ptr<Device> device =
      Device::FromData("1.02", std::move(device_pose), std::move(profiles),
                       std::move(cameras), std::move(vendor_info),
                       std::move(mesh), std::move(navigational_connectivity));
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

  const char namespaceHref[] = "http://fakeh.ref";

  // XDM Device node.
  xmlNsPtr device_ns = NewNs(namespaceHref, XdmConst::Device());
  xmlNodePtr device_node = NewNode(device_ns, XdmConst::Device());
  xmlAddChild(description_node, device_node);
  xmlSetNsProp(device_node, device_ns, ToXmlChar("Revision"),
               ToXmlChar("1.02"));

  // Device:DevicePose node.
  xmlNodePtr pose_node = NewNode(device_ns, "DevicePose");
  xmlAddChild(device_node, pose_node);
  xmlNsPtr pose_ns = NewNs(namespaceHref, "DevicePose");
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("Latitude"), ToXmlChar("1.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("Longitude"), ToXmlChar("2.5"));
  xmlSetNsProp(pose_node, pose_ns, ToXmlChar("Altitude"), ToXmlChar("-1"));

  // Device:Cameras node.
  xmlNodePtr cameras_node = NewNode(device_ns, "Cameras");
  xmlAddChild(device_node, cameras_node);

  // rdf:Seq node.
  xmlNsPtr rdf_ns = NewNs(namespaceHref, XmlConst::RdfPrefix());
  xmlNodePtr cameras_rdf_seq_node = NewNode(rdf_ns, XmlConst::RdfSeq());
  xmlAddChild(cameras_node, cameras_rdf_seq_node);

  // Set up minimal Camera nodes.
  const char audio_ns_href[] = "http://ns.xdm.org/photos/1.0/audio/";
  const char audio_mime[] = "audio/mp4";
  string base64_encoded;
  ASSERT_TRUE(EncodeBase64(kMediaData, &base64_encoded));

  int num_cameras = 3;
  xmlNsPtr audio_ns = NewNs(audio_ns_href, "Audio");
  xmlNsPtr camera_ns = NewNs(namespaceHref, XdmConst::Camera());
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

  // Set up VendorInfo node.
  xmlNodePtr vendor_info_node = NewNode(device_ns, "VendorInfo");
  xmlAddChild(device_node, vendor_info_node);
  xmlNsPtr vendor_info_ns = NewNs(namespaceHref, "VendorInfo");
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Manufacturer"),
               ToXmlChar(manufacturer.data()));
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Model"),
               ToXmlChar(model.data()));
  xmlSetNsProp(vendor_info_node, vendor_info_ns, ToXmlChar("Notes"),
               ToXmlChar(notes.data()));

  // Set up Mesh node.
  string base64_encoded_vertex_position;
  ASSERT_TRUE(
      EncodeFloatArrayBase64(kVertexPosition, &base64_encoded_vertex_position));
  string base64_encoded_face_indices;
  ASSERT_TRUE(EncodeIntArrayBase64(kFaceIndices, &base64_encoded_face_indices));
  xmlNodePtr mesh_node = NewNode(device_ns, "Mesh");
  xmlAddChild(device_node, mesh_node);
  xmlNsPtr mesh_ns = NewNs(namespaceHref, XdmConst::Mesh());
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

  // Set up NavigationalConnectivity node.
  xmlNodePtr navigational_connectivity_node =
      NewNode(device_ns, "NavigationalConnectivity");
  xmlAddChild(device_node, navigational_connectivity_node);
  xmlNsPtr navigational_connectivity_ns =
      NewNs(namespaceHref, XdmConst::NavigationalConnectivity());
  xmlNodePtr navigational_edges_node =
      NewNode(navigational_connectivity_ns, "NavigationalEdges");
  xmlAddChild(navigational_connectivity_node, navigational_edges_node);
  xmlNodePtr rdf_seq_node = SetupRdfSeqOfIndices(kNavigationalEdges);
  xmlAddChild(navigational_edges_node, rdf_seq_node);

  // Create a Device object from the XMP metadata.
  std::unique_ptr<Device> device = Device::FromXmp(*xmp_data);
  ASSERT_NE(nullptr, device);
  EXPECT_EQ("1.02", device->GetRevision());

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

  const VendorInfo* read_vendor_info = device->GetVendorInfo();
  ASSERT_NE(nullptr, read_vendor_info);
  EXPECT_EQ(manufacturer, read_vendor_info->GetManufacturer());
  EXPECT_EQ(model, read_vendor_info->GetModel());
  EXPECT_EQ(notes, read_vendor_info->GetNotes());

  const Mesh* read_mesh = device->GetMesh();
  EXPECT_NE(nullptr, read_mesh);
  EXPECT_EQ(kVertexCount, read_mesh->GetVertexCount());
  EXPECT_EQ(kVertexPosition, read_mesh->GetVertexPosition());
  EXPECT_EQ(kFaceCount, read_mesh->GetFaceCount());
  EXPECT_EQ(kFaceIndices, read_mesh->GetFaceIndices());
  EXPECT_EQ(kSoftware, read_mesh->GetSoftware());
  EXPECT_TRUE(read_mesh->GetMetric());

  const NavigationalConnectivity* read_navigational_connectivity =
      device->GetNavigationalConnectivity();
  EXPECT_NE(nullptr, read_navigational_connectivity);
  EXPECT_EQ(kNavigationalEdges,
            read_navigational_connectivity->GetNavigationalEdges());

  xmlFreeNs(audio_ns);
  xmlFreeNs(camera_ns);
  xmlFreeNs(device_ns);
  xmlFreeNs(pose_ns);
  xmlFreeNs(rdf_ns);
  xmlFreeNs(vendor_info_ns);
  xmlFreeNs(mesh_ns);
  xmlFreeNs(navigational_connectivity_ns);
  xmlFreeNs(rdf_seq_node->ns);
}

TEST(Device, ReadMetadataFromXmlFileThenSerialize) {
  // Create a Device object from the .xml file.
  const string testdata_path =
      TestFileAbsolutePath(kDeviceDataPosedCollectionPath);
  std::unique_ptr<Device> device = Device::FromXmlFile(testdata_path);
  ASSERT_NE(nullptr, device);
  EXPECT_EQ("1.02", device->GetRevision());

  // Check DevicePose.
  const DevicePose* device_pose = device->GetDevicePose();
  ASSERT_NE(nullptr, device_pose);
  std::vector<double> values = device_pose->GetPosition();
  ASSERT_EQ(3, values.size());
  EXPECT_DOUBLE_EQ(kDeviceLat, values[0]);
  EXPECT_DOUBLE_EQ(kDeviceLon, values[1]);
  EXPECT_DOUBLE_EQ(kDeviceAlt, values[2]);
  EXPECT_EQ(0, device_pose->GetOrientationRotationXYZAngle().size());

  // Check Cameras.
  const Cameras* cameras = device->GetCameras();
  ASSERT_NE(nullptr, cameras);

  const std::vector<const Camera*>& read_camera_list = cameras->GetCameras();
  EXPECT_EQ(3, read_camera_list.size());
  for (int i = 0; i < kCameraNum; i++) {
    const Camera* camera = read_camera_list[i];
    ASSERT_NE(nullptr, camera);

    const Image* image = camera->GetImage();
    ASSERT_NE(nullptr, image);
    EXPECT_EQ(kImageMime, image->GetMime());
    EXPECT_EQ(kImageId, image->GetImageId());
    EXPECT_TRUE(image->GetData().empty());

    const CameraPose* camera_pose = camera->GetCameraPose();
    ASSERT_NE(nullptr, camera_pose);
    std::vector<double> values = camera_pose->GetOrientationRotationXYZAngle();
    ASSERT_EQ(4, values.size());
    EXPECT_DOUBLE_EQ(kCameraRotationAxisX, values[0]);
    EXPECT_DOUBLE_EQ(kCameraRotationAxisY, values[1]);
    EXPECT_DOUBLE_EQ(kCameraRotationAxisZ, values[2]);
    EXPECT_DOUBLE_EQ(kCameraRotationAngle, values[3]);
    EXPECT_EQ(0, camera_pose->GetPositionXYZ().size());
  }

  const VendorInfo* read_vendor_info = device->GetVendorInfo();
  ASSERT_NE(nullptr, read_vendor_info);
  EXPECT_EQ(manufacturer, read_vendor_info->GetManufacturer());
  EXPECT_EQ(model, read_vendor_info->GetModel());
  EXPECT_EQ(notes, read_vendor_info->GetNotes());

  const Mesh* read_mesh = device->GetMesh();
  EXPECT_NE(nullptr, read_mesh);
  EXPECT_EQ(kVertexCount, read_mesh->GetVertexCount());
  EXPECT_EQ(kVertexPosition, read_mesh->GetVertexPosition());
  EXPECT_EQ(kFaceCount, read_mesh->GetFaceCount());
  EXPECT_EQ(kFaceIndices, read_mesh->GetFaceIndices());
  EXPECT_EQ(kSoftware, read_mesh->GetSoftware());
  EXPECT_FALSE(read_mesh->GetMetric());

  const NavigationalConnectivity* read_navigational_connectivity =
      device->GetNavigationalConnectivity();
  EXPECT_NE(nullptr, read_navigational_connectivity);
  EXPECT_EQ(kNavigationalEdges,
            read_navigational_connectivity->GetNavigationalEdges());

  // Serialize Device and verify the serialized string is the same as the
  // content in the .xml file.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  ASSERT_TRUE(device->SerializeToXmp(xmp_data.get()));
  std::string expected_xdm_data;
  ReadFileToStringOrDie(testdata_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xmp_data->ExtendedSection()));
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
