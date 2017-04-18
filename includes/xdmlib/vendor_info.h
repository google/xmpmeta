// Copyright 2017 The XMPMeta Authors. All Rights Reserved.
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

#ifndef XMPMETA_XDM_VENDOR_INFO_H_
#define XMPMETA_XDM_VENDOR_INFO_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xdmlib/element.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

/**
 * A VendorInfo element for an XDM device.
 */
class VendorInfo : public Element {
 public:
  // Appends child elements' namespaces' and their respective hrefs to the
  // given collection, and any parent nodes' names to prefix_names.
  // Key: Name of the namespace.
  // Value: Full namespace URL.
  // Example: ("VendorInfo", "http://ns.xdm.org/photos/1.0/vendorinfo/")
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  // Serializes this object.
  bool Serialize(xml::Serializer* serializer) const override;

  // Creates an VendorInfo from the given fields. Returns null if
  // any of the required fields is empty (see fields below).
  // Manufacturer is the manufacturer of the element that created the content.
  // Model is the model of the element that created the content.
  // Notes is general comments.
  static std::unique_ptr<VendorInfo> FromData(const string& manufacturer,
                                              const string& model,
                                              const string& notes);

  // Returns the deserialized VendorInfo; null if parsing fails.
  static std::unique_ptr<VendorInfo> FromDeserializer(
      const xml::Deserializer& parent_deserializer,
      const string& namespace_str);

  // Returns the Manufacturer.
  const string& GetManufacturer() const;

  // Returns the Model.
  const string& GetModel() const;

  // Returns the Notes.
  const string& GetNotes() const;

  // Disallow copying.
  VendorInfo(const VendorInfo&) = delete;
  void operator=(const VendorInfo&) = delete;

 private:
  VendorInfo();

  bool ParseFields(const xml::Deserializer& deserializer);

  // Required field.
  string manufacturer_;  // The manufacturer.

  // Optional fields.
  string model_;  // The model.
  string notes_;  // The notes.
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_VENDOR_INFO_H_
