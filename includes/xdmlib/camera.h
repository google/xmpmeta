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

#ifndef XMPMETA_XDM_CAMERA_H_
#define XMPMETA_XDM_CAMERA_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xdmlib/audio.h"
#include "xdmlib/camera_pose.h"
#include "xdmlib/element.h"
#include "xdmlib/equirect_model.h"
#include "xdmlib/image.h"
#include "xdmlib/imaging_model.h"
#include "xdmlib/vendor_info.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

// Implements the Camera element from the XDM specification, with
// serialization and deserialization.
class Camera : public Element {
 public:
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  bool Serialize(xml::Serializer* serializer) const override;

  // Creates a Camera from the given objects.
  // Optional Camera elements can be null.
  // TODO(miraleung): Describe restrictions here when all objects are
  // checked in.
  static std::unique_ptr<Camera> FromData(
      std::unique_ptr<Audio> audio, std::unique_ptr<Image> image,
      std::unique_ptr<CameraPose> camera_pose,
      std::unique_ptr<VendorInfo> vendor_info,
      std::unique_ptr<ImagingModel> imaging_model);

  // Returns the deserialized Camera object, null if parsing fails.
  // TODO(miraleung): Describe restrictions here when all objects are
  // checked in.
  static std::unique_ptr<Camera> FromDeserializer(
      const xml::Deserializer& parent_deserializer);

  // Getters.
  const Audio* GetAudio() const;
  const Image* GetImage() const;
  const CameraPose* GetCameraPose() const;
  const VendorInfo* GetVendorInfo() const;
  const ImagingModel* GetImagingModel() const;

  // Disallow copying.
  Camera(const Camera&) = delete;
  void operator=(const Camera&) = delete;

 private:
  Camera() = default;

  bool ParseChildElements(const xml::Deserializer& deserializer);

  // TODO(miraleung): Add rest of the objects here when they're checked in.
  std::unique_ptr<Audio> audio_;
  std::unique_ptr<Image> image_;

  // Optional elements.
  std::unique_ptr<CameraPose> camera_pose_;
  std::unique_ptr<VendorInfo> vendor_info_;
  std::unique_ptr<ImagingModel> imaging_model_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_CAMERA_H_
