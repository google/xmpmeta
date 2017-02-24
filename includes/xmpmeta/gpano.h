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

#ifndef XMPMETA_GPANO_H_
#define XMPMETA_GPANO_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xmpmeta/pano_meta_data.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {

class GPano {
 public:
  // Appends GPano's XML namespace name and href to the given collection.
  // Key: Name of the namespace.
  // Value: Full namespace URL.
  // Example: ("GPano", "http://ns.google.com/photos/1.0/panorama/").
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) const;

  // Creates a GPano from the given PanoMetaData.
  static std::unique_ptr<GPano>
      CreateFromData(const PanoMetaData& pano_meta_data);

  // Creates a GPano from pre-extracted XMP metadata. Returns null if parsing
  // fails. Extended XMP is not needed.
  static std::unique_ptr<GPano> FromXmp(const XmpData& xmp);

  // Creates a GPano by extracting XMP metadata from a JPEG and parsing it. If
  // using XMP for other things as well, FromXmp() should be used instead to
  // prevent redundant extraction of XMP from the JPEG.
  static std::unique_ptr<GPano> FromJpegFile(const string& filename);

  // Returns the GPano data formatted as PanoMetaData.
  const PanoMetaData& GetPanoMetaData() const;

  // Serializes properties to XML.
  // Does not write optional Photo Sphere metadata fields.
  bool Serialize(xml::Serializer* serializer) const;

  // Serializes properties to XML, with the option to control writing
  // optional Photo Sphere metadata fields.
  bool Serialize(xml::Serializer* serializer,
                 bool write_optional_photo_sphere_meta) const;

  // Disallow copying.
  GPano(const GPano&) = delete;
  void operator=(const GPano&) = delete;

 private:
  GPano();

  xmpmeta::PanoMetaData meta_data_;
};

}  // namespace xmpmeta

#endif  // XMPMETA_GPANO_H_
