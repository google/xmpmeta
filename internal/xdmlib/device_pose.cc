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

#include "xdmlib/device_pose.h"

#include <math.h>

#include "glog/logging.h"
#include "xdmlib/const.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {
namespace {

const char kLatitude[] = "Latitude";
const char kLongitude[] = "Longitude";
const char kAltitude[] = "Altitude";
const char kRotationAxisX[] = "RotationAxisX";
const char kRotationAxisY[] = "RotationAxisY";
const char kRotationAxisZ[] = "RotationAxisZ";
const char kRotationAngle[] = "RotationAngle";
const char kTimestamp[] = "Timestamp";
const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/devicepose/";

const std::vector<double>
NormalizeAxisAngle(const std::vector<double>& coords) {
  if (coords.size() < 4) {
    return std::vector<double>();
  }
  double length = sqrt((coords[0] * coords[0]) +
                       (coords[1] * coords[1]) +
                       (coords[2] * coords[2]));
  const std::vector<double> normalized =
      { coords[0] / length, coords[1] / length, coords[2] / length, coords[3] };
  return normalized;
}

}  // namespace

// Private constructor.
DevicePose::DevicePose() : timestamp_(-1) {}

// Public methods.
void DevicePose::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->emplace(XdmConst::DevicePose(), kNamespaceHref);
}

std::unique_ptr<DevicePose>
DevicePose::FromData(const std::vector<double>& position,
                     const std::vector<double>& orientation,
                     const int64 timestamp) {
  if (position.empty() && orientation.empty()) {
    LOG(ERROR) << "Either position or orientation must be provided";
    return nullptr;
  }

  std::unique_ptr<DevicePose> device_pose(new DevicePose());
  if (position.size() >= 3) {
    device_pose->position_ = position;
  }

  if (orientation.size() >= 4) {
    device_pose->orientation_ = NormalizeAxisAngle(orientation);
  }

  if (timestamp >= 0) {
    device_pose->timestamp_ = timestamp;
  }

  return device_pose;
}

std::unique_ptr<DevicePose>
DevicePose::FromDeserializer(const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(XdmConst::DevicePose()), XdmConst::DevicePose());
  if (deserializer == nullptr) {
    return nullptr;
  }
  std::unique_ptr<DevicePose> device_pose(new DevicePose());
  if (!device_pose->ParseDevicePoseFields(*deserializer)) {
    return nullptr;
  }
  return device_pose;
}

bool DevicePose::HasPosition() const { return position_.size() == 3; }
bool DevicePose::HasOrientation() const { return orientation_.size() == 4; }

const std::vector<double>&
DevicePose::GetPosition() const { return position_; }

const std::vector<double>&
DevicePose::GetOrientationRotationXYZAngle() const { return orientation_; }

int64 DevicePose::GetTimestamp() const { return timestamp_; }

bool DevicePose::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }

  if (!HasPosition() && !HasOrientation()) {
    LOG(ERROR) << "Device pose has neither position nor orientation";
    return false;
  }

  bool success = true;
  if (position_.size() == 3) {
    success &=
        serializer->WriteProperty(XdmConst::DevicePose(), kLatitude,
                                  std::to_string(position_[0])) &&
        serializer->WriteProperty(XdmConst::DevicePose(), kLongitude,
                                   std::to_string(position_[1])) &&
        serializer->WriteProperty(XdmConst::DevicePose(), kAltitude,
                                  std::to_string(position_[2]));
  }

  if (orientation_.size() == 4) {
    success &=
        serializer->WriteProperty(XdmConst::DevicePose(), kRotationAxisX,
                                  std::to_string(orientation_[0])) &&
        serializer->WriteProperty(XdmConst::DevicePose(), kRotationAxisY,
                                  std::to_string(orientation_[1])) &&
        serializer->WriteProperty(XdmConst::DevicePose(), kRotationAxisZ,
                                  std::to_string(orientation_[2])) &&
        serializer->WriteProperty(XdmConst::DevicePose(), kRotationAngle,
                                  std::to_string(orientation_[3]));
  }

  if (timestamp_ >= 0) {
    serializer->WriteProperty(XdmConst::DevicePose(), kTimestamp,
                              std::to_string(timestamp_));
  }

  return success;
}

// Private methods.
bool DevicePose::ParseDevicePoseFields(const Deserializer& deserializer) {
  double lat, lon, alt;
  // If a position field is present, the rest must be as well.
  if (deserializer.ParseDouble(XdmConst::DevicePose(), kLatitude, &lat)) {
    if (!deserializer.ParseDouble(XdmConst::DevicePose(), kLongitude, &lon)) {
      return false;
    }
    if (!deserializer.ParseDouble(XdmConst::DevicePose(), kAltitude, &alt)) {
      return false;
    }
    position_ = { lat, lon, alt };
  }

  // Same for orientation.
  double x, y, z;
  if (deserializer.ParseDouble(XdmConst::DevicePose(), kRotationAxisX, &x)) {
    if (!deserializer.ParseDouble(XdmConst::DevicePose(), kRotationAxisY,
                                  &y)) {
      return false;
    }
    if (!deserializer.ParseDouble(XdmConst::DevicePose(), kRotationAxisZ,
                                  &z)) {
      return false;
    }
    double angle;
    if (!deserializer.ParseDouble(XdmConst::DevicePose(), kRotationAngle,
                                  &angle)) {
      return false;
    }
    std::vector<double> axis_angle = { x, y, z, angle };
    orientation_ = NormalizeAxisAngle(axis_angle);
  }

  if (position_.size() < 3 && orientation_.size() < 4) {
    return false;
  }

  deserializer.ParseLong(XdmConst::DevicePose(), kTimestamp, &timestamp_);
  return true;
}

}  // namespace xdm
}  // namespace xmpmeta
