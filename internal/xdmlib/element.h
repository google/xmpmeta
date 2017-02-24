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

#ifndef XMPMETA_XDM_ELEMENT_H_
#define XMPMETA_XDM_ELEMENT_H_

#include <unordered_map>

#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

/**
 * An interface for an element in the XDM spec.
 */
class Element {
 public:
  virtual ~Element() {}

  // Appends child elements' namespaces' and their respective hrefs to the
  // given collection, and any parent nodes' names to prefix_names.
  // Key: Name of the namespace.
  // Value: Full namespace URL.
  // Example: ("Audio", "http://ns.xdm.org/photos/1.0/audio/")
  virtual void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) = 0;

  // Serializes this element.
  virtual bool Serialize(xml::Serializer* serializer) const = 0;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_ELEMENT_H_
