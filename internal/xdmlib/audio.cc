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

#include "xdmlib/audio.h"

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
const char kMimeMp4[] = "audio/mp4";

const char kNamespaceHref[] = "http://ns.xdm.org/photos/1.0/audio/";

}  // namespace

// Private constructor.
Audio::Audio() {}

// Public methods.
void Audio::GetNamespaces(
    std::unordered_map<string, string>* ns_name_href_map) {
  if (ns_name_href_map == nullptr) {
    LOG(ERROR) << "Namespace list or own namespace is null";
    return;
  }
  ns_name_href_map->emplace(XdmConst::Audio(), kNamespaceHref);
}

// Public methods.
std::unique_ptr<Audio> Audio::FromData(const string& data, const string& mime) {
  if (data.empty() || mime.empty()) {
    LOG(ERROR) << "No audio data or mimetype given";
    return nullptr;
  }
  std::unique_ptr<Audio> audio(new Audio());
  audio->data_ = data;
  audio->mime_ = mime;
  return audio;
}

std::unique_ptr<Audio>
Audio::FromDeserializer(const Deserializer& parent_deserializer) {
  std::unique_ptr<Deserializer> deserializer =
      parent_deserializer.CreateDeserializer(
          XdmConst::Namespace(XdmConst::Audio()), XdmConst::Audio());
  if (deserializer == nullptr) {
    return nullptr;
  }
  std::unique_ptr<Audio> audio(new Audio());
  if (!audio->ParseAudioFields(*deserializer)) {
    return nullptr;
  }
  return audio;
}

const string& Audio::GetData() const { return data_; }

const string& Audio::GetMime() const { return mime_; }

bool Audio::Serialize(Serializer* serializer) const {
  if (serializer == nullptr) {
    LOG(ERROR) << "Serializer is null";
    return false;
  }
  string base64_encoded;
  if (!EncodeBase64(data_, &base64_encoded)) {
    return false;
  }
  if (!serializer->WriteProperty(XdmConst::Audio(), kMime, mime_)) {
    return false;
  }
  return serializer->WriteProperty(XdmConst::Audio(), kData, base64_encoded);
}

// Private methods.
bool Audio::ParseAudioFields(const Deserializer& deserializer) {
  if (!deserializer.ParseString(XdmConst::Audio(), kMime, &mime_)) {
    return false;
  }
  return deserializer.ParseBase64(XdmConst::Audio(), kData , &data_);
}

}  // namespace xdm
}  // namespace xmpmeta
