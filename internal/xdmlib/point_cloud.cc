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

#include "xdmlib/point_cloud.h"

#include "glog/logging.h"
#include "strings/numbers.h"
#include "xdmlib/const.h"
#include "xmpmeta/base64.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {
namespace {

const char kPropertyPrefix[] = "PointCloud";
const char kCount[] = "Count";
const char kColor[] = "Color";
const char kPosition[] = "Position";
const char kMetric[] = "Metric";
const char kSoftware[] = "Software";

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/pointcloud/";

}  // namespace

// Private constructor.
PointCloud::PointCloud() : count_(-1), metric_(false) { }

// Public methods.
void PointCloud::GetNamespaces(std::unordered_map<string,
                               string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->insert(std::pair<string, string>(kPropertyPrefix,
                                                     kNamespaceHref));
}

std::unique_ptr<PointCloud>
PointCloud::FromData(int count, const string& position, const string& color,
                     bool metric, const string& software) {
  if (position.empty()) {
    LOG(ERROR) << "No position data given";
    return nullptr;
  }
  std::unique_ptr<PointCloud> point_cloud(new PointCloud());
  point_cloud->count_ = count;
  point_cloud->position_ = position;
  point_cloud->metric_ = metric;
  point_cloud->color_ = color;
  point_cloud->software_ = software;
  return point_cloud;
}

std::unique_ptr<PointCloud>
PointCloud::FromDeserializer(const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(kPropertyPrefix), kPropertyPrefix);
  if (deserializer == nullptr) {
    return nullptr;
  }

  std::unique_ptr<PointCloud> point_cloud(new PointCloud());
  if (!point_cloud->ParseFields(*deserializer)) {
    return nullptr;
  }
  return point_cloud;
}

int PointCloud::GetCount() const { return count_; }
const string& PointCloud::GetPosition() const { return position_; }
const string& PointCloud::GetColor() const { return color_; }
bool PointCloud::GetMetric() const { return metric_; }
const string& PointCloud::GetSoftware() const { return software_; }

bool PointCloud::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }

  string base64_encoded_position;
  if (!EncodeBase64(position_, &base64_encoded_position)) {
    LOG(WARNING) << "Position encoding failed";
    return false;
  }

  // Write required fields.
  if (count_ < 0 || !serializer->WriteProperty(XdmConst::PointCloud(), kCount,
                                               SimpleItoa(count_))) {
    return false;
  }
  if (!serializer->WriteProperty(XdmConst::PointCloud(), kPosition,
                                 base64_encoded_position)) {
    return false;
  }

  // Write optional fields.
  serializer->WriteBoolProperty(XdmConst::PointCloud(), kMetric,
                                metric_);

  if (!color_.empty()) {
    string base64_encoded_color;
    if (!EncodeBase64(color_, &base64_encoded_color)) {
      LOG(ERROR) << "Base64 encoding of color failed";
    } else {
      serializer->WriteProperty(XdmConst::PointCloud(), kColor,
                                base64_encoded_color);
    }
  }

  if (!software_.empty()) {
    serializer->WriteProperty(XdmConst::PointCloud(), kSoftware, software_);
  }
  return true;
}

// Private methods.
bool PointCloud::ParseFields(const Deserializer& deserializer) {
  // Required fields.
  if (!deserializer.ParseInt(XdmConst::PointCloud(), kCount, &count_)) {
    return false;
  }
  if (!deserializer.ParseBase64(XdmConst::PointCloud(), kPosition,
                                &position_)) {
    return false;
  }

  // Optional fields.
  if (!deserializer.ParseBoolean(XdmConst::PointCloud(), kMetric, &metric_)) {
    // Set it to the default value.
    metric_ = false;
  }
  deserializer.ParseBase64(XdmConst::PointCloud(), kColor, &color_);
  deserializer.ParseString(XdmConst::PointCloud(), kSoftware, &software_);
  return true;
}

}  // namespace xdm
}  // namespace xmpmeta
