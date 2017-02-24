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

#ifndef XMPMETA_GIMAGE_H_
#define XMPMETA_GIMAGE_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {

class GImage {
 public:
  // Appends GImage's XML namespace name and href to the given collection.
  // Key: Name of the namespace.
  // Value: Full namespace URL.
  // Example: ("GImage", "http://ns.google.com/photos/1.0/image/").
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) const;

  // Creates a GImage from the given fields. Returns null if either field is
  // empty. The fields are copied to the new object.
  // Data is NOT base64-encoded, and is the image data of the right eye.
  // Mime is the mimetype of the image data, such as image/jpeg.
  static std::unique_ptr<GImage> CreateFromData(const string& data,
                                                const string& mime);

  // Creates a GImage from pre-extracted XMP metadata. Returns null if parsing
  // fails. Both standard and extended XMP are required.
  static std::unique_ptr<GImage> FromXmp(const XmpData& xmp);

  // Creates a GImage by extracting XMP metadata from a JPEG and parsing it. If
  // using XMP for other things as well, FromXmp() should be used instead to
  // prevent redundant extraction of XMP from the JPEG.
  static std::unique_ptr<GImage> FromJpegFile(const string& filename);

  // Determines whether the requisite fields are present in the XMP metadata.
  // Only the Mime field is checked in order to make this fast. Therefore,
  // extended XMP is not needed.
  static bool IsPresent(const XmpData& xmp);

  // Same as above but extracts XMP metadata from a file instead of reusing
  // already extracted metadata.
  static bool IsPresent(const string& filename);

  // Returns the GImage data, which has been base-64 decoded but is still
  // encoded according to the mime type of the GImage.
  const string& GetData() const;

  // Returns the GImage mime type.
  const string& GetMime() const;

  // Serializes properties to XML.
  // The first serializer is assumed to be for the standard section in XMP, and
  // the second for the extended section.
  bool Serialize(xml::Serializer* std_section_serializer,
                 xml::Serializer* ext_section_serializer) const;

  // Disallow copying.
  GImage(const GImage&) = delete;
  void operator=(const GImage&) = delete;

 private:
  GImage();

  string data_;
  string mime_;
};

}  // namespace xmpmeta

#endif  // XMPMETA_GIMAGE_H_
