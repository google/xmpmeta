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

#ifndef XMPMETA_XDM_DEVICE_POSE_H_
#define XMPMETA_XDM_DEVICE_POSE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "xdmlib/element.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

/**
 * Implements the DevicePose element in the XDM specification, with
 * serialization and deserialization.
 * See xdm.org.
 */
class DevicePose : public Element {
 public:
  // Appends child elements' namespaces' and their respective hrefs to the
  // given collection, and any parent nodes' names to prefix_names.
  // Key: Name of the namespace.
  // Value: Full namespace URL.
  // Example: ("DevicePose", "http://ns.xdm.org/photos/1.0/devicepose/")
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  // Serializes this object. Returns true on success.
  bool Serialize(xml::Serializer* serializer) const override;

  // Creates a DevicePose from the given data.
  // The order of values in position is latitude, longitude, altitude..
  // The order of values in orientation is the rotation x, y, z angle, where
  // x, y, z are the rotation axis coordinates, and angle is the rotation angle
  // in radians.
  // rotation angle in angle-axis format (radians).
  // Position and orientation are in raw coordinates, and will be stored as
  // normalied values. Please refer to the XDM specification for the device
  // coordinate system.
  // At least one valid position or orientation must be provided. These
  // arguments will be ignored if the vector is of the wrong size.
  static std::unique_ptr<DevicePose>
      FromData(const std::vector<double>& position,
               const std::vector<double>& orientation,
               const int64 timestamp = -1);

  // Returns the deserialized XdmAudio; null if parsing fails.
  // The returned pointer is owned by the caller.
  static std::unique_ptr<DevicePose>
      FromDeserializer(const xml::Deserializer& parent_deserializer);

  // Returns true if the device's position is provided.
  bool HasPosition() const;

  // Returns true if the device's orientation is provided.
  bool HasOrientation() const;

  // Returns the device's position fields, or an empty vector if they are
  // not present.
  const std::vector<double>& GetPosition() const;

  // Returns the device's orientation fields, or an empty vector if they are
  // not present.
  const std::vector<double>& GetOrientationRotationXYZAngle() const;

  // Timestamp.
  int64 GetTimestamp() const;

  // Disallow copying.
  DevicePose(const DevicePose&) = delete;
  void operator=(const DevicePose&) = delete;

 private:
  DevicePose();

  // Extracts device pose fields.
  bool ParseDevicePoseFields(const xml::Deserializer& deserializer);

  // Position variables, in meters relative to camera 0.
  // If providing position data, all three fields must be set.
  // Stored in normalized form.
  std::vector<double> position_;

  // Orientation variables.
  // If providing orientation data, all four fields must be set.
  // Stored in normalized form.
  std::vector<double> orientation_;

  // Timestamp is Epoch time in milliseconds.
  int64 timestamp_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_DEVICE_POSE_H_
