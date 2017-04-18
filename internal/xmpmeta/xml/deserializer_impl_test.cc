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

#include "xmpmeta/xml/deserializer_impl.h"

#include <memory>
#include <string>
#include <vector>

#include <libxml/tree.h>

#include "gtest/gtest.h"
#include "strings/numbers.h"
#include "xmpmeta/base64.h"
#include "xmpmeta/xml/const.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::xml::ToXmlChar;
using xmpmeta::xml::XmlConst;

namespace xmpmeta {
namespace xml {
namespace {

// Creates a new XML node.
xmlNodePtr NewNode(xmlNsPtr xml_ns, const char* node_name) {
  return xmlNewNode(xml_ns, ToXmlChar(node_name));
}

// Creates a new namespace.
xmlNsPtr NewNamespace(const char* href, const char* ns_name) {
  return xmlNewNs(nullptr, ToXmlChar(href), ToXmlChar(ns_name));
}

template <typename T>
xmlNodePtr RdfSeqNodeFromArray(const std::vector<T> data) {
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  for (int i = 0; i < data.size(); i++) {
    xmlNodePtr li_node = NewNode(nullptr, XmlConst::RdfLi());
    xmlNodeSetContent(li_node, ToXmlChar(std::to_string(data[i]).data()));
    xmlAddChild(seq_node, li_node);
  }
  return seq_node;
}

TEST(DeserializerImpl, CreateDeserializerEmptyChildName) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);

  DeserializerImpl deserializer(node);
  std::unique_ptr<Deserializer> created_deserializer =
      deserializer.CreateDeserializer("", "");
  ASSERT_EQ(nullptr, created_deserializer.get());

  xmlFreeNode(node);
}

TEST(DeserializerImpl, CreateDeserializerNoChildNode) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  DeserializerImpl deserializer(node);

  const string child_name("ChildName");
  std::unique_ptr<Deserializer> created_deserializer =
      deserializer.CreateDeserializer("", child_name);
  ASSERT_EQ(nullptr, created_deserializer.get());

  xmlFreeNode(node);
}

TEST(DeserializerImpl, CreateDeserializerWithoutPrefixOnPrefixedNode) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  const string child_name("ChildName");
  xmlNsPtr child_ns = NewNamespace("http://somehref.com", "ChildNodePrefix");
  xmlNodePtr child_node = NewNode(child_ns, child_name.data());
  xmlSetNsProp(child_node, nullptr, ToXmlChar("PropertyOne"),
               ToXmlChar("PropertyOneValue"));
  xmlAddChild(node, child_node);

  xmlNsPtr another_child_ns =
      NewNamespace("http://somehref.com", "AnotherChildNodePrefix");

  xmlNodePtr another_child_node = NewNode(another_child_ns, child_name.data());
  xmlSetNsProp(another_child_node, nullptr, ToXmlChar("PropertyTwo"),
               ToXmlChar("PropertyTwoValue"));
  xmlAddChild(node, another_child_node);

  DeserializerImpl deserializer(node);
  std::unique_ptr<Deserializer> created_deserializer =
      deserializer.CreateDeserializer("", child_name);
  ASSERT_NE(nullptr, created_deserializer.get());
  // First child node was matched, and not the second one.
  string value;
  EXPECT_TRUE(created_deserializer->ParseString("", "PropertyOne", &value));
  EXPECT_EQ(string("PropertyOneValue"), value);
  EXPECT_FALSE(created_deserializer->ParseString("", "PropertyTwo", &value));

  xmlFreeNs(child_ns);
  xmlFreeNs(another_child_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, CreateDeserializerWithPrefixOnPrefixedNode) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  const string child_name("ChildName");
  xmlNsPtr child_ns = NewNamespace("http://somehref.com", "ChildNodePrefix");
  xmlNodePtr child_node = NewNode(child_ns, child_name.data());
  xmlSetNsProp(child_node, child_ns, ToXmlChar("PropertyOne"),
               ToXmlChar("PropertyOneValue"));
  xmlAddChild(node, child_node);

  DeserializerImpl deserializer(node);
  std::unique_ptr<Deserializer> created_deserializer =
      deserializer.CreateDeserializer("AnotherChildNodePrefix", child_name);
  ASSERT_EQ(nullptr, created_deserializer.get());

  xmlNsPtr another_child_ns =
      NewNamespace("http://somehref.com", "AnotherChildNodePrefix");
  xmlNodePtr another_child_node = NewNode(another_child_ns, child_name.data());
  xmlSetNsProp(another_child_node, another_child_ns, ToXmlChar("PropertyTwo"),
               ToXmlChar("PropertyTwoValue"));
  xmlAddChild(node, another_child_node);
  created_deserializer =
      deserializer.CreateDeserializer("AnotherChildNodePrefix", child_name);
  ASSERT_NE(nullptr, created_deserializer.get());

  xmlFreeNs(child_ns);
  xmlFreeNs(another_child_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, CreateDeserializerOnNonPrefixedNode) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  const string child_name("ChildName");
  xmlNodePtr child_node = NewNode(nullptr, child_name.data());
  xmlAddChild(node, child_node);

  DeserializerImpl deserializer(node);
  std::unique_ptr<Deserializer> created_deserializer =
      deserializer.CreateDeserializer("SomePrefix", child_name);
  ASSERT_EQ(nullptr, created_deserializer.get());
  created_deserializer = deserializer.CreateDeserializer("", child_name);
  ASSERT_NE(nullptr, created_deserializer.get());

  xmlFreeNode(node);
}

