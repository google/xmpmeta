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

#ifndef XMPMETA_XDM_PROFILE_H_
#define XMPMETA_XDM_PROFILE_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xdmlib/element.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

// Implements the Profile element in the XDM specification, with
// serialization and deserialization.
class Profile : public Element {
 public:
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  bool Serialize(xml::Serializer* serializer) const override;

  // Creates a Profile element from the given fields. Returns null if
  // the type is empty, or if the camera_indices are shorter than the
  // specified minimum length for types supported in the XDM specification.
  // Type is case-sensitive. If wrong, this will be treated as an unsupported
  // type.
  static std::unique_ptr<Profile>
      FromData(const string& type, const std::vector<int>& camera_indices);

  // Returns the deserialized Profile, null if parsing fails.
  static std::unique_ptr<Profile>
      FromDeserializer(const xml::Deserializer& parent_deserializer);

  // Returns the Profile type.
  const string& GetType() const;

  // Returns the camera indices.
  const std::vector<int>& GetCameraIndices() const;

  // Disallow copying.
  Profile(const Profile&) = delete;
  void operator=(const Profile&) = delete;

 private:
  Profile(const string& type, const std::vector<int>& camera_indices);

  string type_;
  std::vector<int> camera_indices_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_PROFILE_H_
