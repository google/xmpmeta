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

#ifndef XMPMETA_XDM_EQUIRECT_MODEL_H_
#define XMPMETA_XDM_EQUIRECT_MODEL_H_

#include <memory>
#include <unordered_map>

#include "xdmlib/dimension.h"
#include "xdmlib/element.h"
#include "xdmlib/point.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

// Implements the EquirectModel element in the XDM specification, with
// serialization and deserialization.
class EquirectModel : public Element {
 public:
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  bool Serialize(xml::Serializer* serializer) const override;

  // Creates an EquirectModel from the given fields. Returns null if
  // any of the required fields are not present (see fields below).
  // The order of numbers in cropped_origin is (left, top).
  // The order of numbers in cropped_size is (cropped width, cropped height).
  // The order of numbers in full_size = (full width, full height).
  static std::unique_ptr<EquirectModel>  FromData(const Point& cropped_origin,
                                                  const Dimension& cropped_size,
                                                  const Dimension& full_size);

  // Returns the deserialized equirect model, null if parsing fails.
  static std::unique_ptr<EquirectModel>
      FromDeserializer(const xml::Deserializer& parent_deserializer);

  // Getters.
  const Point& GetCroppedOrigin() const;
  const Dimension& GetCroppedSize() const;
  const Dimension& GetFullSize() const;

  // Disallow copying.
  EquirectModel(const EquirectModel&) = delete;
  void operator=(const EquirectModel&) = delete;

 private:
  EquirectModel(const Point& cropped_origin,
                const Dimension& cropped_size,
                const Dimension& full_size);

  // Cropped origin coordinates.
  // cropped_origin_.first: CroppedLeftAreaPixels
  // cropped_origin_.second: CroppedTopAreaPixels
  Point cropped_origin_;

  // Cropped size, in pixels.
  // cropped_size_.width: CroppedAreaImageWidthPixels
  // cropped_size_.height: CroppedAreaImageHeightPixels
  Dimension cropped_size_;

  // Full size, in pixels.
  // full_size_.width: FullImageAreaImageWidthPixels
  // full_size_.height: FullImageAreaImageHeightPixels
  Dimension full_size_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_EQUIRECT_MODEL_H_
