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

#include "xdmlib/navigational_connectivity.h"

#include "glog/logging.h"
#include "xdmlib/const.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {
namespace {

const char kNavigationalEdges[] = "NavigationalEdges";

const char kNamespaceHref[] =
    "http://ns.xdm.org/photos/1.0/navigationalconnectivity/";

// Returns false if the size of the navigational edges list is odd or if any
// edge in the list is negative. The list length cannot be odd because it
// consists of (i, j) pairs and each pair indicates an edge from image i to
// image j.
bool ValidateNavigationalEdges(const std::vector<int>& navigational_edges) {
  if (navigational_edges.size() % 2 != 0) {
    return false;
  }
  for (const int edge : navigational_edges) {
    if (edge < 0) {
      return false;
    }
  }
  return true;
}

}  // namespace

// Private constructor.
NavigationalConnectivity::NavigationalConnectivity(
    const std::vector<int>& navigational_edges)
    : navigational_edges_(navigational_edges) {}

// Public methods.
void NavigationalConnectivity::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->emplace(XdmConst::NavigationalConnectivity(),
                            kNamespaceHref);
}

std::unique_ptr<NavigationalConnectivity> NavigationalConnectivity::FromData(
    const std::vector<int>& navigational_edges) {
  if (navigational_edges.empty()) {
    LOG(ERROR) << "NavigationalConnectivity must have navigational edges";
    return nullptr;
  }

  // Check that the navigational edges list have even number of edges and each
  // edge is greater than or equal to 0.
  if (!ValidateNavigationalEdges(navigational_edges)) {
    return nullptr;
  }

  return std::unique_ptr<NavigationalConnectivity>(
      new NavigationalConnectivity(navigational_edges));
}

std::unique_ptr<NavigationalConnectivity>
NavigationalConnectivity::FromDeserializer(
    const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(XdmConst::NavigationalConnectivity()),
          XdmConst::NavigationalConnectivity());
  if (deserializer == nullptr) {
    return nullptr;
  }
  std::unique_ptr<NavigationalConnectivity> navigational_connectivity(
      new NavigationalConnectivity({}));
  if (!deserializer->ParseIntArray(
          XdmConst::NavigationalConnectivity(), kNavigationalEdges,
          &navigational_connectivity->navigational_edges_)) {
    return nullptr;
  }

  // Check that the navigational edges list have an even number of edges and
  // each edge is greater than or equal to 0.
  if (!ValidateNavigationalEdges(
          navigational_connectivity->navigational_edges_)) {
    return nullptr;
  }
  return navigational_connectivity;
}

const std::vector<int>& NavigationalConnectivity::GetNavigationalEdges() const {
  return navigational_edges_;
}

bool NavigationalConnectivity::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }
  if (navigational_edges_.empty()) {
    LOG(ERROR) << "Navigational edges list is empty";
    return false;
  }
  return serializer->WriteIntArray(XdmConst::NavigationalConnectivity(),
                                   kNavigationalEdges, navigational_edges_);
}

}  // namespace xdm
}  // namespace xmpmeta
