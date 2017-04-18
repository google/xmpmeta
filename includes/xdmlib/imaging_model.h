// Copyright 2017 The XMPMeta Authors. All Rights Reserved.
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

#ifndef XMPMETA_XDM_IMAGING_MODEL_H_
#define XMPMETA_XDM_IMAGING_MODEL_H_

#include <memory>
#include <unordered_map>

#include "xdmlib/dimension.h"
#include "xdmlib/element.h"
#include "xdmlib/point.h"
#include "xmpmeta/xml/deserializer.h"
#include "xmpmeta/xml/serializer.h"

namespace xmpmeta {
namespace xdm {

// An interface for an imaging model in the XDM spec.
class ImagingModel : public Element {
 public:
  virtual ~ImagingModel() {}

  // Returns the type of the imaging model, e.g. EquirectModel, PerspectiveModel
  // or FisheyeModel.
  virtual const char* GetType() const = 0;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_IMAGING_MODEL_H_
