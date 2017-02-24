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

#ifndef XMPMETA_XDM_POINT_CLOUD_H_
#define XMPMETA_XDM_POINT_CLOUD_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xdmlib/element.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

// Implements the Point Cloud element from the XDM specification, with
// serialization and deserialization.
namespace xmpmeta {
namespace xdm {

class PointCloud : public Element {
 public:
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  bool Serialize(xml::Serializer* serializer) const override;

  // Creates a Point Cloud from the given fields. Returns null if position is
  // empty. The first two arguments are required fields, the rest are optional.
  // If the color or software fields are empty, they will not be serialized.
  static std::unique_ptr<PointCloud>
      FromData(int count, const string& position, const string& color,
               bool metric, const string& software);

  // Returns the deserialized PointCloud; null if parsing fails.
  // The returned pointer is owned by the caller.
  static std::unique_ptr<PointCloud>
      FromDeserializer(const xml::Deserializer& parent_deserializer);

  // Getters.
  int GetCount() const;
  const string& GetPosition() const;  // Raw data, i.e. not base64 encoded.
  const string& GetColor() const;
  bool GetMetric() const;
  const string& GetSoftware() const;

  PointCloud(const PointCloud&) = delete;
  void operator=(const PointCloud&) = delete;

 private:
  PointCloud();

  bool ParseFields(const xml::Deserializer& deserializer);

  // Required fields.
  int count_;
  string position_;  // Raw data, i.e. not base64 encoded.

  // Optional fields.
  bool metric_;
  string color_;  // Raw data, i.e. not base64 encoded.
  string software_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_POINT_CLOUD_H_
