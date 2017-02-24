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

#include "xmpmeta/photo_sphere_writer.h"

#include <unordered_map>

#include <libxml/tree.h>

#include "base/port.h"
#include "glog/logging.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::xml::GetFirstDescriptionElement;
using xmpmeta::xml::Serializer;
using xmpmeta::xml::SerializerImpl;
using xmpmeta::xml::ToXmlChar;
using xmpmeta::xml::XmlConst;

namespace xmpmeta {
namespace {

// Creates and populates a namespace map from a name, href mapping,
// with its name and href.
void PopulateNamespaceMap(
    const std::unordered_map<string, string>& ns_name_href_map,
    std::unordered_map<string, xmlNsPtr>* dest_map) {
  for (const auto& entry : ns_name_href_map) {
    if (dest_map->count(entry.first) == 0 && !entry.second.empty()) {
      dest_map->emplace(entry.first,
                        xmlNewNs(nullptr, ToXmlChar(entry.second.data()),
                                 ToXmlChar(entry.first.data())));
    }
  }
}

}  // namespace

bool WritePhotoSphereMetaToXmp(const GPano& gpano, XmpData* xmp_data) {
  std::unordered_map<string, string> ns_name_href_map;

  // GPano field will be written to the standard section.
  gpano.GetNamespaces(&ns_name_href_map);
  std::unordered_map<string, xmlNsPtr> main_namespaces;
  PopulateNamespaceMap(ns_name_href_map, &main_namespaces);

  std::unique_ptr<SerializerImpl> main_serializer =
      SerializerImpl::FromDataAndSerializeNamespaces(
          main_namespaces,
          GetFirstDescriptionElement(*xmp_data->MutableStandardSection()));
  if (!main_serializer) {
    return false;
  }

  if (!gpano.Serialize(main_serializer.get(),
                       true /* write_optional_photo_sphere_meta */)) {
    LOG(ERROR) << "Could not serialize GPano to XmpData";
    return false;
  }

  return true;
}

}  // namespace xmpmeta
