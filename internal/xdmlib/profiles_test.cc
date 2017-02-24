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

#include "xdmlib/profiles.h"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "xdmlib/const.h"
#include "xdmlib/profile.h"
#include "xmpmeta/base64.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer_impl.h"
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

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/profile/";

// Test data constants.
const char kProfilesDataPath[] = "xdm/profiles_testdata.txt";

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

// Returns an XML rdf:Seq node with a list of the given values.
xmlNodePtr SetupRdfSeqOfIndices(const std::vector<int>& values,
                                const xmlNsPtr rdf_ns) {
  xmlNodePtr rdf_seq_node = NewNode(rdf_ns, XmlConst::RdfSeq());
  for (int value : values) {
    xmlNodePtr rdf_li_node = NewNode(rdf_ns, XmlConst::RdfLi());
    xmlNodeSetContent(
        rdf_li_node, ToXmlChar(std::to_string(value).data()));
    xmlAddChild(rdf_seq_node, rdf_li_node);
  }
  return rdf_seq_node;
}

std::unique_ptr<Profile> CreateVrPhotoProfile() {
  return Profile::FromData("VRPhoto", {0, 1});
}

std::unique_ptr<Profile> CreateDepthPhotoProfile() {
  return Profile::FromData("DepthPhoto", {0});
}

void ExpectProfileEquals(const Profile& profile_1, const Profile& profile_2) {
  EXPECT_EQ(profile_1.GetType(), profile_2.GetType());
  std::vector<int> indices_1 = profile_1.GetCameraIndices();
  std::vector<int> indices_2 = profile_2.GetCameraIndices();
  ASSERT_EQ(indices_1.size(), indices_2.size());
  for (int i = 0; i < indices_1.size(); i++) {
    EXPECT_EQ(indices_1[i], indices_2[i]);
  }
}

TEST(Profiles, GetNamespaces) {
  std::unique_ptr<Profile> profile = CreateVrPhotoProfile();
  std::vector<std::unique_ptr<Profile>> profile_list;
  profile_list.emplace_back(std::move(profile));
  std::unique_ptr<Profiles> profiles =
      Profiles::FromProfileArray(&profile_list);
  ASSERT_NE(nullptr, profiles);

  string prefix("Profile");
  std::unordered_map<string, string> ns_name_href_map;
  ASSERT_TRUE(ns_name_href_map.empty());
  profiles->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(1, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));

  // Same namespace will not be added again.
  profiles->GetNamespaces(&ns_name_href_map);
  EXPECT_EQ(1, ns_name_href_map.size());
  EXPECT_EQ(string(kNamespaceHref), string(ns_name_href_map[prefix]));
}

TEST(Profiles, FromProfileArray) {
  std::unique_ptr<Profile> vr_profile = CreateVrPhotoProfile();
  std::unique_ptr<Profile> depth_profile = CreateDepthPhotoProfile();
  std::vector<std::unique_ptr<Profile>> profile_list;
  profile_list.emplace_back(std::move(vr_profile));
  profile_list.emplace_back(std::move(depth_profile));
  int num_profiles = profile_list.size();

  std::unique_ptr<Profiles> profiles =
      Profiles::FromProfileArray(&profile_list);
  ASSERT_NE(nullptr, profiles);
  const std::vector<const Profile*> read_profile_list = profiles->GetProfiles();
  EXPECT_EQ(num_profiles, read_profile_list.size());
  // Pointer ownership of vr_profile and depth_profile has been transferred to
  // profiles.
  ExpectProfileEquals(*CreateVrPhotoProfile(), *read_profile_list[0]);
  ExpectProfileEquals(*CreateDepthPhotoProfile(), *read_profile_list[1]);
}

TEST(Profiles, FromEmptyProfileList) {
  std::vector<std::unique_ptr<Profile>> profile_list;
  std::unique_ptr<Profiles> profiles =
      Profiles::FromProfileArray(&profile_list);
  ASSERT_EQ(nullptr, profiles);
}

TEST(Profiles, SerializeWithoutRdfPrefix) {
  std::unique_ptr<Profile> vr_profile = CreateVrPhotoProfile();
  std::unique_ptr<Profile> depth_profile = CreateDepthPhotoProfile();
  std::vector<std::unique_ptr<Profile>> profile_list;
  profile_list.emplace_back(std::move(vr_profile));
  profile_list.emplace_back(std::move(depth_profile));

  std::unique_ptr<Profiles> profiles =
      Profiles::FromProfileArray(&profile_list);
  ASSERT_NE(nullptr, profiles);

  // Create XML serializer.
  const char device_name[] = "Device";
  const char profile_name[] = "Profile";
  const char namespaceHref[] = "http://notarealh.ref";

  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, NewNs(namespaceHref, device_name));
  namespaces.emplace(profile_name, NewNs(kNamespaceHref, profile_name));

  xmlNodePtr device_node = NewNode(nullptr, device_name);
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  ASSERT_FALSE(profiles->Serialize(&serializer));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Profiles, Serialize) {
  std::unique_ptr<Profile> vr_profile = CreateVrPhotoProfile();
  std::unique_ptr<Profile> depth_profile = CreateDepthPhotoProfile();
  std::vector<std::unique_ptr<Profile>> profile_list;
  profile_list.emplace_back(std::move(vr_profile));
  profile_list.emplace_back(std::move(depth_profile));

  std::unique_ptr<Profiles> profiles =
      Profiles::FromProfileArray(&profile_list);
  ASSERT_NE(nullptr, profiles);

  // Create XML serializer.
  const char device_name[] = "Device";
  const char profile_name[] = "Profile";
  const char namespaceHref[] = "http://notarealh.ref";

  std::unordered_map<string, xmlNsPtr> namespaces;
  namespaces.emplace(device_name, NewNs(namespaceHref, device_name));
  namespaces.emplace(profile_name, NewNs(kNamespaceHref, profile_name));
  namespaces.emplace(XmlConst::RdfPrefix(),
                     NewNs(XmlConst::RdfNodeNs(), XmlConst::RdfPrefix()));

  xmlNodePtr device_node = NewNode(nullptr, device_name);
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar(XmlConst::Version()));
  xmlDocSetRootElement(xml_doc, device_node);

  // Create serializer.
  SerializerImpl serializer(namespaces, device_node);
  ASSERT_TRUE(profiles->Serialize(&serializer));

  const string testdata_path = TestFileAbsolutePath(kProfilesDataPath);
  std::string expected_xdm_data;
  ReadFileToStringOrDie(testdata_path, &expected_xdm_data);
  EXPECT_EQ(expected_xdm_data, XmlDocToString(xml_doc));

  // Free all XML objects.
  for (const auto& entry : namespaces) {
    xmlFreeNs(entry.second);
  }
  xmlFreeDoc(xml_doc);
}

