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

#include "glog/logging.h"
#include "xdmlib/const.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {

void Profiles::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr || profile_list_.empty()) {
    LOG(ERROR) << "Namespace list is null or profile list is empty";
    return;
  }
  for (const auto& profile : profile_list_) {
    profile->GetNamespaces(ns_name_href_map);
  }
}

std::unique_ptr<Profiles> Profiles::FromProfileArray(
    std::vector<std::unique_ptr<Profile>>* profile_list) {
  if (profile_list->empty()) {
    LOG(ERROR) << "Profile list is empty";
    return nullptr;
  }
  std::unique_ptr<Profiles> profiles(new Profiles());
  profiles->profile_list_ = std::move(*profile_list);
  return profiles;
}

std::unique_ptr<Profiles>
Profiles::FromDeserializer(const Deserializer& parent_deserializer) {
  std::unique_ptr<Profiles> profiles(new Profiles());
  int i = 0;
  for (std::unique_ptr<Deserializer> deserializer =
       parent_deserializer.CreateDeserializerFromListElementAt(
           XdmConst::Namespace(XdmConst::Profiles()), XdmConst::Profiles(), i);
       deserializer != nullptr;
       deserializer =
          parent_deserializer.CreateDeserializerFromListElementAt(
              XdmConst::Namespace(XdmConst::Profiles()), XdmConst::Profiles(),
              ++i)) {
    std::unique_ptr<Profile> profile = Profile::FromDeserializer(*deserializer);
    if (profile != nullptr) {
      profiles->profile_list_.emplace_back(std::move(profile));
    }
  }

  if (profiles->profile_list_.empty()) {
    return nullptr;
  }
  return profiles;
}

const std::vector<const Profile*> Profiles::GetProfiles() const {
  std::vector<const Profile*> profile_list;
  for (const auto& profile : profile_list_) {
    profile_list.push_back(profile.get());
  }
  return profile_list;
}

bool Profiles::Serialize(Serializer* serializer) const {
  if (profile_list_.empty()) {
    LOG(ERROR) << "Profile list is empty";
    return false;
  }
  bool success = true;
  int i = 0;
  std::unique_ptr<Serializer> profiles_serializer =
      serializer->CreateListSerializer(
          XdmConst::Namespace(XdmConst::Profiles()), XdmConst::Profiles());
  if (profiles_serializer == nullptr) {
    // Error is logged in Serializer.
    return false;
  }
  for (const auto& profile : profile_list_) {
    std::unique_ptr<Serializer> profile_serializer =
        profiles_serializer->CreateItemSerializer(
            XdmConst::Namespace(XdmConst::Profile()), XdmConst::Profile());
    if (profile_serializer == nullptr) {
      continue;
    }
    success &= profile->Serialize(profile_serializer.get());
    if (!success) {
      LOG(ERROR) << "Could not serialize profile " << i;
    }
    ++i;
  }
  return success;
}

}  // namespace xdm
}  // namespace xmpmeta
