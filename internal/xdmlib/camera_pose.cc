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

#include "xdmlib/camera_pose.h"

#include <math.h>

#include "glog/logging.h"
#include "xdmlib/const.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {
namespace {

const char kPositionX[] = "PositionX";
const char kPositionY[] = "PositionY";
const char kPositionZ[] = "PositionZ";
const char kRotationAxisX[] = "RotationAxisX";
const char kRotationAxisY[] = "RotationAxisY";
const char kRotationAxisZ[] = "RotationAxisZ";
const char kRotationAngle[] = "RotationAngle";
const char kTimestamp[] = "Timestamp";
const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/camerapose/";

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
CameraPose::CameraPose() : timestamp_(-1) {}

// Public methods.
void CameraPose::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->emplace(XdmConst::CameraPose(), kNamespaceHref);
}

std::unique_ptr<CameraPose>
CameraPose::FromData(const std::vector<double>& position,
                     const std::vector<double>& orientation,
                     const int64 timestamp) {
  if (position.empty() && orientation.empty()) {
    LOG(ERROR) << "Either position or orientation must be provided";
    return nullptr;
  }

  std::unique_ptr<CameraPose> camera_pose(new CameraPose());
  if (position.size() >= 3) {
    camera_pose->position_ = position;
  }

  if (orientation.size() >= 4) {
    camera_pose->orientation_ = NormalizeAxisAngle(orientation);
  }

  if (timestamp >= 0) {
    camera_pose->timestamp_ = timestamp;
  }

  return camera_pose;
}

std::unique_ptr<CameraPose>
CameraPose::FromDeserializer(const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(XdmConst::CameraPose()), XdmConst::CameraPose());
  if (deserializer == nullptr) {
    return nullptr;
  }
  std::unique_ptr<CameraPose> camera_pose(new CameraPose());
  if (!camera_pose->ParseCameraPoseFields(*deserializer)) {
    return nullptr;
  }
  return camera_pose;
}

bool CameraPose::HasPosition() const { return position_.size() == 3; }
bool CameraPose::HasOrientation() const { return orientation_.size() == 4; }

const std::vector<double>&
CameraPose::GetPositionXYZ() const { return position_; }

const std::vector<double>&
CameraPose::GetOrientationRotationXYZAngle() const { return orientation_; }

int64 CameraPose::GetTimestamp() const { return timestamp_; }

bool CameraPose::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }

  if (!HasPosition() && !HasOrientation()) {
    LOG(ERROR) << "Camera pose has neither position nor orientation";
    return false;
  }

  bool success = true;
  if (position_.size() == 3) {
    success &=
        serializer->WriteProperty(XdmConst::CameraPose(), kPositionX,
                                  std::to_string(position_[0])) &&
        serializer->WriteProperty(XdmConst::CameraPose(), kPositionY,
                                  std::to_string(position_[1])) &&
        serializer->WriteProperty(XdmConst::CameraPose(), kPositionZ,
                                  std::to_string(position_[2]));
  }

  if (orientation_.size() == 4) {
    success &=
        serializer->WriteProperty(XdmConst::CameraPose(), kRotationAxisX,
                                  std::to_string(orientation_[0])) &&
        serializer->WriteProperty(XdmConst::CameraPose(), kRotationAxisY,
                                  std::to_string(orientation_[1])) &&
        serializer->WriteProperty(XdmConst::CameraPose(), kRotationAxisZ,
                                  std::to_string(orientation_[2])) &&
        serializer->WriteProperty(XdmConst::CameraPose(), kRotationAngle,
                                  std::to_string(orientation_[3]));
  }

  if (timestamp_ >= 0) {
    serializer->WriteProperty(XdmConst::CameraPose(), kTimestamp,
                              std::to_string(timestamp_));
  }

  return success;
}

// Private methods.
bool CameraPose::ParseCameraPoseFields(const Deserializer& deserializer) {
  double x, y, z;
  const string& prefix = XdmConst::CameraPose();
  // If a position field is present, the rest must be as well.
  if (deserializer.ParseDouble(prefix, kPositionX, &x)) {
    if (!deserializer.ParseDouble(prefix, kPositionY, &y)) {
      return false;
    }
    if (!deserializer.ParseDouble(prefix, kPositionZ, &z)) {
      return false;
    }
    position_ = { x, y, z };
  }

  // Same for orientation.
  if (deserializer.ParseDouble(prefix, kRotationAxisX, &x)) {
    if (!deserializer.ParseDouble(prefix, kRotationAxisY, &y)) {
      return false;
    }
    if (!deserializer.ParseDouble(prefix, kRotationAxisZ, &z)) {
      return false;
    }
    double angle;
    if (!deserializer.ParseDouble(prefix, kRotationAngle, &angle)) {
      return false;
    }
    std::vector<double> axis_angle = { x, y, z, angle };
    orientation_ = NormalizeAxisAngle(axis_angle);
  }

  if (position_.size() < 3 && orientation_.size() < 4) {
    return false;
  }

  deserializer.ParseLong(prefix, kTimestamp, &timestamp_);
  return true;
}

}  // namespace xdm
}  // namespace xmpmeta
