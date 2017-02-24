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

#include "xdmlib/profile.h"

#include "glog/logging.h"
#include "xdmlib/const.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {
namespace {

const char kType[] = "Type";
const char kCameraIndices[] = "CameraIndices";

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/profile/";

// Profile type names.
const char kVrPhoto[] = "VRPhoto";
const char kDepthPhoto[] = "DepthPhoto";

// Profile camera indices' sizes.
const size_t kVrPhotoIndicesSize = 2;
const size_t kDepthPhotoIndicesSize = 1;

// Returns true if the type is unsupported, or if the type is supported in the
// XDM Profile element and the size of the camera indices matches that
// specified in the spec.
bool ValidateKnownTypeAndIndices(const string& type,
                                 size_t camera_indices_size) {
  bool isVrPhoto = (kVrPhoto == type);
  bool isDepthPhoto = (kDepthPhoto == type);
  if (!isVrPhoto && !isDepthPhoto) {
    return true;
  }
  bool matches =
      (isVrPhoto && camera_indices_size >= kVrPhotoIndicesSize) ||
      (isDepthPhoto && camera_indices_size >= kDepthPhotoIndicesSize);
  if (!matches) {
    LOG(WARNING) << "Size of camera indices for "
                 << (isVrPhoto ? kVrPhoto : kDepthPhoto)
                 << " must be at least "
                 << (isVrPhoto ? kVrPhotoIndicesSize : kDepthPhotoIndicesSize);
  }
  return matches;
}

}  // namespace

// Private constructor.
Profile::Profile(const string& type, const std::vector<int>& camera_indices) :
    type_(type), camera_indices_(camera_indices) {}

// Public methods.
void Profile::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->emplace(XdmConst::Profile(), kNamespaceHref);
}

std::unique_ptr<Profile>
Profile::FromData(const string& type, const std::vector<int>& camera_indices) {
  if (type.empty()) {
    LOG(ERROR) << "Profile must have a type";
    return nullptr;
  }
  // Check that the camera indices' length is at least the size of that
  // specified for the type. This has no restrictions on unsupported profile
  // types.
  if (!ValidateKnownTypeAndIndices(type, camera_indices.size())) {
    return nullptr;
  }

  return std::unique_ptr<Profile>(new Profile(type, camera_indices));
}

std::unique_ptr<Profile>
Profile::FromDeserializer(const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(XdmConst::Profile()), XdmConst::Profile());
  if (deserializer == nullptr) {
    return nullptr;
  }
  std::unique_ptr<Profile> profile(new Profile("", {}));
  if (!deserializer->ParseString(XdmConst::Profile(), kType, &profile->type_)) {
    return nullptr;
  }
  deserializer->ParseIntArray(XdmConst::Profile(), kCameraIndices,
                              &profile->camera_indices_);
  if (!ValidateKnownTypeAndIndices(profile->type_,
                                   profile->camera_indices_.size())) {
    return nullptr;
  }
  return profile;
}

const string& Profile::GetType() const { return type_; }

const std::vector<int>& Profile::GetCameraIndices() const {
  return camera_indices_;
}

bool Profile::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }
  if (!serializer->WriteProperty(XdmConst::Profile(), kType, type_)) {
    return false;
  }
  if (camera_indices_.empty()) {
    return true;
  }
  return serializer->WriteIntArray(XdmConst::Profile(), kCameraIndices,
                                   camera_indices_);
}

}  // namespace xdm
}  // namespace xmpmeta
