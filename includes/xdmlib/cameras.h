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

#ifndef XMPMETA_XDM_CAMERAS_H_
#define XMPMETA_XDM_CAMERAS_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xdmlib/camera.h"
#include "xdmlib/element.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

// Implements the Device:Cameras field from the XDM specification, with
// serialization and deserialization for its child Camera elements.
class Cameras : public Element {
 public:
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  bool Serialize(xml::Serializer* serializer) const override;

  // Creates this object from the given cameras. Returns null if the list is
  // empty.
  // If creation succeeds, ownership of the Camera objects are transferred to
  // the resulting Cameras object. The vector of Camera objects will be cleared.
  static std::unique_ptr<Cameras>
      FromCameraArray(std::vector<std::unique_ptr<Camera>>* cameras);

  // Returns the deserialized cameras in a Cameras object, null if parsing
  // failed for all the cameras.
  static std::unique_ptr<Cameras>
      FromDeserializer(const xml::Deserializer& parent_deserializer);

  // Returns the list of cameras.
  const std::vector<const Camera*> GetCameras() const;

  // Disallow copying.
  Cameras(const Cameras&) = delete;
  void operator=(const Cameras&) = delete;

 private:
  Cameras();

  std::vector<std::unique_ptr<Camera>> camera_list_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_CAMERAS_H_
