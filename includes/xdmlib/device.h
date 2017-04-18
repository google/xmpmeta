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

#ifndef XMPMETA_XDM_DEVICE_H_
#define XMPMETA_XDM_DEVICE_H_

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "xdmlib/cameras.h"
#include "xdmlib/device_pose.h"
#include "xdmlib/mesh.h"
#include "xdmlib/navigational_connectivity.h"
#include "xdmlib/profiles.h"
#include "xdmlib/vendor_info.h"
#include "xmpmeta/xmp_data.h"

namespace xmpmeta {
namespace xdm {

// Implements a Device from the XDM specification, with serialization and
// deserialization.
// Does not implement the Element interface because Device is at the top level
// in the XDM tree.
class Device {
 public:
  // Creates this object from the given XDM elements.
  static std::unique_ptr<Device> FromData(
      const string& revision, std::unique_ptr<DevicePose> device_pose,
      std::unique_ptr<Profiles> profiles, std::unique_ptr<Cameras> cameras,
      std::unique_ptr<VendorInfo> vendor_info, std::unique_ptr<Mesh> mesh,
      std::unique_ptr<NavigationalConnectivity> navigational_connectivity);

  // Creates a Device from pre-extracted XMP metadata. Returns null if
  // parsing fails. Both the standard and extended XMP sections are required.
  static std::unique_ptr<Device> FromXmp(const XmpData& xmp);

  // Creates a Device by extracting XMP metadata from a JPEG and parsing it.
  // If using XMP for other things as well, FromXmp() should be used instead to
  // prevent redundant extraction of XMP from the JPEG.
  static std::unique_ptr<Device> FromJpegFile(const string& filename);

  // Creates a Device by parsing XML file containing the metadata.
  static std::unique_ptr<Device> FromXmlFile(const string& filename);

  // Getters.
  const string& GetRevision() const;
  const Cameras* GetCameras() const;
  const DevicePose* GetDevicePose() const;
  const Profiles* GetProfiles() const;
  const VendorInfo* GetVendorInfo() const;
  const Mesh* GetMesh() const;
  const NavigationalConnectivity* GetNavigationalConnectivity() const;

  // Not const for XML memory management reasons. More info in source comments.
  bool SerializeToXmp(XmpData* xmp);

  // Saves Device metadata to a .xml file.
  bool SerializeToXmlFile(const char* filename);

  // Disallow copying.
  Device(const Device&) = delete;
  void operator=(const Device&) = delete;

 private:
  Device();

  // Retrieves the namespaces of all child elements.
  void GetNamespaces(
      std::unordered_map<string, string>* ns_name_href_map) const;
  // Gathers all the XML namespaces of child elements.
  void PopulateNamespaces();
  // Parses Device fields and XDM children elements from XmpData.
  bool ParseFields(const XmpData& xmp);
  // Parses Device fields and XDM children elements from xmlDocPtr.
  bool ParseFields(const xmlDocPtr& xmp);
  bool Serialize(xmlDocPtr* xmlDoc);

  // Keep a reference to the XML namespaces, so that they are created only once
  // when Device is constructed.
  std::unordered_map<string, xmlNsPtr> namespaces_;

  // XDM fields and elements.
  string revision_;
  std::unique_ptr<Profiles> profiles_;
  std::unique_ptr<Cameras> cameras_;

  // Optional fields.
  std::unique_ptr<VendorInfo> vendor_info_;

  // The following fields are relevant to PosedCollection profile only.
  std::unique_ptr<DevicePose> device_pose_;
  std::unique_ptr<Mesh> mesh_;
  std::unique_ptr<NavigationalConnectivity> navigational_connectivity_;
};

}  // namespace xdm
}  // namespace xmpmeta

#endif  // XMPMETA_XDM_DEVICE_H_
