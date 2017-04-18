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
// Command-line tool for writing the XMP metadata for a VR photo.
//
// Example usage (all files in --data_dir):
//  xmp_writer_cli \
//    --data_dir /path/to/some/directory
//    --left_eye left.jpg \
//    --right_eye right.jpg \
//    --audio audio.mp4 \
//    --output output.jpg

#include <memory>
#include <string>

#include "file/base/file.h"
#include "file/base/helpers.h"
#include "file/base/path.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "image/wimage/wimage.h"
#include "image/wimage/wimage_file_io.h"
#include "strings/stringpiece.h"
#include "strings/stringpiece_utils.h"
#include "xmpmeta/gaudio.h"
#include "xmpmeta/gimage.h"
#include "xmpmeta/gpano.h"
#include "xmpmeta/pano_meta_data.h"
#include "xmpmeta/vr_photo_writer.h"
#include "xmpmeta/xml/serializer_impl.h"
#include "xmpmeta/xml/utils.h"
#include "xmpmeta/xmp_const.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_writer.h"

// Required flags.
DEFINE_string(data_dir, "", "Directory of input and output data");
DEFINE_string(left_eye, "", "Left eye image file in --data_dir");
DEFINE_string(right_eye, "", "Right eye image file in --data_dir");
DEFINE_string(output, "", "Name of output file; overwrites file if it exists");

// Optional PanoMetaData flags.
// If unspecified, these will default to legal (but perhaps incorrect) values.
DEFINE_string(audio, "", "Audio file (to embed) in --data_dir");
DEFINE_double(cropped_left, 0, "PanoMetaData: cropped left");
DEFINE_double(cropped_top, 0, "PanoMetaData: cropped top");
DEFINE_double(cropped_width, 0, "PanoMetaData: cropped width");
DEFINE_double(cropped_height, 0, "PanoMetaData: cropped height");
DEFINE_double(full_width, 0, "PanoMetaData: full width");
DEFINE_double(full_height, 0, "PanoMetaData: full height");
DEFINE_int32(initial_heading, 0, "PanoMetaData: initial heading degrees");

// Optional mimetype flags.
DEFINE_string(right_image_mime, "image/jpeg", "Right eye image mimetype");
DEFINE_string(audio_mime, "audio/mp4", "Audio mimetype");

using xmpmeta::CreateXmpData;
using xmpmeta::GAudio;
using xmpmeta::GImage;
using xmpmeta::GPano;
using xmpmeta::PanoMetaData;
using xmpmeta::WriteLeftEyeAndXmpMeta;
using xmpmeta::WriteVrPhotoMetaToXmp;
using xmpmeta::XmpData;

// Initializes the PanoMetaData to the flags' values, or default ones if
// the flags are not specified.
void InitializePanoMetaData(const string& input_name,
                            PanoMetaData* meta_data) {
  WImageBuffer4_b image;
  StringPiece extension = file::Extension(input_name);
  if (strings::StartsWith(extension, "p") ||
      strings::StartsWith(extension, "P")) {
    CHECK(WImageFileIO::ReadImageFromPNG(input_name, &image)) << input_name;
  } else {
    CHECK(WImageFileIO::ReadImageFromJPEG(input_name, &image)) << input_name;
  }

  // Initialize with default values if the flags were not provided.
  meta_data->full_width =
      FLAGS_full_width ? FLAGS_full_width : image.Width();
  meta_data->full_height =
      FLAGS_full_height ? FLAGS_full_height : 0.5f * meta_data->full_width;
  meta_data->cropped_left = FLAGS_cropped_left;
  meta_data->cropped_top =
      FLAGS_cropped_top ? FLAGS_cropped_top
      : 0.5f * (meta_data->full_height - image.Height());
  meta_data->cropped_width =
      FLAGS_cropped_width ? FLAGS_cropped_width : image.Width();
  meta_data->cropped_height =
      FLAGS_cropped_height ? FLAGS_cropped_height : image.Height();
  meta_data->initial_heading_degrees = FLAGS_initial_heading;

  QCHECK_GE(meta_data->full_width, 1);
  QCHECK_GE(meta_data->full_height, 1);
  QCHECK_GE(meta_data->cropped_width, 1);
  QCHECK_GE(meta_data->cropped_height, 1);
  QCHECK_GE(meta_data->cropped_left, 0);
  QCHECK_GE(meta_data->cropped_top, 0);
  QCHECK_LE(meta_data->cropped_left + meta_data->cropped_width,
            meta_data->full_width);
  QCHECK_LE(meta_data->cropped_top + meta_data->cropped_height,
            meta_data->full_height);
}

int main(int argc, char** argv) {
  InitGoogle(argv[0], &argc, &argv, true);

  QCHECK(!FLAGS_data_dir.empty());
  QCHECK(!FLAGS_left_eye.empty());
  QCHECK(!FLAGS_right_eye.empty());
  QCHECK(!FLAGS_output.empty());

  const string left_filename = file::JoinPath(FLAGS_data_dir, FLAGS_left_eye);
  const string right_filename = file::JoinPath(FLAGS_data_dir, FLAGS_right_eye);
  const string output_filename = file::JoinPath(FLAGS_data_dir, FLAGS_output);

  // TODO(miraleung): Add timings.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  PanoMetaData metadata;
  InitializePanoMetaData(left_filename, &metadata);

  string original_right_data;
  QCHECK_OK(file::GetContents(right_filename, &original_right_data,
                              file::Defaults()));
  string original_left_data;
  QCHECK_OK(file::GetContents(left_filename, &original_left_data,
                              file::Defaults()));

  std::unique_ptr<GPano> gpano = GPano::CreateFromData(metadata);
  std::unique_ptr<GAudio> gaudio;

  if (!FLAGS_audio.empty()) {
    const string audio_filename = file::JoinPath(FLAGS_data_dir, FLAGS_audio);
    string original_audio_data;
    QCHECK_OK(file::GetContents(audio_filename, &original_audio_data,
                                file::Defaults()));
    gaudio = GAudio::CreateFromData(original_audio_data, FLAGS_audio_mime);
  }
  std::unique_ptr<GImage> gimage =
      GImage::CreateFromData(original_right_data, FLAGS_right_image_mime);

  if (!WriteVrPhotoMetaToXmp(*gimage, *gpano, gaudio.get(), xmp_data.get())) {
    LOG(ERROR) << "Could not serialize GPano metadata to XMP";
    return 1;
  }

  if (!WriteLeftEyeAndXmpMeta(original_left_data, output_filename, *xmp_data)) {
    LOG(ERROR) << "Could not write XmpData to " << output_filename;
  }

  return 0;
}
