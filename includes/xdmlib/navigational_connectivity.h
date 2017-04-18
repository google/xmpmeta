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

#ifndef XMPMETA_XDM_NAVIGATIONAL_CONNECTIVITY_H_
#define XMPMETA_XDM_NAVIGATIONAL_CONNECTIVITY_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xdmlib/element.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

// Implements the NavigationalConnectivity element in the XDM specification,
// with serialization and deserialization.
class NavigationalConnectivity : public Element {
 public:
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  bool Serialize(xml::Serializer* serializer) const override;

  // Creates a NavigationalConnectivity element from the given fields. Returns
  // nullptr if navigational_edges list is invalid, e.g. it has an odd number of
  // edges or any edge is negative. The list length cannot be odd because it
  // consists of (i, j) pairs and each pair indicates an edge from image i to
  // image j.
  static std::unique_ptr<NavigationalConnectivity> FromData(
      const std::vector<int>& navigational_edges);

  // Returns the deserialized NavigationalConnectivity. Returns nullptr if
  // parsing fails or if the parsed data is invalid.
  static std::unique_ptr<NavigationalConnectivity> FromDeserializer(
      const xml::Deserializer& parent_deserializer);

  // Returns the navigational edges.
  const std::vector<int>& GetNavigationalEdges() const;

  // Disallow copying.
  NavigationalConnectivity(const NavigationalConnectivity&) = delete;
  void operator=(const NavigationalConnectivity&) = delete;

 private:
  NavigationalConnectivity(const std::vector<int>& navigational_edges);

  // Directed graph representing the pairs of camera between which a viewer
  // should allow transitions. [I1, J1, I2, J2, ...]
  std::vector<int> navigational_edges_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_NAVIGATIONAL_CONNECTIVITY_H_