TEST(DeserializerImpl, CreateDeserializerFromListElementEmptyName) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlNodePtr seq_parent_node = NewNode(nullptr, "Parent");
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  xmlNodePtr li_node = NewNode(nullptr, XmlConst::RdfLi());
  xmlAddChild(seq_node, li_node);
  xmlAddChild(seq_parent_node, seq_node);
  xmlAddChild(node, seq_parent_node);

  DeserializerImpl deserializer(node);
  std::unique_ptr<Deserializer> created_deserializer =
      deserializer.CreateDeserializerFromListElementAt("", "", 0);
  ASSERT_EQ(nullptr, created_deserializer.get());

  xmlFreeNode(node);
}

TEST(DeserializerImpl, CreateDeserializerFromListElementNegativeIndex) {
  const char* node_name = "NodeName";
  const char* parent_name = "Parent";
  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlNodePtr seq_parent_node = NewNode(nullptr, parent_name);
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  xmlNodePtr li_node = NewNode(nullptr, XmlConst::RdfLi());
  xmlAddChild(seq_node, li_node);
  xmlAddChild(seq_parent_node, seq_node);
  xmlAddChild(node, seq_parent_node);

  DeserializerImpl deserializer(node);
  std::unique_ptr<Deserializer> created_deserializer =
      deserializer.CreateDeserializerFromListElementAt("", parent_name, -1);
  ASSERT_EQ(nullptr, created_deserializer.get());

  xmlFreeNode(node);
}

