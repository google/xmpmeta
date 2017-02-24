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

#include "xmpmeta/xmp_const.h"

namespace xmpmeta {

// XMP namespace constants.
const char* XmpConst::Namespace() {
  return "adobe:ns:meta/";
}

const char* XmpConst::NamespacePrefix() {
  return "x";
}

const char* XmpConst::NodeName() {
  return "xmpmeta";
}

const char* XmpConst::AdobePropName() {
  return "xmptk";
}

const char* XmpConst::AdobePropValue() {
  return "Adobe XMP";
}

const char* XmpConst::NoteNamespace() {
  return "http://ns.adobe.com/xmp/note/";
}

// XMP headers.
const char* XmpConst::Header() {
  return "http://ns.adobe.com/xap/1.0/";
}

const char* XmpConst::ExtensionHeader() {
  return "http://ns.adobe.com/xmp/extension/";
}

const char* XmpConst::HasExtensionPrefix() {
  return "xmpNote";
}

const char* XmpConst::HasExtension() {
  return "HasExtendedXMP";
}

// Sizes.
const int XmpConst::ExtensionHeaderOffset() {
  return 8;
}

const int XmpConst::MaxBufferSize() {
  return 65502;
}

const int XmpConst::ExtendedMaxBufferSize() {
  return 65458;
}

}  // namespace xmpmeta
