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

#ifndef XMPMETA_XDM_MESH_H_
#define XMPMETA_XDM_MESH_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "xdmlib/element.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

// Implements the triangle mesh element from the XDM specification, with
// serialization and deserialization.
namespace xmpmeta {
namespace xdm {

class Mesh : public Element {
 public:
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) override;

  bool Serialize(xml::Serializer* serializer) const override;

  // Creates a Point Cloud from the given fields. Returns null if position is
  // empty. The first two arguments are required fields, the rest are optional.
  // If the color or software fields are empty, they will not be serialized.
  static std::unique_ptr<Mesh> FromData(
      int vertex_count, const std::vector<float>& vertex_position,
      int face_count, const std::vector<int>& face_indicies, bool metric,
      const string& software);

  // Returns the deserialized Mesh; null if parsing fails.
  // The returned pointer is owned by the caller.
  static std::unique_ptr<Mesh> FromDeserializer(
      const xml::Deserializer& parent_deserializer);

  // Getters.
  int GetVertexCount() const;
  const std::vector<float>& GetVertexPosition() const;
  int GetFaceCount() const;
  const std::vector<int>& GetFaceIndices() const;
  bool GetMetric() const;
  const string& GetSoftware() const;

  Mesh(const Mesh&) = delete;
  void operator=(const Mesh&) = delete;

 private:
  Mesh();

  bool ParseFields(const xml::Deserializer& deserializer);

  // Required fields.

  // Number of points (specified by x, y, z triplets) in the data
  int vertex_count_;

  // A list of x, y, z floating-point
  // triples, using the current device's coordinate system: [X1, Y1, Z1,
  // X2, Y2, Z2,...]
  std::vector<float> vertex_position_;

  // Number of triangles.
  int face_count_;

  // A list of triplet of integer indices
  // defining the three vertices of a triangle in the mesh. The order of the
  // three vertices must be counter-clockwise as seen from the front of the
  // face. [I1, J1, K1, I2, J2, K2, ...]. The index values are in
  // [0, VertexCount -1].
  std::vector<int> face_indicies_;

  // Optional fields.

  // Whether the Position values are expressed in meters. If set to false or not
  // set, the units are unknown (i.e., the mesh is defined up to a scale). If
  // this value is not set, then some cases (such as measurement) will not be
  // possible.
  bool metric_;

  // The software that created this mesh
  string software_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_MESH_H_