TEST(DeserializerImpl, CreateDeserializerFromListElementNoSeqNode) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com", "NodePrefix");
  xmlNodePtr node = NewNode(node_ns, node_name);

  DeserializerImpl deserializer(node);
  std::unique_ptr<Deserializer> created_deserializer =
      deserializer.CreateDeserializerFromListElementAt("NodePrefix", node_name,
                                                       0);
  ASSERT_EQ(nullptr, created_deserializer.get());

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, CreateDeserializerFromListElementBadLiElementIndex) {
  const char* parent_name = "Parent";
  xmlNodePtr node = NewNode(nullptr, "NodeName");
  xmlNsPtr parent_ns = NewNamespace("http://somehref.com", "Prefix");
  xmlNodePtr seq_parent_node = NewNode(parent_ns, parent_name);
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  xmlNodePtr li_node = NewNode(nullptr, XmlConst::RdfLi());
  xmlAddChild(seq_node, li_node);
  xmlAddChild(seq_parent_node, seq_node);
  xmlAddChild(node, seq_parent_node);

  DeserializerImpl deserializer(node);
  std::unique_ptr<Deserializer> created_deserializer =
      deserializer.CreateDeserializerFromListElementAt("Prefix", parent_name,
                                                       1);
  ASSERT_EQ(nullptr, created_deserializer.get());

  xmlFreeNs(parent_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, CreateDeserializerFromListElement) {
  const char* node_name = "NodeName";
  const char* parent_name = "Parent";

  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlNsPtr parent_ns = NewNamespace("http://somehref.com", "Prefix");
  xmlNodePtr seq_parent_node = NewNode(parent_ns, parent_name);
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  xmlNodePtr li_node1 = NewNode(nullptr, XmlConst::RdfLi());
  xmlNodePtr li_node2 = NewNode(nullptr, XmlConst::RdfLi());
  xmlNodePtr li_node3 = NewNode(nullptr, XmlConst::RdfLi());
  xmlAddChild(seq_node, li_node1);
  xmlAddChild(seq_node, li_node2);
  xmlAddChild(seq_node, li_node3);
  xmlAddChild(seq_parent_node, seq_node);
  xmlAddChild(node, seq_parent_node);

  // Test different indices.
  DeserializerImpl deserializer(node);
  ASSERT_NE(
      nullptr,
      deserializer.CreateDeserializerFromListElementAt("Prefix", parent_name, 0)
          .get());
  ASSERT_NE(nullptr,
            deserializer.CreateDeserializerFromListElementAt("", parent_name, 0)
                .get());
  ASSERT_NE(
      nullptr,
      deserializer.CreateDeserializerFromListElementAt("Prefix", parent_name, 1)
          .get());
  ASSERT_NE(
      nullptr,
      deserializer.CreateDeserializerFromListElementAt("Prefix", parent_name, 2)
          .get());

  xmlFreeNs(parent_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseDoubleArrayNoSeqParentNodeElement) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com", "NodePrefix");
  xmlNodePtr node = NewNode(node_ns, node_name);

  DeserializerImpl deserializer(node);
  std::vector<double> values;
  ASSERT_FALSE(deserializer.ParseDoubleArray("", "nonexistentnode", &values));
  ASSERT_TRUE(values.empty());

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseDoubleArrayNoSeqElement) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com", "NodePrefix");
  xmlNodePtr node = NewNode(node_ns, node_name);

  DeserializerImpl deserializer(node);
  std::vector<double> values;
  ASSERT_FALSE(deserializer.ParseDoubleArray("NodePrefix", node_name, &values));
  ASSERT_TRUE(values.empty());

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseDoubleArrayNoListElements) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com", "NodePrefix");
  xmlNodePtr node = NewNode(node_ns, node_name);
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  xmlAddChild(node, seq_node);

  DeserializerImpl deserializer(node);
  std::vector<double> values;
  ASSERT_TRUE(deserializer.ParseDoubleArray("NodePrefix", node_name, &values));
  ASSERT_TRUE(values.empty());

  ASSERT_TRUE(deserializer.ParseDoubleArray("", node_name, &values));
  ASSERT_TRUE(values.empty());

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseTwoArraysOfReals) {
  const std::vector<double> expected_values_one = {1.234, 5.678, 8.9011};
  xmlNodePtr seq_node_one = RdfSeqNodeFromArray(expected_values_one);

  const std::vector<double> expected_values_two = {3.21, 5.291};
  xmlNodePtr seq_node_two = RdfSeqNodeFromArray(expected_values_two);

  // Create and set parents of the rdf:Seq nodes.
  const char* child_one_name = "ChildOne";
  const char* child_two_name = "ChildTwo";

  xmlNsPtr node_ns = NewNamespace("http://somehref.com", "NodePrefix");
  xmlNodePtr node = NewNode(node_ns, "NodeName");
  xmlNodePtr child_one_node = NewNode(nullptr, child_one_name);
  xmlNsPtr child_two_ns = NewNamespace("http://somehref.com", "ChildTwoPrefix");
  xmlNodePtr child_two_node = NewNode(child_two_ns, child_two_name);
  xmlAddChild(node, child_one_node);
  xmlAddChild(node, child_two_node);
  xmlAddChild(child_one_node, seq_node_one);
  xmlAddChild(child_two_node, seq_node_two);

  // Deserialize the first child node.
  DeserializerImpl deserializer(node);
  std::vector<double> values;
  ASSERT_FALSE(deserializer.ParseDoubleArray("NonexistentPrefix",
                                             child_one_name, &values));
  EXPECT_TRUE(values.empty());
  ASSERT_TRUE(deserializer.ParseDoubleArray("", child_one_name, &values));
  EXPECT_EQ(expected_values_one, values);

  // Deserialize the second child node.
  ASSERT_TRUE(
      deserializer.ParseDoubleArray("ChildTwoPrefix", child_two_name, &values));
  EXPECT_EQ(expected_values_two, values);
  ASSERT_TRUE(deserializer.ParseDoubleArray("", child_two_name, &values));
  EXPECT_EQ(expected_values_two, values);

  // Clean up. This also frees all of the node's children.
  xmlFreeNs(node_ns);
  xmlFreeNs(child_two_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseTwoArraysOfInts) {
  const std::vector<int> expected_values_one = {20, 30, 40};
  xmlNodePtr seq_node_one = RdfSeqNodeFromArray(expected_values_one);

  const std::vector<int> expected_values_two = {55, 77};
  xmlNodePtr seq_node_two = RdfSeqNodeFromArray(expected_values_two);

  // Create and set parents of the rdf:Seq nodes.
  const char* child_one_name = "ChildOne";
  const char* child_two_name = "ChildTwo";

  xmlNsPtr node_ns = NewNamespace("http://somehref.com", "NodePrefix");
  xmlNodePtr node = NewNode(node_ns, "NodeName");
  xmlNodePtr child_one_node = NewNode(nullptr, child_one_name);
  xmlNsPtr child_two_ns = NewNamespace("http://somehref.com", "ChildTwoPrefix");
  xmlNodePtr child_two_node = NewNode(child_two_ns, child_two_name);
  xmlAddChild(node, child_one_node);
  xmlAddChild(node, child_two_node);
  xmlAddChild(child_one_node, seq_node_one);
  xmlAddChild(child_two_node, seq_node_two);

  // Deserialize the first child node.
  DeserializerImpl deserializer(node);
  std::vector<int> values;
  ASSERT_FALSE(
      deserializer.ParseIntArray("NonexistentPrefix", child_one_name, &values));
  EXPECT_TRUE(values.empty());
  ASSERT_TRUE(deserializer.ParseIntArray("", child_one_name, &values));
  EXPECT_EQ(expected_values_one, values);

  // Deserialize the second child node.
  ASSERT_TRUE(
      deserializer.ParseIntArray("ChildTwoPrefix", child_two_name, &values));
  EXPECT_EQ(expected_values_two, values);
  ASSERT_TRUE(deserializer.ParseIntArray("", child_two_name, &values));
  EXPECT_EQ(expected_values_two, values);

  // Clean up. This also frees all of the node's children.
  xmlFreeNs(node_ns);
  xmlFreeNs(child_two_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseIntegerArrayWithDoubleTypes) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  xmlNodePtr li_node1 = NewNode(nullptr, XmlConst::RdfLi());
  xmlNodePtr li_node2 = NewNode(nullptr, XmlConst::RdfLi());

  // Set list node values.
  xmlNodeSetContent(li_node1, ToXmlChar("1"));
  xmlNodeSetContent(li_node2, ToXmlChar("5.234"));

  // Set up node descendants.
  xmlAddChild(seq_node, li_node1);
  xmlAddChild(seq_node, li_node2);

  xmlAddChild(node, seq_node);
  DeserializerImpl deserializer(node);
  std::vector<int> values;
  ASSERT_FALSE(deserializer.ParseIntArray("", node_name, &values));
  EXPECT_EQ(std::vector<int>({1}), values);

  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseIntegerArrayWithStringType) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  xmlNodePtr li_node1 = NewNode(nullptr, XmlConst::RdfLi());
  xmlNodePtr li_node2 = NewNode(nullptr, XmlConst::RdfLi());

  // Set list node values.
  xmlNodeSetContent(li_node1, ToXmlChar("1"));
  xmlNodeSetContent(li_node2, ToXmlChar("ThisIsNotAnInteger"));

  // Set up node descendants.
  xmlAddChild(seq_node, li_node1);
  xmlAddChild(seq_node, li_node2);

  xmlAddChild(node, seq_node);
  DeserializerImpl deserializer(node);
  std::vector<int> values;
  EXPECT_FALSE(deserializer.ParseIntArray("", node_name, &values));

  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseDoubleArray) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  xmlNodePtr li_node1 = NewNode(nullptr, XmlConst::RdfLi());
  xmlNodePtr li_node2 = NewNode(nullptr, XmlConst::RdfLi());
  xmlNodePtr li_node3 = NewNode(nullptr, XmlConst::RdfLi());

  // Set list node values.
  const std::vector<double> expected_values = {1.234, 5.678, 8.9011};
  xmlNodeSetContent(li_node1,
                    ToXmlChar(std::to_string(expected_values[0]).data()));
  xmlNodeSetContent(li_node2,
                    ToXmlChar(std::to_string(expected_values[1]).data()));
  xmlNodeSetContent(li_node3,
                    ToXmlChar(std::to_string(expected_values[2]).data()));

  // Set up node descendants.
  xmlAddChild(seq_node, li_node1);
  xmlAddChild(seq_node, li_node2);
  xmlAddChild(seq_node, li_node3);

  xmlAddChild(node, seq_node);
  DeserializerImpl deserializer(node);
  std::vector<double> values;
  ASSERT_TRUE(deserializer.ParseDoubleArray("", node_name, &values));
  EXPECT_EQ(expected_values, values);

  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseDoubleArrayWithStringType) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlNodePtr seq_node = NewNode(nullptr, XmlConst::RdfSeq());
  xmlNodePtr li_node1 = NewNode(nullptr, XmlConst::RdfLi());
  xmlNodePtr li_node2 = NewNode(nullptr, XmlConst::RdfLi());

  // Set list node values.
  const std::vector<double> expected_values = {1.234, 5.678, 8.9011};
  xmlNodeSetContent(li_node1, ToXmlChar("1.23"));
  xmlNodeSetContent(li_node2, ToXmlChar("NotADouble"));

  // Set up node descendants.
  xmlAddChild(seq_node, li_node1);
  xmlAddChild(seq_node, li_node2);

  xmlAddChild(node, seq_node);
  DeserializerImpl deserializer(node);
  std::vector<double> values;
  EXPECT_FALSE(deserializer.ParseDoubleArray("", node_name, &values));

  xmlFreeNode(node);
}

