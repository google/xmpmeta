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

#include "xmpmeta/photo_sphere_writer.h"

#include <memory>
#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "xmpmeta/file.h"
#include "xmpmeta/pano_meta_data.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/xmp_writer.h"
#include "xmpmeta/xml/utils.h"

namespace xmpmeta {
namespace {

const char kStandardSectionData[] = "photo_sphere_std_section_data.txt";

TEST(PhotoSphereWriter, Write) {
  PanoMetaData meta;
  meta.full_width = 10;
  meta.full_height = 20;
  meta.cropped_width = 5;
  meta.cropped_height = 10;
  meta.cropped_left = 0;
  meta.cropped_top = 3;
  std::unique_ptr<GPano> gpano = GPano::CreateFromData(meta);

  // Write the XMP.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(false);
  ASSERT_EQ(nullptr, xmp_data->ExtendedSection());
  ASSERT_TRUE(WritePhotoSphereMetaToXmp(*gpano, xmp_data.get()));

  // Check that the XML document matches our expectations.
  const string std_section_data_path =
      TestFileAbsolutePath(kStandardSectionData);
  std::string expected_data;
  ReadFileToStringOrDie(std_section_data_path, &expected_data);
  EXPECT_EQ(expected_data, xml::XmlDocToString(xmp_data->StandardSection()));
}

}  // namespace
}  // namespace xmpmeta
