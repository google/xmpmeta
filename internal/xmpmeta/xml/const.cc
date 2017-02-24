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

#include "xmpmeta/xml/const.h"

namespace xmpmeta {
namespace xml {

const char* XmlConst::EncodingStr() {
  return "UTF-8";
}

// RDF metadata constants.
const char* XmlConst::RdfAbout() {
  return "about";
}

const char* XmlConst::RdfDescription() {
  return "Description";
}

const char* XmlConst::RdfNodeName() {
  return "RDF";
}

const char* XmlConst::RdfNodeNs() {
  return "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
}

const char* XmlConst::RdfPrefix() {
  return "rdf";
}

const char* XmlConst::RdfSeq() {
  return "Seq";
}

const char* XmlConst::RdfLi() {
  return "li";
}

// XML metadata constants.
const char* XmlConst::NsAttrName() {
  return "xmlns";
}

const char* XmlConst::Separator() {
  return ":";
}

const char* XmlConst::Version() {
  return "1.0";
}

}  // namespace xml
}  // namespace xmpmeta