// Variations on a theme of property parser tests.
// Doubles.
TEST(DeserializerImpl, ParseDoubleEmptyName) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlSetProp(node, ToXmlChar("Name"), ToXmlChar(std::to_string(1.234).data()));
  DeserializerImpl deserializer(node);
  double value;
  ASSERT_FALSE(deserializer.ParseDouble("", "", &value));
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseDoubleNoProperty) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  DeserializerImpl deserializer(node);
  double value;
  ASSERT_FALSE(deserializer.ParseDouble("", "Name", &value));
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseDoublePropertyNoPrefix) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  string property_name("Name");
  double property_value = 1.24;

  xmlSetNsProp(node, nullptr, ToXmlChar(property_name.data()),
               ToXmlChar(std::to_string(property_value).data()));
  DeserializerImpl deserializer(node);

  double value;
  ASSERT_TRUE(deserializer.ParseDouble("", property_name, &value));
  EXPECT_FLOAT_EQ(property_value, value);
  ASSERT_FALSE(deserializer.ParseDouble("SomePrefix", property_name, &value));

  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseDouble) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com/", node_name);
  xmlNodePtr node = NewNode(nullptr, node_name);
  string property_name("Name");
  double property_value = 1.24;

  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar(std::to_string(property_value).data()));
  DeserializerImpl deserializer(node);

  double value;
  ASSERT_TRUE(deserializer.ParseDouble("NodeName", property_name, &value));
  EXPECT_FLOAT_EQ(property_value, value);

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

