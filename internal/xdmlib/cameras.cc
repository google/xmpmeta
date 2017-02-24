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

#include "xdmlib/cameras.h"

#include "glog/logging.h"
#include "xdmlib/const.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {

const char kNodeName[] = "Cameras";
const char kCameraName[] = "Camera";

// Private constructor.
Cameras::Cameras() {}

// Public methods.
void Cameras::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr || camera_list_.empty()) {
    LOG(ERROR) << "Namespace list is null or camera list is empty";
    return;
  }
  for (const auto& camera : camera_list_) {
    camera->GetNamespaces(ns_name_href_map);
  }
}

std::unique_ptr<Cameras>
Cameras::FromCameraArray(std::vector<std::unique_ptr<Camera>>* camera_list) {
  if (camera_list == nullptr || camera_list->empty()) {
    LOG(ERROR) << "Camera list is empty";
    return nullptr;
  }
  std::unique_ptr<Cameras> cameras(new Cameras());
  cameras->camera_list_ = std::move(*camera_list);
  return cameras;
}

std::unique_ptr<Cameras>
Cameras::FromDeserializer(const Deserializer& parent_deserializer) {
  std::unique_ptr<Cameras> cameras(new Cameras());
  int i = 0;
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializerFromListElementAt(
          XdmConst::Namespace(kNodeName), kNodeName, 0);
  while (deserializer) {
    std::unique_ptr<Camera> camera = Camera::FromDeserializer(*deserializer);
    if (camera == nullptr) {
      LOG(ERROR) << "Unable to deserialize a camera";
      return nullptr;
    }
    cameras->camera_list_.emplace_back(std::move(camera));
    deserializer =
        parent_deserializer.CreateDeserializerFromListElementAt(
            XdmConst::Namespace(kNodeName), kNodeName, ++i);
  }

  if (cameras->camera_list_.empty()) {
    return nullptr;
  }
  return cameras;
}

const std::vector<const Camera*> Cameras::GetCameras() const {
  std::vector<const Camera*> camera_list;
  for (const auto& camera : camera_list_) {
    camera_list.push_back(camera.get());
  }
  return camera_list;
}

bool Cameras::Serialize(Serializer* serializer) const {
  if (camera_list_.empty()) {
    LOG(ERROR) << "Camera list is empty";
    return false;
  }
  std::unique_ptr<Serializer> cameras_serializer =
      serializer->CreateListSerializer(XdmConst::Namespace(kNodeName),
                                       kNodeName);
  if (cameras_serializer == nullptr) {
    // Error is logged in Serializer.
    return false;
  }
  for (int i = 0; i < camera_list_.size(); i++) {
    std::unique_ptr<Serializer> camera_serializer =
        cameras_serializer->CreateItemSerializer(
            XdmConst::Namespace(kCameraName), kCameraName);
    if (camera_serializer == nullptr) {
      LOG(ERROR) << "Could not create a list item serializer for Camera";
      return false;
    }
    if (!camera_list_[i]->Serialize(camera_serializer.get())) {
      LOG(ERROR) << "Could not serialize camera " << i;
      return false;
    }
  }
  return true;
}

}  // namespace xdm
}  // namespace xmpmeta
