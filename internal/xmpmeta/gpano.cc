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

#include "xmpmeta/gpano.h"

#include "glog/logging.h"
#include "xmpmeta/xmp_parser.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/serializer.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::PanoMetaData;
using xmpmeta::xml::DeserializerImpl;
using xmpmeta::xml::GetFirstDescriptionElement;
using xmpmeta::xml::ToXmlChar;
using xmpmeta::xml::XmlConst;

namespace xmpmeta {
namespace {

const char kPrefix[] = "GPano";
const char kCroppedAreaLeftPixels[] = "CroppedAreaLeftPixels";
const char kCroppedAreaTopPixels[] = "CroppedAreaTopPixels";
const char kCroppedAreaImageWidthPixels[] = "CroppedAreaImageWidthPixels";
const char kCroppedAreaImageHeightPixels[] = "CroppedAreaImageHeightPixels";
const char kFullPanoWidthPixels[] = "FullPanoWidthPixels";
const char kFullPanoHeightPixels[] = "FullPanoHeightPixels";
const char kInitialViewHeadingDegrees[] = "InitialViewHeadingDegrees";
const char kFullPanoWidthPixelsDeprecated[] = "FullPanoImageWidthPixels";
const char kFullPanoHeightPixelsDeprecated[] = "FullPanoImageHeightPixels";
const char kNamespaceHref[] = "http://ns.google.com/photos/1.0/panorama/";
const char kPoseHeadingDegrees[] = "PoseHeadingDegrees";
const char kProjectionType[] = "ProjectionType";
const char kUsePanoramaViewer[] = "UsePanoramaViewer";

// Extracts metadata from xmp.
bool ParseGPanoFields(const XmpData& xmp, PanoMetaData* meta_data) {
  DeserializerImpl std_deserializer(
      GetFirstDescriptionElement(xmp.StandardSection()));
  if (!std_deserializer.ParseInt(kPrefix, kCroppedAreaLeftPixels,
                                 &meta_data->cropped_left)) {
    return false;
  }
  if (!std_deserializer.ParseInt(kPrefix, kCroppedAreaTopPixels,
                                 &meta_data->cropped_top)) {
    return false;
  }
  if (!std_deserializer.ParseInt(kPrefix, kCroppedAreaImageWidthPixels,
                                 &meta_data->cropped_width)) {
    return false;
  }
  if (!std_deserializer.ParseInt(kPrefix, kCroppedAreaImageHeightPixels,
                                 &meta_data->cropped_height)) {
    return false;
  }
  if (!std_deserializer.ParseInt(kPrefix, kFullPanoWidthPixels,
                                 &meta_data->full_width) &&
      !std_deserializer.ParseInt(kPrefix, kFullPanoWidthPixelsDeprecated,
                                 &meta_data->full_width)) {
    return false;
  }
  if (!std_deserializer.ParseInt(kPrefix, kFullPanoHeightPixels,
                                 &meta_data->full_height) &&
      !std_deserializer.ParseInt(kPrefix, kFullPanoHeightPixelsDeprecated,
                                 &meta_data->full_height)) {
    return false;
  }
  if (!std_deserializer.ParseInt(kPrefix, kInitialViewHeadingDegrees,
                                 &meta_data->initial_heading_degrees)) {
    // If kInitialViewHeadingDegrees is not defined, set it to the center of the
    // cropped panorama.
    meta_data->initial_heading_degrees =
        (meta_data->cropped_left + meta_data->cropped_width / 2) * 360 /
        meta_data->full_width;
  }

  // Optional fields that are helpful for Photo Spheres, but not necessarily
  // helpful for VR Photos.
  std_deserializer.ParseInt(kPrefix, kPoseHeadingDegrees,
                            &meta_data->pose_heading_degrees);
  string projection_type_str;
  if (!std_deserializer.ParseString(kPrefix, kProjectionType,
                                    &projection_type_str)) {
    meta_data->projection_type.FromString(projection_type_str);
  }
  std_deserializer.ParseBoolean(kPrefix, kUsePanoramaViewer,
                                &meta_data->use_panorama_viewer);

  return true;
}

}  // namespace

GPano::GPano() {}

void GPano::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) const {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->emplace(kPrefix, kNamespaceHref);
}

const PanoMetaData& GPano::GetPanoMetaData() const { return meta_data_; }

std::unique_ptr<GPano> GPano::CreateFromData(const PanoMetaData& metadata) {
  std::unique_ptr<GPano> gpano(new GPano());
  gpano->meta_data_ = metadata;
  return gpano;
}

std::unique_ptr<GPano> GPano::FromXmp(const XmpData& xmp) {
  std::unique_ptr<GPano> gpano(new GPano());
  const bool success = ParseGPanoFields(xmp, &gpano->meta_data_);
  return success ? std::move(gpano) : nullptr;
}

std::unique_ptr<GPano> GPano::FromJpegFile(const string& filename) {
  XmpData xmp;
  const bool kSkipExtended = true;
  if (!ReadXmpHeader(filename, kSkipExtended, &xmp)) {
    return nullptr;
  }
  return FromXmp(xmp);
}

bool GPano::Serialize(xml::Serializer* serializer) const {
  return Serialize(serializer, false);
}

bool GPano::Serialize(xml::Serializer* serializer,
                      bool write_optional_photo_sphere_meta) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }

  const string cropped_left_str = std::to_string(meta_data_.cropped_left);
  const string cropped_top_str = std::to_string(meta_data_.cropped_top);
  const string cropped_width_str = std::to_string(meta_data_.cropped_width);
  const string cropped_height_str = std::to_string(meta_data_.cropped_height);
  const string full_width_str = std::to_string(meta_data_.full_width);
  const string full_height_str = std::to_string(meta_data_.full_height);
  const string init_heading_degrees_str =
      std::to_string(meta_data_.initial_heading_degrees);

  // Short-circuiting ensures that serialization halts at the first error if any
  // occurs.
  if (!serializer->WriteProperty(kPrefix, kCroppedAreaLeftPixels,
                                 cropped_left_str) ||
      !serializer->WriteProperty(kPrefix, kCroppedAreaTopPixels,
                                 cropped_top_str) ||
      !serializer->WriteProperty(kPrefix, kCroppedAreaImageWidthPixels,
                                 cropped_width_str) ||
      !serializer->WriteProperty(kPrefix, kCroppedAreaImageHeightPixels,
                                 cropped_height_str) ||
      !serializer->WriteProperty(kPrefix, kFullPanoWidthPixels,
                                 full_width_str) ||
      !serializer->WriteProperty(kPrefix, kFullPanoHeightPixels,
                                 full_height_str) ||
      !serializer->WriteProperty(kPrefix, kInitialViewHeadingDegrees,
                                 init_heading_degrees_str)) {
    return false;
  }

  if (write_optional_photo_sphere_meta) {
    const string pose_heading_degrees_str =
        std::to_string(meta_data_.pose_heading_degrees);
    const string projection_type_str = meta_data_.projection_type.ToString();
    const string use_panorama_viewer_str =
        meta_data_.use_panorama_viewer ? "True" : "False";
    if (!serializer->WriteProperty(kPrefix, kPoseHeadingDegrees,
                                   pose_heading_degrees_str) ||
        !serializer->WriteProperty(kPrefix, kProjectionType,
                                   projection_type_str) ||
        !serializer->WriteProperty(kPrefix, kUsePanoramaViewer,
                                   use_panorama_viewer_str)) {
      return false;
    }
  }

  return true;
}

}  // namespace xmpmeta
