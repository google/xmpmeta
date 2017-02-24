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

#ifndef XMPMETA_XMP_CONST_H_
#define XMPMETA_XMP_CONST_H_

namespace xmpmeta {

// Constants used in writing XMP metadata.
struct XmpConst {
  // XMP namespaces.
  static const char* Namespace();
  static const char* NamespacePrefix();
  static const char* NodeName();
  static const char* AdobePropName();
  static const char* AdobePropValue();
  static const char* NoteNamespace();

  // XMP headers.
  static const char* Header();
  static const char* ExtensionHeader();
  static const char* HasExtensionPrefix();
  static const char* HasExtension();

  // Sizes.
  static const int ExtensionHeaderOffset();
  static const int MaxBufferSize();
  static const int ExtendedMaxBufferSize();
};

}  // namespace xmpmeta

#endif  // XMPMETA_XMP_CONST_H_
