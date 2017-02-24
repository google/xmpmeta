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

#include "xmpmeta/xmp_writer.h"

#include <memory>
#include <string>
#include <vector>

#include <libxml/tree.h>

#include "base/port.h"
#include "xmpmeta/file.h"
#include "xmpmeta/test_util.h"
#include "xmpmeta/test_xmp_creator.h"
#include "xmpmeta/xmp_const.h"
#include "xmpmeta/xmp_parser.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/deserializer_impl.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::xml::DeserializerImpl;
using xmpmeta::xml::GetFirstDescriptionElement;
using xmpmeta::xml::FromXmlChar;
using xmpmeta::xml::ToXmlChar;
using xmpmeta::xml::XmlConst;

namespace xmpmeta {
namespace {

// TODO(miraleung): Add check for buffer length and position in header of
// extended XMP, here and in parser.

const char kDataName[] = "Data";
const char kDataValue[] = "9865";
const char kInFile[] = "in.jpg";
const char kMimeName[] = "Mime";
const char kMimeValue[] = "image/jpeg";
const char kOutFile[] = "out.jpg";
const char kPrefix[] = "GImage";
const char kData[] = "Data";
const char kStandardSectionTestDataPath[] =
    "xmp_writer_standard_section_data.txt";
const char kExtendedSectionTestDataPath[] =
    "xmp_writer_extended_section_data.txt";

// XMP standard test data.
const char kXmpBody[] =
    "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP\">\n"
    "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
    "    <rdf:Description rdf:about=\"\"\n"
    "      xmlns:GImage=\"http://ns.google.com/photos/1.0/image/\"\n"
    "      xmlns:xmpNote=\"http://ns.adobe.com/xmp/note/\"\n"
    "      GImage:Mime=\"image/jpeg\"\n"
    "      xmpNote:HasExtendedXMP=\"c5247828a5155031a73ef9faeeb31391\"/>\n"
    "  </rdf:RDF>\n"
    "</x:xmpmeta>\n";

// XMP extension test data.
const char kXmpExtensionHeaderPart2[] =
    "c5247828a5155031a73ef9faeeb31391xxxxxxxx";
const char kXmpExtensionBody[] =
    "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP\">\n"
    "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
    "    <rdf:Description rdf:about=\"\"\n"
    "      xmlns:GImage=\"http://ns.google.com/photos/1.0/image/\"\n"
    "      GImage:Data=\"9865\"/>\n"
    "  </rdf:RDF>\n"
    "</x:xmpmeta>\n";

// XDM test data.
const char kXdmBody[] =
    "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP\">\n"
    "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
    "    <rdf:Description rdf:about=\"\"\n"
    "      xmlns:GImage=\"http://ns.google.com/photos/1.0/image/\"\n"
    "      xmlns:xmpNote=\"http://ns.adobe.com/xmp/note/\"\n"
    "      GImage:Mime=\"image/jpeg\"\n"
    "      xmpNote:HasExtendedXMP=\"a1a13d8dc9a1e69ad632d16906eb02ae\"/>\n"
    "  </rdf:RDF>\n"
    "</x:xmpmeta>\n";

const char kXdmExtensionHeaderPart2[] =
    "a1a13d8dc9a1e69ad632d16906eb02aexxxxxxxx";
const char kXdmExtensionBody[] =
    "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP\">\n"
    "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
    "    xmlns:Device=\"http://ns.xdm.org/photos/1.0/device/\"\n"
    "    xmlns:Camera=\"http://ns.xdm.org/photos/1.0/camera/\"\n"
    "    xmlns:Image=\"http://ns.xdm.org/photos/1.0/image/\"\n"
    "    xmlns:Audio=\"http://ns.xdm.org/photos/1.0/audio/\"\n>"
    "    <rdf:Description rdf:about=\"\"\n"
    "     Device:Revision=\"1.0\"\n"
    "     Device:ContainerSignature=\"signature\">\n"
    "     <Device:Cameras>\n"
    "       <rdf:Seq>\n"
    "         <rdf:li>\n"
    "           <Camera:Image\n"
    "              Image:Mime=\"image/png\"\n"
    "              Image:Data=\"imageData1111\" />\n"
    "           <Camera:Audio>\n"
    "              <Audio:Mime>audio/mp3</Audio:Mime>\n"
    "              <Audio:Data>audioData1111</Audio:Data>\n"
    "           </Camera:Audio>\n"
    "        </rdf:li>\n"
    "         <rdf:li>\n"
    "           <Camera:Image>"
    "              <Image:Mime>image/jpeg</Image:Mime>\n"
    "              <Image:Data>imageData2222</Image:Data>\n"
    "           </Camera:Image>\n"
    "           <Camera:Audio>\n"
    "              <Audio:Mime>audio/mp4</Audio:Mime>\n"
    "              <Audio:Data>audioData2222</Audio:Data>\n"
    "           </Camera:Audio>\n"
    "        </rdf:li>\n"
    "         <rdf:li>\n"
    "           <Camera:Image>"
    "              <Image:Mime>image/tiff</Image:Mime>\n"
    "              <Image:Data>imageData3333</Image:Data>\n"
    "           </Camera:Image>\n"
    "           <Camera:Audio>\n"
    "              <Audio:Mime>audio/wav</Audio:Mime>\n"
    "              <Audio:Data>audioData3333</Audio:Data>\n"
    "           </Camera:Audio>\n"
    "        </rdf:li>\n"
    "      </rdf:Seq>\n"
    "     </Device:Cameras>\n"
    "   </rdf:Description>\n"
    " </rdf:RDF>\n"
    "</x:xmpmeta>\n";

// XMP generic test metadata.
const int kXmpStandardBoilerplateSize = 190;  // No namespaces or properties.
// Standard section w/o namespaces or properties, and empty extended section.
const int kXmpStandardBoilerplateSizeWithExtendedSection = 236;
const int kXmpExtendedBoilerplateSize = 188;
const char kLargePropertyName[] = "Data";

const string MockPayload(int size) {
  return string("S") + string(size - 2, '.') + "E";
}

void ClearAndSetProperty(XmpData* xmp_data, bool set_property_on_extended,
                         const char* property_name,
                         const string& property_value) {
  xmlNodePtr description_node =
      GetFirstDescriptionElement(
          set_property_on_extended
          ? *xmp_data->MutableExtendedSection()
          : *xmp_data->MutableStandardSection());
  ASSERT_TRUE(description_node != nullptr);
  xmlSetProp(description_node, ToXmlChar(property_name),
             ToXmlChar(property_value.c_str()));
}

// xmp_string includes both the standard and extended sections.
int NumExtendedSections(const string& xmp_string) {
  int num_extended_sections = 0;
  int position = 0;
  while ((position = xmp_string.find(XmpConst::ExtensionHeader(), position))
         != string::npos) {
    num_extended_sections++;
    position += strlen(XmpConst::ExtensionHeader());
  }
  return num_extended_sections;
}

// Checks the root node.
void CheckXmpRootNode(const xmlDocPtr xmp_section) {
  ASSERT_TRUE(xmp_section != nullptr);

  // Check the number of root nodes..
  int num_elements = 0;
  for (xmlNodePtr child = xmp_section->children; child != nullptr;
       child = child->next) {
    num_elements++;
  }
  ASSERT_EQ(1, num_elements);
  xmlNodePtr xmpmeta_node = xmp_section->children;
  ASSERT_TRUE(xmpmeta_node != nullptr);
  ASSERT_EQ(string(XmpConst::NodeName()),
            string(FromXmlChar(xmpmeta_node->name)));

  // Check root node's properties.
  int num_properties = 0;
  for (xmlAttrPtr attr = xmpmeta_node->properties; attr != nullptr;
      attr = attr->next) {
    num_properties++;
  }
  ASSERT_EQ(1, num_properties);
  const xmlAttrPtr xmpmeta_node_prop = xmpmeta_node->properties;
  ASSERT_EQ(string(XmpConst::AdobePropName()),
            string(FromXmlChar(xmpmeta_node_prop->name)));

  ASSERT_NE(nullptr,
            xmlHasProp(xmpmeta_node, ToXmlChar(XmpConst::AdobePropName())));

  // Check root node's namespace.
  const xmlNsPtr xmpmeta_ns = xmpmeta_node->ns;
  ASSERT_EQ(string(XmpConst::NamespacePrefix()),
            string(FromXmlChar(xmpmeta_ns->prefix)));
  ASSERT_EQ(string(XmpConst::Namespace()),
            string(FromXmlChar(xmpmeta_ns->href)));

  // Check number of children of root node.
  num_elements = 0;
  for (xmlNodePtr child = xmpmeta_node->children; child != nullptr;
       child = child->next) {
    num_elements++;
  }
  ASSERT_EQ(1, num_elements);
}

// Checks the rdf::RDF node.
void CheckXmpRdfNode(const xmlDocPtr xmp_section) {
  ASSERT_TRUE(xmp_section != nullptr);
  xmlNodePtr xmpmeta_node = xmp_section->children;
  ASSERT_TRUE(xmpmeta_node != nullptr);
  xmlNodePtr rdf_node = xmpmeta_node->children;
  ASSERT_EQ(string(XmlConst::RdfNodeName()),
            string(FromXmlChar(rdf_node->name)));

  int num_properties = 0;
  for (xmlAttrPtr attr = rdf_node->properties; attr != nullptr;
      attr = attr->next) {
    num_properties++;
  }
  ASSERT_EQ(0, num_properties);

  // Check rdf:RDF node's namespace.
  const xmlNsPtr rdf_ns = rdf_node->ns;
  ASSERT_EQ(string(XmlConst::RdfPrefix()), string(FromXmlChar(rdf_ns->prefix)));
  ASSERT_EQ(string(XmlConst::RdfNodeNs()), string(FromXmlChar(rdf_ns->href)));

  // Check number of children of the rdf:RDF node.
  int num_elements = 0;
  for (xmlNodePtr child = rdf_node->children; child != nullptr;
       child = child->next) {
    num_elements++;
  }
  ASSERT_EQ(1, num_elements);
}

// Checks the rdf:Description node.
void CheckXmpDescriptionNode(const xmlDocPtr xmp_section) {
  ASSERT_TRUE(xmp_section != nullptr);
  xmlNodePtr xmpmeta_node = xmp_section->children;
  ASSERT_TRUE(xmpmeta_node != nullptr);
  xmlNodePtr rdf_node = xmpmeta_node->children;
  ASSERT_TRUE(rdf_node != nullptr);
  xmlNodePtr description_node = rdf_node->children;
  ASSERT_EQ(string(XmlConst::RdfDescription()),
            string(FromXmlChar(description_node->name)));

  int num_properties = 0;
  xmlAttrPtr attr;
  for (attr = description_node->properties; attr != nullptr;
      attr = attr->next) {
    num_properties++;
  }
  // Only the rdf:about property should be present.
  ASSERT_EQ(1, num_properties);
  attr = description_node->properties;
  ASSERT_NE(nullptr, attr);
  ASSERT_EQ(XmlConst::RdfAbout(), string(FromXmlChar(attr->name)));
  ASSERT_NE(nullptr, attr->ns);
  ASSERT_NE(nullptr, attr->ns->prefix);
  ASSERT_EQ(XmlConst::RdfPrefix(), string(FromXmlChar(attr->ns->prefix)));

  // rdf:Description has a namespace.
  ASSERT_NE(nullptr, description_node->ns);
  ASSERT_EQ(string(XmlConst::RdfPrefix()),
            FromXmlChar(description_node->ns->prefix));

  // New description node is findable.
  ASSERT_EQ(description_node,
            GetFirstDescriptionElement(xmp_section));

  // Check number of children of description node.
  int num_elements = 0;
  for (xmlNodePtr child = description_node->children; child != nullptr;
       child = child->next) {
    num_elements++;
  }
  ASSERT_EQ(0, num_elements);
}

// Checks the nodes and properties of the XMP section.
void CheckXmpSection(const xmlDocPtr xmp_section) {
  CheckXmpRootNode(xmp_section);
  CheckXmpRdfNode(xmp_section);
  CheckXmpDescriptionNode(xmp_section);
}

TEST(XmpWriter, CreateXmpDataStandardSectionOnly) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(false);
  ASSERT_EQ(nullptr, xmp_data->ExtendedSection());
  CheckXmpSection(xmp_data->StandardSection());
}

TEST(XmpWriter, CreateXmpDataWithExtenededSection) {
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  CheckXmpSection(xmp_data->StandardSection());
  CheckXmpSection(xmp_data->ExtendedSection());
}

TEST(XmpWriter, WriteValidStandardXmp) {
  // Initialize XmpData with some data.
  const string in_filename = TempFileAbsolutePath(kInFile);
  std::vector<string> standard_xmp;
  standard_xmp.push_back(TestXmpCreator::CreateStandardXmpString(kXmpBody));
  TestXmpCreator::WriteJPEGFile(in_filename, standard_xmp);
  XmpData xmp_data;
  ASSERT_TRUE(ReadXmpHeader(in_filename, true, &xmp_data));

  // Check that it was initialized with the expected data.
  string value;
  DeserializerImpl deserializer(
      GetFirstDescriptionElement(xmp_data.StandardSection()));
  ASSERT_TRUE(deserializer.ParseString(kPrefix, kMimeName, &value));
  ASSERT_EQ(string(kMimeValue), value);

  // Write the XMP.
  const string out_filename = TempFileAbsolutePath(kOutFile);
  std::vector<string> empty_sections;
  TestXmpCreator::WriteJPEGFile(out_filename, empty_sections);
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, xmp_data));

  // Read the XMP back in.
  XmpData new_xmp_data;
  ASSERT_TRUE(ReadXmpHeader(out_filename, true, &new_xmp_data));
  DeserializerImpl new_deserializer(
      GetFirstDescriptionElement(xmp_data.StandardSection()));
  ASSERT_TRUE(new_deserializer.ParseString(kPrefix, kMimeName, &value));
  ASSERT_EQ(string(kMimeValue), value);
  ASSERT_TRUE(deserializer.ParseString("xmpNote", "HasExtendedXMP", &value));
  ASSERT_EQ(string("c5247828a5155031a73ef9faeeb31391"), value);
}

