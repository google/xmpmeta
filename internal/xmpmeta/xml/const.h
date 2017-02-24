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

#ifndef XMPMETA_XML_CONST_H_
#define XMPMETA_XML_CONST_H_

namespace xmpmeta {
namespace xml {

struct XmlConst {
  // Encoding type.
  static const char* EncodingStr();

  // RDF metadata.
  static const char* RdfAbout();
  static const char* RdfDescription();
  static const char* RdfNodeName();
  static const char* RdfNodeNs();
  static const char* RdfPrefix();
  static const char* RdfSeq();
  static const char* RdfLi();

  // XML metadata.
  static const char* NsAttrName();
  static const char* Separator();
  static const char* Version();
};

}  // namespace xml
}  // namespace xmpmeta

#endif  // XMPMETA_XML_CONST_H_
