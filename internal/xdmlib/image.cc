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

#include "xdmlib/image.h"

#include "glog/logging.h"
#include "xdmlib/const.h"
#include "xmpmeta/base64.h"

using xmpmeta::xml::Deserializer;
using xmpmeta::xml::Serializer;

namespace xmpmeta {
namespace xdm {
namespace {

const char kMime[] = "Mime";
const char kData[] = "Data";
const char kImageId[] = "ImageId";

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/image/";

}  // namespace

// Private constructor.
Image::Image() {}

// Public methods.
void Image::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->emplace(XdmConst::Image(), kNamespaceHref);
}

std::unique_ptr<Image> Image::FromData(const string& data, const string& mime,
                                       const string& image_id) {
  if ((data.empty() && image_id.empty()) || mime.empty()) {
    LOG(ERROR) << "No image data/id or mimetype given";
    return nullptr;
  }
  if (!data.empty() && !image_id.empty()) {
    LOG(ERROR) << "Either image data or id is needed, not both";
    return nullptr;
  }
  std::unique_ptr<Image> image(new Image());
  image->data_ = data;
  image->mime_ = mime;
  image->image_id_ = image_id;
  return image;
}

std::unique_ptr<Image> Image::FromDeserializer(
    const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(XdmConst::Image()), XdmConst::Image());
  if (deserializer == nullptr) {
    return nullptr;
  }
  std::unique_ptr<Image> image(new Image());
  if (!image->ParseImageFields(*deserializer)) {
    return nullptr;
  }
  return image;
}

const string& Image::GetData() const { return data_; }

const string& Image::GetMime() const { return mime_; }

const string& Image::GetImageId() const { return image_id_; }

bool Image::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }

  if (!serializer->WriteProperty(XdmConst::Image(), kMime, mime_)) {
    return false;
  }
  if (!data_.empty()) {
    string base64_encoded;
    if (!EncodeBase64(data_, &base64_encoded)) {
      return false;
    }
    return serializer->WriteProperty(XdmConst::Image(), kData, base64_encoded);
  }
  if (!image_id_.empty()) {
    return serializer->WriteProperty(XdmConst::Image(), kImageId, image_id_);
  }
  return false;
}

// Private methods.
bool Image::ParseImageFields(const Deserializer& deserializer) {
  if (!deserializer.ParseString(XdmConst::Image(), kMime, &mime_)) {
    return false;
  }

  return (deserializer.ParseString(XdmConst::Image(), kImageId, &image_id_) ||
          deserializer.ParseBase64(XdmConst::Image(), kData, &data_));
}

}  // namespace xdm
}  // namespace xmpmeta
