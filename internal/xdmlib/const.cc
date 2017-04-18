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

#include "third_party/xmpmeta/internal/xdmlib/const.h"

#include "base/port.h"

namespace xmpmeta {
namespace xdm {
namespace {

// Element names.
const char kAudio[] = "Audio";
const char kCamera[] = "Camera";
const char kCameraPose[] = "CameraPose";
const char kDevice[] = "Device";
const char kDevicePose[] = "DevicePose";
const char kEquirectModel[] = "EquirectModel";
const char kImage[] = "Image";
const char kMesh[] = "Mesh";
const char kNavigationalConnectivity[] = "NavigationalConnectivity";
const char kPointCloud[] = "PointCloud";
const char kProfile[] = "Profile";
const char kVendorInfo[] = "VendorInfo";

// Type names.
const char kCameras[] = "Cameras";
const char kImagingModel[] = "ImagingModel";
const char kProfiles[] = "Profiles";

}  // namespace

// XDM element names.
const char* XdmConst::Audio() { return kAudio; }

const char* XdmConst::Camera() { return kCamera; }

const char* XdmConst::CameraPose() { return kCameraPose; }

const char* XdmConst::Device() { return kDevice; }

const char* XdmConst::DevicePose() { return kDevicePose; }

const char* XdmConst::EquirectModel() { return kEquirectModel; }

const char* XdmConst::Image() { return kImage; }

const char* XdmConst::Mesh() { return kMesh; }

const char* XdmConst::NavigationalConnectivity() {
  return kNavigationalConnectivity;
}

const char* XdmConst::PointCloud() { return kPointCloud; }

const char* XdmConst::Profile() { return kProfile; }

const char* XdmConst::VendorInfo() { return kVendorInfo; }

// XDM type names.
const char* XdmConst::Cameras() { return kCameras; }

const char* XdmConst::ImagingModel() { return kImagingModel; }

const char* XdmConst::Profiles() { return kProfiles; }

// Returns the namespace to which the given XDM element or type belongs.
const string XdmConst::Namespace(const string& node_name) {
  // Elements.
  if (node_name == kAudio || node_name == kCameraPose ||
      node_name == kEquirectModel || node_name == kImage ||
      node_name == kPointCloud) {
    return kCamera;
  }
  if (node_name == kCamera || node_name == kDevicePose ||
      node_name == kProfile || node_name == kMesh ||
      node_name == kNavigationalConnectivity) {
    return kDevice;
  }

  // Types.
  if (node_name == kImagingModel) {
    return kCamera;
  }
  if (node_name == kCameras || node_name == kProfiles) {
    return kDevice;
  }
  return "";
}

}  // namespace xdm
}  // namespace xmpmeta
