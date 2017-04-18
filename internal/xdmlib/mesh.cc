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

#include "xdmlib/mesh.h"

#include "glog/logging.h"
#include "strings/numbers.h"
#include "xdmlib/const.h"
#include "xmpmeta/base64.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {
namespace {

const char kPropertyPrefix[] = "Mesh";
const char kVertexCount[] = "VertexCount";
const char kVertexPosition[] = "VertexPosition";
const char kFaceCount[] = "FaceCount";
const char kFaceIndices[] = "FaceIndices";
const char kMetric[] = "Metric";
const char kSoftware[] = "Software";

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/mesh/";

}  // namespace

// Private constructor.
Mesh::Mesh() {}

// Public methods.
void Mesh::GetNamespaces(std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->insert(
      std::pair<string, string>(kPropertyPrefix, kNamespaceHref));
}

std::unique_ptr<Mesh> Mesh::FromData(int vertex_count,
                                     const std::vector<float>& vertex_position,
                                     int face_count,
                                     const std::vector<int>& face_indicies,
                                     bool metric, const string& software) {
  if (vertex_count <= 0 || face_count <= 0 ||
      vertex_position.size() != 3 * vertex_count ||
      face_indicies.size() != 3 * face_count) {
    LOG(ERROR) << "Given data is wrong.";
    return nullptr;
  }
  std::unique_ptr<Mesh> mesh(new Mesh());
  mesh->vertex_count_ = vertex_count;
  mesh->vertex_position_ = vertex_position;
  mesh->face_count_ = face_count;
  mesh->face_indicies_ = face_indicies;
  mesh->metric_ = metric;
  mesh->software_ = software;
  return mesh;
}

std::unique_ptr<Mesh> Mesh::FromDeserializer(
    const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(kPropertyPrefix), kPropertyPrefix);
  if (deserializer == nullptr) {
    return nullptr;
  }

  std::unique_ptr<Mesh> mesh(new Mesh());
  if (!mesh->ParseFields(*deserializer)) {
    return nullptr;
  }
  return mesh;
}

int Mesh::GetVertexCount() const { return vertex_count_; }
const std::vector<float>& Mesh::GetVertexPosition() const {
  return vertex_position_;
}
int Mesh::GetFaceCount() const { return face_count_; }
const std::vector<int>& Mesh::GetFaceIndices() const { return face_indicies_; }
bool Mesh::GetMetric() const { return metric_; }
const string& Mesh::GetSoftware() const { return software_; }

bool Mesh::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }

  string base64_encoded_vertex_position;
  if (!EncodeFloatArrayBase64(vertex_position_,
                              &base64_encoded_vertex_position)) {
    LOG(WARNING) << "Vertex position encoding failed";
    return false;
  }

  string base64_encoded_face_indicies;
  if (!EncodeIntArrayBase64(face_indicies_, &base64_encoded_face_indicies)) {
    LOG(WARNING) << "Face indices encoding failed";
    return false;
  }

  // Write required fields.
  if (vertex_count_ < 0 ||
      !serializer->WriteProperty(XdmConst::Mesh(), kVertexCount,
                                 SimpleItoa(vertex_count_))) {
    return false;
  }
  if (!serializer->WriteProperty(XdmConst::Mesh(), kVertexPosition,
                                 base64_encoded_vertex_position)) {
    return false;
  }
  if (face_count_ < 0 ||
      !serializer->WriteProperty(XdmConst::Mesh(), kFaceCount,
                                 SimpleItoa(face_count_))) {
    return false;
  }
  if (!serializer->WriteProperty(XdmConst::Mesh(), kFaceIndices,
                                 base64_encoded_face_indicies)) {
    return false;
  }

  // Write optional fields.
  serializer->WriteBoolProperty(XdmConst::Mesh(), kMetric, metric_);

  if (!software_.empty()) {
    serializer->WriteProperty(XdmConst::Mesh(), kSoftware, software_);
  }
  return true;
}

// Private methods.
bool Mesh::ParseFields(const Deserializer& deserializer) {
  // Required fields.
  if (!deserializer.ParseInt(XdmConst::Mesh(), kVertexCount, &vertex_count_)) {
    return false;
  }
  if (!deserializer.ParseFloatArrayBase64(XdmConst::Mesh(), kVertexPosition,
                                          vertex_position_)) {
    return false;
  }
  if (!deserializer.ParseInt(XdmConst::Mesh(), kFaceCount, &face_count_)) {
    return false;
  }
  if (!deserializer.ParseIntArrayBase64(XdmConst::Mesh(), kFaceIndices,
                                        face_indicies_)) {
    return false;
  }

  // Optional fields.
  if (!deserializer.ParseBoolean(XdmConst::Mesh(), kMetric, &metric_)) {
    // Set it to the default value.
    metric_ = false;
  }
  deserializer.ParseString(XdmConst::Mesh(), kSoftware, &software_);
  return true;
}

}  // namespace xdm
}  // namespace xmpmeta
