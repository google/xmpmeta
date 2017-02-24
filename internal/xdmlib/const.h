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

#ifndef THIRD_PARTY_XMPMETA_INTERNAL_XMPMETA_XDM_CONST_H_
#define THIRD_PARTY_XMPMETA_INTERNAL_XMPMETA_XDM_CONST_H_

#include <map>
#include <string>

namespace xmpmeta {
namespace xdm {

struct XdmConst {
  // XDM element names.
  static const char* Audio();
  static const char* Camera();
  static const char* CameraPose();
  static const char* Device();
  static const char* DevicePose();
  static const char* EquirectModel();
  static const char* Image();
  static const char* PointCloud();
  static const char* Profile();

  // XDM type names (not shared with elements).
  static const char* Cameras();
  static const char* ImagingModel();
  static const char* Profiles();

  // Maps elements to the names of their XML namespaces.
  static const string Namespace(const string& node_name);
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // THIRD_PARTY_XMPMETA_INTERNAL_XMPMETA_XDM_CONST_H_
