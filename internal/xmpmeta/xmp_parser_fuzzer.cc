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

#include "xmpmeta/gaudio.h"
#include "xmpmeta/gimage.h"
#include "xmpmeta/gpano.h"
#include "xmpmeta/xmp_parser.h"

using xmpmeta::GAudio;
using xmpmeta::GImage;
using xmpmeta::GPano;

namespace xmpmeta {
namespace {

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  XmpData xmp1;
  string in = string(reinterpret_cast<const char*>(data), size);
  if (ReadXmpFromMemory(in, false /* skip_extended */, &xmp1)) {
    // Extract the Pano metadata.
    GPano::FromXmp(xmp1);

    // Extract the Image metadata.
    GImage::FromXmp(xmp1);

    // Extract the Audio metadata.
    GAudio::FromXmp(xmp1);
  }

  XmpData xmp2;
  if (ReadXmpFromMemory(in, true /* skip_extended */, &xmp2)) {
    // Extract the Pano metadata.
    GPano::FromXmp(xmp2);

    // Extract the Image metadata.
    GImage::FromXmp(xmp2);

    // Extract the Audio metadata.
    GAudio::FromXmp(xmp2);
  }

  return 0;
}

}  // namespace
}  // namespace xmpmeta