TEST(XmpWriter, WriteExtendedXmp) {
  // Initialize XmpData with some data.
  const string filename = TempFileAbsolutePath(kInFile);
  std::vector<string> xmp_sections =
      TestXmpCreator::CreateExtensionXmpStrings(1, kXmpExtensionHeaderPart2,
                                                kXmpExtensionBody);
  xmp_sections.insert(xmp_sections.begin(),
                      TestXmpCreator::CreateStandardXmpString(kXmpBody));
  TestXmpCreator::WriteJPEGFile(filename, xmp_sections);
  XmpData xmp_data;
  ASSERT_TRUE(ReadXmpHeader(filename, false, &xmp_data));

  // Check that it was initialized with the expected data.
  string value;
  DeserializerImpl deserializer(
      GetFirstDescriptionElement(xmp_data.ExtendedSection()));
  ASSERT_TRUE(deserializer.ParseString(kPrefix, kDataName, &value));
  ASSERT_EQ(string(kDataValue), value);

  // Write the XMP.
  const string out_filename = TempFileAbsolutePath(kOutFile);
  std::vector<string> empty_sections;
  TestXmpCreator::WriteJPEGFile(out_filename, empty_sections);
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, xmp_data));

  // Read the XMP back in.
  XmpData new_xmp_data;
  ASSERT_TRUE(ReadXmpHeader(out_filename, false, &new_xmp_data));
  DeserializerImpl new_deserializer(
      GetFirstDescriptionElement(xmp_data.ExtendedSection()));
  ASSERT_TRUE(new_deserializer.ParseString(kPrefix, kDataName, &value));
  ASSERT_EQ(string(kDataValue), value);
  const string std_section_data_path =
      TestFileAbsolutePath(kStandardSectionTestDataPath);
  std::string expected_data;
  ReadFileToStringOrDie(std_section_data_path, &expected_data);
  EXPECT_EQ(expected_data, xml::XmlDocToString(new_xmp_data.StandardSection()));
  const string ext_section_data_path =
      TestFileAbsolutePath(kExtendedSectionTestDataPath);
  ReadFileToStringOrDie(ext_section_data_path, &expected_data);
  EXPECT_EQ(expected_data, xml::XmlDocToString(new_xmp_data.ExtendedSection()));
}

