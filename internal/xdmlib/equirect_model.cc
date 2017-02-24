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

#include "xdmlib/equirect_model.h"

#include "glog/logging.h"
#include "xdmlib/const.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {
namespace {

const char kPropertyPrefix[] = "EquirectModel";
const char kNodePrefix[] = "ImagingModel";
const char kCroppedLeft[] = "CroppedAreaLeftPixels";
const char kCroppedTop[] = "CroppedAreaTopPixels";
const char kCroppedImageWidth[] = "CroppedAreaImageWidthPixels";
const char kCroppedImageHeight[] = "CroppedAreaImageHeightPixels";
const char kFullImageWidth[] = "FullImageWidthPixels";
const char kFullImageHeight[] = "FullImageHeightPixels";

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/equirectmodel/";
const char kNodeNamespaceHref[] = "http://ns.xdm.org/photos/1.0/imagingmodel/";

std::unique_ptr<EquirectModel> ParseFields(const Deserializer& deserializer) {
  int cropped_left;
  int cropped_top;
  int cropped_image_width;
  int cropped_image_height;
  int full_image_width;
  int full_image_height;
  const string& prefix = XdmConst::EquirectModel();
  // Short-circuiting ensures unnecessary reads will not be performed.
  if (!deserializer.ParseInt(prefix, kCroppedLeft, &cropped_left) ||
      !deserializer.ParseInt(prefix, kCroppedTop, &cropped_top) ||
      !deserializer.ParseInt(prefix, kCroppedImageWidth,
                             &cropped_image_width) ||
      !deserializer.ParseInt(prefix, kCroppedImageHeight,
                             &cropped_image_height) ||
      !deserializer.ParseInt(prefix, kFullImageWidth, &full_image_width) ||
      !deserializer.ParseInt(prefix, kFullImageHeight, &full_image_height)) {
    return nullptr;
  }
  return EquirectModel::FromData(
      Point(cropped_left, cropped_top),
      Dimension(cropped_image_width, cropped_image_height),
      Dimension(full_image_width, full_image_height));
}

}  // namespace

// Private constructor.
EquirectModel::EquirectModel(const Point& cropped_origin,
                             const Dimension& cropped_size,
                             const Dimension& full_size) :
  cropped_origin_(cropped_origin), cropped_size_(cropped_size),
  full_size_(full_size) {}

// Public methods.
void EquirectModel::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->emplace(kNodePrefix, kNodeNamespaceHref);
  ns_name_href_map->emplace(kPropertyPrefix, kNamespaceHref);
}

std::unique_ptr<EquirectModel>
EquirectModel::FromData(const Point& cropped_origin,
                        const Dimension& cropped_size,
                        const Dimension& full_size) {
  return std::unique_ptr<EquirectModel>(
      new EquirectModel(cropped_origin, cropped_size, full_size));
}

std::unique_ptr<EquirectModel>
EquirectModel::FromDeserializer(const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(kPropertyPrefix), kPropertyPrefix);
  if (deserializer == nullptr) {
    return nullptr;
  }
  return ParseFields(*deserializer);
}

const Point&
EquirectModel::GetCroppedOrigin() const { return cropped_origin_; }

const Dimension&
EquirectModel::GetCroppedSize() const { return cropped_size_; }

const Dimension&
EquirectModel::GetFullSize() const { return full_size_; }

bool EquirectModel::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }

  // Short-circuiting ensures unnecessary writes will not be performed.
  if (!serializer->WriteProperty(XdmConst::EquirectModel(), kCroppedLeft,
                                 std::to_string(cropped_origin_.x)) ||
      !serializer->WriteProperty(XdmConst::EquirectModel(), kCroppedTop,
                                 std::to_string(cropped_origin_.y)) ||
      !serializer->WriteProperty(XdmConst::EquirectModel(), kCroppedImageWidth,
                                 std::to_string(cropped_size_.width)) ||
      !serializer->WriteProperty(XdmConst::EquirectModel(), kCroppedImageHeight,
                                 std::to_string(cropped_size_.height)) ||
      !serializer->WriteProperty(XdmConst::EquirectModel(), kFullImageWidth,
                                 std::to_string(full_size_.width)) ||
      !serializer->WriteProperty(XdmConst::EquirectModel(), kFullImageHeight,
                                 std::to_string(full_size_.height))) {
    return false;
  }
  return true;
}

}  // namespace xdm
}  // namespace xmpmeta