// String.
TEST(DeserializerImpl, ParseStringEmptyName) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlSetProp(node, ToXmlChar("Name"), ToXmlChar("Value"));
  DeserializerImpl deserializer(node);
  string value;
  ASSERT_FALSE(deserializer.ParseString("", "", &value));
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseStringNoProperty) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  DeserializerImpl deserializer(node);
  string value;
  ASSERT_FALSE(deserializer.ParseString("", "Name", &value));
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseStringPropertyNoPrefix) {
  xmlNodePtr node = NewNode(nullptr, "NodeName");
  string property_name("Name");
  string property_value("Value");

  xmlSetNsProp(node, nullptr, ToXmlChar(property_name.data()),
               ToXmlChar(property_value.data()));
  DeserializerImpl deserializer(node);

  string value;
  ASSERT_TRUE(deserializer.ParseString("", property_name, &value));
  ASSERT_EQ(property_value, value);
  ASSERT_FALSE(deserializer.ParseString("SomePrefix", property_name, &value));

  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseString) {
  xmlNsPtr node_ns = NewNamespace("http://somehref.com/", "NodeName");
  xmlNodePtr node = NewNode(nullptr, "NodeName");
  string property_name("Name");
  string property_value("Value");

  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar(property_value.data()));
  DeserializerImpl deserializer(node);

  string value;
  ASSERT_TRUE(deserializer.ParseString("NodeName", property_name, &value));
  ASSERT_EQ(property_value, value);

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