TEST(Profiles, ReadMetadata) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  xmlNodePtr description_node =
      GetFirstDescriptionElement(xmp_data->ExtendedSection());

  // XDM Device node.
  xmlNodePtr device_node = NewNode(nullptr, "Device");
  xmlAddChild(description_node, device_node);

  // Device:Profiles node.
  xmlNsPtr device_ns = NewNs("http://fakeh.ref", XdmConst::Device());
  xmlNodePtr profiles_node = NewNode(device_ns, XdmConst::Profiles());
  xmlAddChild(device_node, profiles_node);

  // rdf:Seq node.
  xmlNsPtr rdf_ns = NewNs("http://fakeh.ref", XmlConst::RdfPrefix());
  xmlNodePtr rdf_seq_node = NewNode(rdf_ns, XmlConst::RdfSeq());
  xmlAddChild(profiles_node, rdf_seq_node);

  // Set up Profile nodes.
  const char profile_ns_href[] = "http://ns.xdm.org/photos/1.0/profile/";
  const char profile_prefix[] = "Profile";
  xmlNsPtr profile_ns = NewNs(profile_ns_href, profile_prefix);


  // VR photo profile.
  std::vector<int> indices = {0, 1};
  xmlNodePtr vr_indices_rdf_seq_node = SetupRdfSeqOfIndices(indices, rdf_ns);
  xmlNodePtr vr_profile_camera_indices_node =
      NewNode(profile_ns, "CameraIndices");
  xmlAddChild(vr_profile_camera_indices_node, vr_indices_rdf_seq_node);

  xmlNodePtr vr_profile_node = NewNode(device_ns, profile_prefix);
  xmlAddChild(vr_profile_node, vr_profile_camera_indices_node);
  xmlSetNsProp(vr_profile_node, profile_ns, ToXmlChar("Type"),
               ToXmlChar("VRPhoto"));

  // Depth photo profile.
  indices = {0};
  xmlNodePtr depth_camera_indices_rdf_seq_node =
      SetupRdfSeqOfIndices(indices, rdf_ns);
  xmlNodePtr depth_profile_camera_indices_node =
      NewNode(profile_ns, "CameraIndices");
  xmlAddChild(depth_profile_camera_indices_node,
              depth_camera_indices_rdf_seq_node);

  xmlNodePtr depth_profile_node = NewNode(device_ns, profile_prefix);
  xmlAddChild(depth_profile_node, depth_profile_camera_indices_node);
  xmlSetNsProp(depth_profile_node, profile_ns, ToXmlChar("Type"),
               ToXmlChar("DepthPhoto"));


  // Insert profile nodes into the rdf:Seq list of profiles.
  xmlNodePtr vr_profile_rdf_li_node = NewNode(rdf_ns, XmlConst::RdfLi());
  xmlAddChild(vr_profile_rdf_li_node, vr_profile_node);
  xmlAddChild(rdf_seq_node, vr_profile_rdf_li_node);


  xmlNodePtr depth_profile_rdf_li_node = NewNode(rdf_ns, XmlConst::RdfLi());
  xmlAddChild(depth_profile_rdf_li_node, depth_profile_node);
  xmlAddChild(rdf_seq_node, depth_profile_rdf_li_node);

  // Create a Profiles object from the metadata.
  DeserializerImpl deserializer(description_node);
  std::unique_ptr<Profiles> profiles = Profiles::FromDeserializer(deserializer);
  ASSERT_NE(nullptr, profiles.get());

  const std::vector<const Profile*> read_profile_list = profiles->GetProfiles();
  EXPECT_EQ(2, read_profile_list.size());
  const std::unique_ptr<Profile> vr_profile = CreateVrPhotoProfile();
  const std::unique_ptr<Profile> depth_profile = CreateDepthPhotoProfile();
  ExpectProfileEquals(*vr_profile, *read_profile_list[0]);
  ExpectProfileEquals(*depth_profile, *read_profile_list[1]);

  xmlFreeNs(device_ns);
  xmlFreeNs(profile_ns);
  xmlFreeNs(rdf_ns);
}

}  // namespace
}  // namespace xdm
}  // namespace xmpmeta
