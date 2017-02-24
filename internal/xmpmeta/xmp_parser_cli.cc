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
//
////////////////////////////////////////////////////////////////////////////////
//
// Command-line tool for extracting the XMP metadata for a VR photo.
// The PanoMetaData is printed out, and the right panorama and audio files are
// optionally saved to disk.
//
// Example usage:
//  xmp_parser_cli \
//    --input=<input_jpeg_file> \
//    --output_image=<output_jpeg_file> \
//    --output_audio=<output_audio_file> \
//    --alsologtostderr

#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "xmpmeta/file.h"
#include "xmpmeta/gaudio.h"
#include "xmpmeta/gimage.h"
#include "xmpmeta/gpano.h"
#include "xmpmeta/pano_meta_data.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_parser.h"
#include "xmpmeta/xml/utils.h"

DEFINE_string(input, "", "Jpeg file to be parsed.");
DEFINE_string(output_image, "", "Jpeg file for the right eye image.");
DEFINE_string(output_audio, "", "Audio file if present.");

using xmpmeta::PanoMetaData;
using xmpmeta::GAudio;
using xmpmeta::GImage;
using xmpmeta::GPano;
using xmpmeta::ReadXmpHeader;
using xmpmeta::WriteStringToFileOrDie;
using xmpmeta::XmpData;

// Prints the PanoMetaData to the log.
void PrintPanoMetaData(const PanoMetaData& meta_data) {
  LOG(INFO) << "Parsed PanoMetaData:";
  LOG(INFO) << " Cropped left: " << meta_data.cropped_left;
  LOG(INFO) << " Cropped top: " << meta_data.cropped_top;
  LOG(INFO) << " Cropped width: " << meta_data.cropped_width;
  LOG(INFO) << " Cropped height: " << meta_data.cropped_height;
  LOG(INFO) << " Full width: " << meta_data.full_width;
  LOG(INFO) << " Full height: " << meta_data.full_height;
  LOG(INFO) << " Initial heading: " << meta_data.initial_heading_degrees;
}

int main(int argc, char** argv) {
  InitGoogle(argv[0], &argc, &argv, true);
  QCHECK(!FLAGS_input.empty());

  // Parse the XMP.
  XmpData xmp;
  const bool skip_extended =
      FLAGS_output_image.empty() && FLAGS_output_audio.empty();
  QCHECK(ReadXmpHeader(FLAGS_input, skip_extended, &xmp));

  // Print the PanoMetaData.
  auto gpano = CHECK_NOTNULL(GPano::FromXmp(xmp));
  PrintPanoMetaData(gpano->GetPanoMetaData());

  // Optionally decode and save the right image.
  if (!FLAGS_output_image.empty()) {
    auto gimage = CHECK_NOTNULL(GImage::FromXmp(xmp));
    WriteStringToFileOrDie(gimage->GetData(), FLAGS_output_image);
  }

  // Optionally decode and save the audio (if it exists).
  if (!FLAGS_output_audio.empty()) {
    auto gaudio = GAudio::FromXmp(xmp);
    if (gaudio != nullptr) {
      WriteStringToFileOrDie(gaudio->GetData(), FLAGS_output_audio);
    } else {
      LOG(WARNING) << "Pano does not appear to have audio";
    }
  }

  return 0;
}
