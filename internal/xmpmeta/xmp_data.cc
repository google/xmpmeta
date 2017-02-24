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

#include "xmpmeta/xmp_data.h"

namespace xmpmeta {

XmpData::XmpData() : xmp_(nullptr), xmp_extended_(nullptr) {}

XmpData::~XmpData() { Reset(); }

void XmpData::Reset() {
  if (xmp_) {
    xmlFreeDoc(xmp_);
    xmp_ = nullptr;
  }
  if (xmp_extended_) {
    xmlFreeDoc(xmp_extended_);
    xmp_extended_ = nullptr;
  }
}

const xmlDocPtr XmpData::StandardSection() const { return xmp_; }

xmlDocPtr* XmpData::MutableStandardSection() { return &xmp_; }

const xmlDocPtr XmpData::ExtendedSection() const { return xmp_extended_; }

xmlDocPtr* XmpData::MutableExtendedSection() { return &xmp_extended_; }

}  // namespace xmpmeta