TEST(XmpWriter, WriteExtendedXmpMultipleSections) {
  const string xdm_revision("1.0");
  const string device_signature("signature");
  const char device_prefix[] = "Device";
  const char revision_name[] = "Revision";
  const char container_signature_name[] = "ContainerSignature";

  // Initialize XmpData with some data.
  const string filename = TempFileAbsolutePath(kInFile);
  std::vector<string> xmp_sections =
      TestXmpCreator::CreateExtensionXmpStrings(10,
                                                kXdmExtensionHeaderPart2,
                                                kXdmExtensionBody);
  xmp_sections.insert(xmp_sections.begin(),
                      TestXmpCreator::CreateStandardXmpString(kXdmBody));
  TestXmpCreator::WriteJPEGFile(filename, xmp_sections);
  XmpData xmp_data;
  ASSERT_TRUE(ReadXmpHeader(filename, false, &xmp_data));

  // Check that it was initialized with the expected data.
  string value;
  DeserializerImpl deserializer(
      GetFirstDescriptionElement(xmp_data.ExtendedSection()));
  ASSERT_TRUE(deserializer.ParseString(device_prefix, revision_name, &value));
  ASSERT_EQ(string(xdm_revision), value);
  ASSERT_TRUE(deserializer.ParseString(device_prefix, container_signature_name,
                                       &value));
  ASSERT_EQ(device_signature, value);

  // Write the XMP.
  const string out_filename = TempFileAbsolutePath(kOutFile);
  std::vector<string> empty_sections;
  TestXmpCreator::WriteJPEGFile(out_filename, empty_sections);
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, xmp_data));

  // Read the XMP back in.
  XmpData new_xmp_data;
  ASSERT_TRUE(ReadXmpHeader(out_filename, false, &new_xmp_data));
  DeserializerImpl new_deserializer(
      GetFirstDescriptionElement(new_xmp_data.ExtendedSection()));
  ASSERT_TRUE(new_deserializer.ParseString(device_prefix, revision_name,
                                           &value));
  ASSERT_EQ(xdm_revision, value);
  ASSERT_TRUE(new_deserializer.ParseString(device_prefix,
                                           container_signature_name, &value));
  ASSERT_EQ(device_signature, value);
}

