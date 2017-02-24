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

#ifndef XMPMETA_XDM_PROFILES_H_
#define XMPMETA_XDM_PROFILES_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xdmlib/profile.h"
#include "xdmlib/element.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

// Implements the Device:Profiles field from the XDM specification, with
// serialization and deserialization for its child Profile elements.
class Profiles : public Element {
 public:
  // Interface methods.
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  bool Serialize(xml::Serializer* serializer) const override;

  // Static methods.

  // Creates this object from the given profiles. If the list is empty, returns
  // a unique_ptr owning nothing.
  static std::unique_ptr<Profiles>
      FromProfileArray(std::vector<std::unique_ptr<Profile>>* profiles);

  // Returns the deserialized profiles in a Profiles object, a unique_ptr owning
  // nothing if parsing failed for all the profiles.
  static std::unique_ptr<Profiles>
      FromDeserializer(const xml::Deserializer& parent_deserializer);

  // Non-static methods.

  // Returns the list of cameras.
  const std::vector<const Profile*> GetProfiles() const;

  // Disallow copying
  Profiles(const Profiles&) = delete;
  void operator=(const Profiles&) = delete;

 private:
  Profiles() = default;

  std::vector<std::unique_ptr<Profile>> profile_list_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_PROFILES_H_
