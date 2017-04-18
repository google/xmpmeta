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

#include "xdmlib/vendor_info.h"

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

const char kPropertyPrefix[] = "VendorInfo";
const char kModel[] = "Model";
const char kManufacturer[] = "Manufacturer";
const char kNotes[] = "Notes";

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/vendorinfo/";

}  // namespace

// Private constructor.
VendorInfo::VendorInfo() : manufacturer_(""), model_(""), notes_("") {}

// Public methods.
void VendorInfo::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->insert(
      std::pair<string, string>(kPropertyPrefix, kNamespaceHref));
}

std::unique_ptr<VendorInfo> VendorInfo::FromData(const string& manufacturer,
                                                 const string& model,
                                                 const string& notes) {
  if (manufacturer.empty()) {
    LOG(ERROR) << "No manufacturer data given";
    return nullptr;
  }
  std::unique_ptr<VendorInfo> vendor_info(new VendorInfo());
  vendor_info->model_ = model;
  vendor_info->manufacturer_ = manufacturer;
  vendor_info->notes_ = notes;
  return vendor_info;
}

std::unique_ptr<VendorInfo> VendorInfo::FromDeserializer(
    const Deserializer& parent_deserializer, const string& namespace_str) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(namespace_str, kPropertyPrefix);
  if (deserializer == nullptr) {
    return nullptr;
  }

  std::unique_ptr<VendorInfo> vendor_info(new VendorInfo());
  if (!vendor_info->ParseFields(*deserializer)) {
    return nullptr;
  }
  return vendor_info;
}

const string& VendorInfo::GetManufacturer() const { return manufacturer_; }
const string& VendorInfo::GetModel() const { return model_; }
const string& VendorInfo::GetNotes() const { return notes_; }

bool VendorInfo::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }

  // Write required field.
  if (!serializer->WriteProperty(XdmConst::VendorInfo(), kManufacturer,
                                 manufacturer_)) {
    return false;
  }

  // Write optional fields.
  if (!model_.empty()) {
    serializer->WriteProperty(XdmConst::VendorInfo(), kModel, model_);
  }
  if (!notes_.empty()) {
    serializer->WriteProperty(XdmConst::VendorInfo(), kNotes, notes_);
  }
  return true;
}

// Private methods.
bool VendorInfo::ParseFields(const Deserializer& deserializer) {
  // Required field.
  if (!deserializer.ParseString(XdmConst::VendorInfo(), kManufacturer,
                                &manufacturer_)) {
    return false;
  }

  // Optional fields.
  deserializer.ParseString(XdmConst::VendorInfo(), kModel, &model_);
  deserializer.ParseString(XdmConst::VendorInfo(), kNotes, &notes_);
  return true;
}

}  // namespace xdm
}  // namespace xmpmeta