// Base64.
TEST(DeserializerImpl, ParseBase64EmptyName) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  xmlSetProp(node, ToXmlChar("Name"), ToXmlChar("123ABC"));
  DeserializerImpl deserializer(node);
  string value;
  ASSERT_FALSE(deserializer.ParseBase64("", "", &value));
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseBase64NoProperty) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  DeserializerImpl deserializer(node);
  string value;
  ASSERT_FALSE(deserializer.ParseBase64("", "Name", &value));
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseBase64EmptyPrefix) {
  const char* node_name = "NodeName";
  xmlNodePtr node = NewNode(nullptr, node_name);
  string property_name("Name");
  string property_value("SomeValue");

  string base64_encoded;
  EncodeBase64(property_value, &base64_encoded);
  xmlSetNsProp(node, nullptr, ToXmlChar(property_name.data()),
               ToXmlChar(base64_encoded.data()));
  DeserializerImpl deserializer(node);

  string value;
  ASSERT_FALSE(deserializer.ParseBase64("SomePrefix", property_name, &value));
  ASSERT_TRUE(value.empty());
  ASSERT_TRUE(deserializer.ParseBase64("", property_name, &value));
  ASSERT_EQ(property_value, value);

  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseBase64) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com/", node_name);
  xmlNodePtr node = NewNode(node_ns, node_name);
  string property_name("Name");
  string property_value("SomeValue");

  string base64_encoded;
  EncodeBase64(property_value, &base64_encoded);
  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar(base64_encoded.data()));
  DeserializerImpl deserializer(node);

  string value;
  ASSERT_TRUE(deserializer.ParseBase64(node_name, property_name, &value));
  ASSERT_EQ(property_value, value);
  ASSERT_TRUE(deserializer.ParseBase64("", property_name, &value));
  ASSERT_EQ(property_value, value);

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseIntArrayBase64) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com/", node_name);
  xmlNodePtr node = NewNode(node_ns, node_name);
  string property_name("Name");
  std::vector<int> property_values;
  for (int i = 0; i <= 100; i++) {
    property_values.push_back(i);
  }

  string base64_encoded;
  EncodeIntArrayBase64(property_values, &base64_encoded);
  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar(base64_encoded.data()));
  DeserializerImpl deserializer(node);

  std::vector<int> values;
  EXPECT_TRUE(
      deserializer.ParseIntArrayBase64(node_name, property_name, values));
  EXPECT_EQ(property_values, values);
  EXPECT_TRUE(deserializer.ParseIntArrayBase64("", property_name, values));
  EXPECT_EQ(property_values, values);
  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseFloatArrayBase64) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com/", node_name);
  xmlNodePtr node = NewNode(node_ns, node_name);
  string property_name("Name");
  std::vector<float> property_values;
  for (int i = 0; i <= 100; i++) {
    property_values.push_back(static_cast<float>(i));
  }

  string base64_encoded;
  EncodeFloatArrayBase64(property_values, &base64_encoded);
  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar(base64_encoded.data()));
  DeserializerImpl deserializer(node);

  std::vector<float> values;
  EXPECT_TRUE(
      deserializer.ParseFloatArrayBase64(node_name, property_name, values));
  EXPECT_EQ(property_values, values);
  EXPECT_TRUE(deserializer.ParseFloatArrayBase64("", property_name, values));
  EXPECT_EQ(property_values, values);

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseBoolean) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com/", node_name);
  xmlNodePtr node = NewNode(node_ns, node_name);
  string property_name("FlagProperty");

  // Lowercase.
  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar("false"));
  DeserializerImpl deserializer(node);

  bool value;
  EXPECT_TRUE(deserializer.ParseBoolean(node_name, property_name, &value));
  EXPECT_FALSE(value);
  EXPECT_TRUE(deserializer.ParseBoolean("", property_name, &value));
  EXPECT_FALSE(value);

  // No prefix.
  xmlSetNsProp(node, nullptr, ToXmlChar(property_name.data()),
               ToXmlChar("false"));
  EXPECT_TRUE(deserializer.ParseBoolean("", property_name, &value));
  EXPECT_FALSE(value);

  // Uppercase.
  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar("TRUE"));
  EXPECT_TRUE(deserializer.ParseBoolean("", property_name, &value));
  EXPECT_TRUE(value);
  EXPECT_TRUE(deserializer.ParseBoolean(node_name, property_name, &value));
  EXPECT_TRUE(value);

  // Camelcase.
  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar("fALse"));
  EXPECT_TRUE(deserializer.ParseBoolean("", property_name, &value));
  EXPECT_FALSE(value);
  EXPECT_TRUE(deserializer.ParseBoolean(node_name, property_name, &value));
  EXPECT_FALSE(value);

  // Some other string.
  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar("falsies"));
  EXPECT_FALSE(deserializer.ParseBoolean("", property_name, &value));

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

TEST(DeserializerImpl, ParseInt) {
  const char* node_name = "NodeName";
  xmlNsPtr node_ns = NewNamespace("http://somehref.com/", node_name);
  xmlNodePtr node = NewNode(node_ns, node_name);
  string property_name("Name");
  double property_value = 12345;

  xmlSetNsProp(node, node_ns, ToXmlChar(property_name.data()),
               ToXmlChar(std::to_string(property_value).data()));
  DeserializerImpl deserializer(node);

  double value;
  ASSERT_TRUE(deserializer.ParseDouble(node_name, property_name, &value));
  EXPECT_EQ(property_value, value);

  // Empty prefix.
  xmlSetNsProp(node, nullptr, ToXmlChar(property_name.data()),
               ToXmlChar(std::to_string(property_value).data()));
  ASSERT_TRUE(deserializer.ParseDouble("", property_name, &value));
  EXPECT_EQ(property_value, value);

  // The prefixed property was not overwritten.
  ASSERT_TRUE(deserializer.ParseDouble(node_name, property_name, &value));
  EXPECT_EQ(property_value, value);

  xmlFreeNs(node_ns);
  xmlFreeNode(node);
}

}  // namespace
}  // namespace xml
}  // namespace xmpmeta