TEST(XmpWriter, WriteNullStandardXmp) {
  // Write the XMP.
  XmpData xmp_data;
  const string out_filename = TempFileAbsolutePath(kOutFile);
  std::vector<string> empty_sections;
  ASSERT_FALSE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                      out_filename, xmp_data));
}

TEST(XmpWriter, WriteToNewFile) {
    const string in_filename = TempFileAbsolutePath(kInFile);
std::vector<string> standard_xmp;
  standard_xmp.push_back(TestXmpCreator::CreateStandardXmpString(kXmpBody));
  TestXmpCreator::WriteJPEGFile(in_filename, standard_xmp);
  XmpData xmp_data;
  // Read the XMP so that we know the failure originates from the non-existent
  // file.
  ASSERT_TRUE(ReadXmpHeader(in_filename, true, &xmp_data));
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     "new_file.jpg", xmp_data));
}

TEST(XmpWriter, WriteXmpStandardXmpLimit) {
  const string out_filename = TempFileAbsolutePath(kOutFile);
  int xmp_max_payload_size =  XmpConst::MaxBufferSize()
      - kXmpStandardBoilerplateSize
      - strlen(kLargePropertyName) - 2;  // For extra delimiter chars.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(false);

  // One byte less than the limit.
  ClearAndSetProperty(xmp_data.get(), false, kLargePropertyName,
              MockPayload(xmp_max_payload_size - 1));
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, *xmp_data));
  // At the limit.
  ClearAndSetProperty(xmp_data.get(), false, kLargePropertyName,
              MockPayload(xmp_max_payload_size));
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, *xmp_data));
  // One byte over the limit.
  ClearAndSetProperty(xmp_data.get(), false, kLargePropertyName,
              MockPayload(xmp_max_payload_size + 1));
  ASSERT_FALSE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                      out_filename, *xmp_data));
}

