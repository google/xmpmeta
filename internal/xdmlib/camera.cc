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

#include "glog/logging.h"
#include "xdmlib/const.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {
namespace {

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/camera/";

}  // namespace

void Camera::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list is null";
    return;
  }
  ns_name_href_map->emplace(XdmConst::Camera(), kNamespaceHref);
  if (audio_) {
    audio_->GetNamespaces(ns_name_href_map);
  }
  if (image_) {
    image_->GetNamespaces(ns_name_href_map);
  }
  if (camera_pose_) {
    camera_pose_->GetNamespaces(ns_name_href_map);
  }
  if (vendor_info_) {
    vendor_info_->GetNamespaces(ns_name_href_map);
  }
  if (imaging_model_) {
    imaging_model_->GetNamespaces(ns_name_href_map);
  }
}

std::unique_ptr<Camera> Camera::FromData(
    std::unique_ptr<Audio> audio, std::unique_ptr<Image> image,
    std::unique_ptr<CameraPose> camera_pose,
    std::unique_ptr<VendorInfo> vendor_info,
    std::unique_ptr<ImagingModel> imaging_model) {
  // TODO(miraleung): Add restrictive checks here for required elements
  // once all they are all checked in. Remove this check.
  if (audio == nullptr && image == nullptr) {
    LOG(ERROR) << "Camera must have at least one child element";
    return nullptr;
  }
  std::unique_ptr<Camera> camera(new Camera());
  camera->audio_ = std::move(audio);
  camera->image_ = std::move(image);
  camera->camera_pose_ = std::move(camera_pose);
  camera->vendor_info_ = std::move(vendor_info);
  camera->imaging_model_ = std::move(imaging_model);

  return camera;
}

std::unique_ptr<Camera> Camera::FromDeserializer(
    const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(XdmConst::Camera()), XdmConst::Camera());
  if (deserializer == nullptr) {
    return nullptr;
  }
  std::unique_ptr<Camera> camera(new Camera());
  if (!camera->ParseChildElements(*deserializer)) {
    return nullptr;
  }
  return camera;
}

const Audio* Camera::GetAudio() const { return audio_.get(); }
const Image* Camera::GetImage() const { return image_.get(); }
const CameraPose* Camera::GetCameraPose() const { return camera_pose_.get(); }
const VendorInfo* Camera::GetVendorInfo() const { return vendor_info_.get(); }
const ImagingModel* Camera::GetImagingModel() const {
  return imaging_model_.get();
}

bool Camera::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }

  // At least one of the below elements are required, and hence must be
  // successfully serialized.
  bool success = false;
  if (audio_ != nullptr) {
    std::unique_ptr<Serializer> audio_serializer = serializer->CreateSerializer(
        XdmConst::Namespace(XdmConst::Audio()), XdmConst::Audio());
    success |= audio_->Serialize(audio_serializer.get());
  }
  if (image_ != nullptr) {
    std::unique_ptr<Serializer> image_serializer = serializer->CreateSerializer(
        XdmConst::Namespace(XdmConst::Image()), XdmConst::Image());
    success |= image_->Serialize(image_serializer.get());
  }

  if (!success) {
    return false;
  }

  // Serialize optional elements.
  if (camera_pose_ != nullptr) {
    std::unique_ptr<Serializer> camera_pose_serializer =
        serializer->CreateSerializer(
            XdmConst::Namespace(XdmConst::CameraPose()),
            XdmConst::CameraPose());
    success &= camera_pose_->Serialize(camera_pose_serializer.get());
  }

  if (vendor_info_ != nullptr) {
    std::unique_ptr<Serializer> vendor_info_serializer =
        serializer->CreateSerializer(XdmConst::Camera(),
                                     XdmConst::VendorInfo());
    success &= vendor_info_->Serialize(vendor_info_serializer.get());
  }

  if (imaging_model_ != nullptr) {
    std::unique_ptr<Serializer> imaging_model_serializer =
        serializer->CreateSerializer(
            XdmConst::Namespace(imaging_model_->GetType()),
            imaging_model_->GetType());
    success &= imaging_model_->Serialize(imaging_model_serializer.get());
  }

  return success;
}

// Private methods.
bool Camera::ParseChildElements(const Deserializer& deserializer) {
  // TODO(miraleung): Add restriction checks here once all elements are
  // checked in.

  // At least one of the elements below must be present in Camera, and hence
  // at least one of these parsings must be successful.
  bool success = false;
  std::unique_ptr<Audio> audio = Audio::FromDeserializer(deserializer);

  if (audio.get()) {
    success = true;
    audio_ = std::move(audio);
  }

  std::unique_ptr<Image> image = Image::FromDeserializer(deserializer);
  if (image.get()) {
    success = true;
    image_ = std::move(image);
  }

  if (!success) {
    return false;
  }

  // Parse optional elements.
  std::unique_ptr<CameraPose> camera_pose =
      CameraPose::FromDeserializer(deserializer);
  if (camera_pose.get()) {
    camera_pose_ = std::move(camera_pose);
  }

  std::unique_ptr<VendorInfo> vendor_info =
      VendorInfo::FromDeserializer(deserializer, XdmConst::Camera());
  if (vendor_info.get()) {
    vendor_info_ = std::move(vendor_info);
  }

  std::unique_ptr<EquirectModel> equirect_model =
      EquirectModel::FromDeserializer(deserializer);
  if (equirect_model != nullptr) {
    imaging_model_ = std::move(equirect_model);
  }
  // TODO(hlou): Add other imaging models.
  return success;
}

}  // namespace xdm
}  // namespace xmpmeta
