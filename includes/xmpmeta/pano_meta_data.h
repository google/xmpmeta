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

#ifndef XMPMETA_PANO_META_DATA_H_
#define XMPMETA_PANO_META_DATA_H_

#include <string>

namespace xmpmeta {

struct ProjectionType {
  enum Type { EQUIRECTANGULAR } type_ = EQUIRECTANGULAR;

  std::string ToString() const {
    if (type_ == EQUIRECTANGULAR) {
      return "equirectangular";
    } else {
      return "unknown";
    }
  }

  bool FromString(const std::string& str) {
    if (str == "equirectangular") {
      type_ = EQUIRECTANGULAR;
      return true;
    }
    return false;
  }
};

// A struct the contains all the meta data needed to describe the geometry
// of a panorama. The fields in the struct are in one-to-one correspondence
// with https://developers.google.com/streetview/spherical-metadata
struct PanoMetaData {
  // The dimensions of the full mosaic image that the result image is
  // cropped from.
  // Note that these dimensions currently need to have a 2:1 aspect ratio
  // as the result is rendered as an equi-rectangular panorama.
  int full_width = 0;
  int full_height = 0;

  // The bounds of the result image within the full equi-rectangular 2:1 pano.
  int cropped_width = 0;
  int cropped_height = 0;
  int cropped_left = 0;
  int cropped_top = 0;

  // The initial heading when looking at the panorama, e.g. the center of the
  // cropped region, in degrees.
  int initial_heading_degrees = 0;

  // Compass heading, measured in degrees, for the center the image. Value must
  // be >= 0 and < 360.
  int pose_heading_degrees = 0;

  // Projection type used in the image file. Google products currently support
  // the value "equirectangular".
  ProjectionType projection_type;

  // Whether to show this image in a photo sphere viewer rather than as a normal
  // flat image.
  bool use_panorama_viewer = true;

  // Returns a PanoMetaData that is a copy of this one, except with pixel
  // coordinates multiplied by the given scale factor.
  PanoMetaData Scale(double s) const;

  // Returns a PanoMetaData that is scaled to have the given cropped dimensions.
  PanoMetaData Scale(int new_cropped_width, int new_cropped_height) const;
};

inline PanoMetaData PanoMetaData::Scale(double s) const {
  PanoMetaData scaled = *this;
  scaled.full_width = static_cast<int>(s * full_width);
  scaled.full_height = static_cast<int>(s * full_height);
  scaled.cropped_width = static_cast<int>(s * cropped_width);
  scaled.cropped_height = static_cast<int>(s * cropped_height);
  scaled.cropped_left = static_cast<int>(s * cropped_left);
  scaled.cropped_top = static_cast<int>(s * cropped_top);
  return scaled;
}

inline PanoMetaData PanoMetaData::Scale(int new_cropped_width,
                                        int new_cropped_height) const {
  PanoMetaData scaled = *this;

  scaled.full_width = full_width * new_cropped_width / cropped_width;
  scaled.full_height = full_height * new_cropped_height / cropped_height;
  scaled.cropped_left = cropped_left * new_cropped_width / cropped_width;
  scaled.cropped_top = cropped_top * new_cropped_height / cropped_height;
  scaled.cropped_width = new_cropped_width;
  scaled.cropped_height = new_cropped_height;
  return scaled;
}

}  // namespace xmpmeta

#endif  // XMPMETA_PANO_META_DATA_H_