TEST(XmpWriter, WriteXmpStandardXmpLimitWithEmptyExtendedSection) {
  const string out_filename = TempFileAbsolutePath(kOutFile);
  int xmp_max_payload_size =  XmpConst::MaxBufferSize()
      - kXmpStandardBoilerplateSizeWithExtendedSection
      - strlen(kLargePropertyName) - 2
      - strlen(XmpConst::HasExtensionPrefix())
      - strlen(XmpConst::HasExtension()) - 32  // Length of GUID.
      - 5;  // Extra delimiter chars.
  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);

  // One byte less than the limit.
  ClearAndSetProperty(xmp_data.get(), false, kLargePropertyName,
              MockPayload(xmp_max_payload_size - 1));
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, *xmp_data));
  // At the limit.
  ClearAndSetProperty(xmp_data.get(), false, kLargePropertyName,
              MockPayload(xmp_max_payload_size));
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, *xmp_data));
  // One byte over the limit.
  ClearAndSetProperty(xmp_data.get(), false, kLargePropertyName,
              MockPayload(xmp_max_payload_size + 1));
  ASSERT_FALSE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                      out_filename, *xmp_data));
}

TEST(XmpWriter, WriteXmpExtendedSectionLimits) {
  const string out_filename = TempFileAbsolutePath(kOutFile);
  int xmp_extended_section_max_payload_size =
      XmpConst::ExtendedMaxBufferSize() - kXmpExtendedBoilerplateSize
      - strlen(kLargePropertyName) - 2  // Extra delimiter chars for payload.
      - XmpConst::ExtensionHeaderOffset()
      - strlen(XmpConst::ExtensionHeader()) - 32 - 1  // Header and GUID.
      - 3;  // Extra delimiter chars for extension.

  std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);

  // One under the limit.
  ClearAndSetProperty(xmp_data.get(), true, kLargePropertyName,
              MockPayload(xmp_extended_section_max_payload_size - 1));
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, *xmp_data));
  std::string xmp_from_file;
  ReadFileToStringOrDie(out_filename, &xmp_from_file);
  ASSERT_EQ(1, NumExtendedSections(xmp_from_file));

  // Just at the limit.
  ClearAndSetProperty(xmp_data.get(), true, kLargePropertyName,
              MockPayload(xmp_extended_section_max_payload_size));
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, *xmp_data));
  xmp_from_file.clear();
  ReadFileToStringOrDie(out_filename, &xmp_from_file);
  ASSERT_EQ(1, NumExtendedSections(xmp_from_file));

  // One byte over the limit.
  ClearAndSetProperty(xmp_data.get(), true, kLargePropertyName,
              MockPayload(xmp_extended_section_max_payload_size + 1));
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, *xmp_data));
  ReadFileToStringOrDie(out_filename, &xmp_from_file);
  ASSERT_EQ(2, NumExtendedSections(xmp_from_file));

  // 10X.
  ClearAndSetProperty(xmp_data.get(), true, kLargePropertyName,
              MockPayload(10 * xmp_extended_section_max_payload_size));
  ASSERT_TRUE(WriteLeftEyeAndXmpMeta(TestXmpCreator::GetFakeJpegPayload(),
                                     out_filename, *xmp_data));
  ReadFileToStringOrDie(out_filename, &xmp_from_file);
  ASSERT_EQ(10, NumExtendedSections(xmp_from_file));
}

}  // namespace
}  // namespace xmpmeta
