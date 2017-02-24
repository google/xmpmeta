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

#include "xmpmeta/xml/search.h"

#include <libxml/tree.h>

#include "gtest/gtest.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::xml::FromXmlChar;
using xmpmeta::xml::ToXmlChar;

namespace xmpmeta {
namespace xml {
namespace {

TEST(Search, DepthFirstSearchDocForExistingNodeWithoutPrefix) {
  // Set up XML structure.
  xmlNodePtr root_node = xmlNewNode(nullptr, ToXmlChar("NodeName"));
  xmlNodePtr child_node = xmlNewNode(nullptr, ToXmlChar("ChildNode"));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar("1.0"));

  xmlDocSetRootElement(xml_doc, root_node);
  xmlAddChild(root_node, child_node);

  xmlNodePtr found_node = xml::DepthFirstSearch(xml_doc, "ChildNode");
  ASSERT_NE(nullptr, found_node);
  EXPECT_EQ(string("ChildNode"), string(FromXmlChar(found_node->name)));

  xmlFreeDoc(xml_doc);
}

TEST(Search, DepthFirstSearchDocForExistingNodeWithPrefixAndNullNamespace) {
  // Set up XML structure.
  xmlNodePtr root_node = xmlNewNode(nullptr, ToXmlChar("NodeName"));
  const char* child_node_prefix = "NamespaceName";
  const char* child_node_name = "ChildNode";
  xmlNodePtr child_node = xmlNewNode(nullptr, ToXmlChar(child_node_name));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar("1.0"));

  xmlDocSetRootElement(xml_doc, root_node);
  xmlAddChild(root_node, child_node);

  xmlNodePtr found_node = xml::DepthFirstSearch(xml_doc, child_node_prefix,
                                                child_node_name);
  ASSERT_EQ(nullptr, found_node);

  xmlFreeDoc(xml_doc);
}

TEST(Search, DepthFirstSearchDocForExistingNodeWithPrefix) {
  // Set up XML structure.
  xmlNodePtr root_node = xmlNewNode(nullptr, ToXmlChar("NodeName"));
  const char* child_node_prefix = "NamespaceName";
  const char* child_node_name = "ChildNode";
  xmlNsPtr child_ns = xmlNewNs(nullptr, ToXmlChar("http://somehref.com"),
                               ToXmlChar(child_node_prefix));
  xmlNodePtr child_node = xmlNewNode(child_ns, ToXmlChar(child_node_name));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar("1.0"));

  xmlDocSetRootElement(xml_doc, root_node);
  xmlAddChild(root_node, child_node);

  xmlNodePtr found_node = xml::DepthFirstSearch(xml_doc, child_node_prefix,
                                                child_node_name);
  ASSERT_NE(nullptr, found_node);
  EXPECT_EQ(string("ChildNode"), string(FromXmlChar(found_node->name)));

  xmlFreeDoc(xml_doc);
  xmlFreeNs(child_ns);
}

TEST(Search, DepthFirstSearchDocForNonexistentNode) {
// Set up XML structure.
  xmlNodePtr root_node = xmlNewNode(nullptr, ToXmlChar("NodeName"));
  xmlNodePtr child_node = xmlNewNode(nullptr, ToXmlChar("ChildNode"));
  xmlDocPtr xml_doc = xmlNewDoc(ToXmlChar("1.0"));

  xmlDocSetRootElement(xml_doc, root_node);
  xmlAddChild(root_node, child_node);

  xmlNodePtr found_node = xml::DepthFirstSearch(xml_doc, "NoSuchNode");
  EXPECT_EQ(nullptr, found_node);

  xmlFreeDoc(xml_doc);
}

TEST(Search, DepthFirstSearchNodeForExistingNodeWithoutPrefix) {
// Set up XML structure.
  xmlNodePtr root_node = xmlNewNode(nullptr, ToXmlChar("NodeName"));
  xmlNodePtr child_node = xmlNewNode(nullptr, ToXmlChar("ChildNode"));

  xmlAddChild(root_node, child_node);

  xmlNodePtr found_node = xml::DepthFirstSearch(root_node, "ChildNode");
  ASSERT_NE(nullptr, found_node);
  EXPECT_EQ(string("ChildNode"), string(FromXmlChar(found_node->name)));

  xmlFreeNode(root_node);
}

TEST(Search, DepthFirstSearchNodeForExistingNodeWithPrefixAndNullNamespace) {
// Set up XML structure.
  xmlNodePtr root_node = xmlNewNode(nullptr, ToXmlChar("NodeName"));
  const char* child_node_name = "ChildNode";
  xmlNodePtr child_node = xmlNewNode(nullptr, ToXmlChar(child_node_name));

  xmlAddChild(root_node, child_node);

  xmlNodePtr found_node = xml::DepthFirstSearch(root_node, "NonexistentPrefix",
                                                child_node_name);
  ASSERT_EQ(nullptr, found_node);

  xmlFreeNode(root_node);
}

TEST(Search, DepthFirstSearchNodeForExistingNodeWithPrefix) {
  // Set up XML structure.
  xmlNodePtr root_node = xmlNewNode(nullptr, ToXmlChar("NodeName"));
  const char* child_node_prefix = "NamespaceName";
  const char* child_node_name = "ChildNode";
  xmlNsPtr child_ns = xmlNewNs(nullptr, ToXmlChar("http://somehref.com"),
                               ToXmlChar(child_node_prefix));
  xmlNodePtr child_node = xmlNewNode(child_ns, ToXmlChar(child_node_name));

  xmlAddChild(root_node, child_node);

  xmlNodePtr found_node = xml::DepthFirstSearch(root_node, child_node_prefix,
                                                child_node_name);
  ASSERT_NE(nullptr, found_node);
  EXPECT_EQ(string("ChildNode"), string(FromXmlChar(found_node->name)));

  xmlFreeNode(root_node);
  xmlFreeNs(child_ns);
}

TEST(Search, DepthFirstSearchNodeForSelf) {
  // Set up XML structure.
  xmlNodePtr root_node = xmlNewNode(nullptr, ToXmlChar("NodeName"));

  xmlNodePtr found_node = xml::DepthFirstSearch(root_node, "NodeName");
  ASSERT_NE(nullptr, found_node);
  EXPECT_EQ(string("NodeName"), string(FromXmlChar(found_node->name)));

  xmlFreeNode(root_node);
}

TEST(Search, DepthFirstSearchNodeForNonexistentNode) {
// Set up XML structure.
  xmlNodePtr root_node = xmlNewNode(nullptr, ToXmlChar("NodeName"));
  xmlNodePtr child_node = xmlNewNode(nullptr, ToXmlChar("ChildNode"));

  xmlAddChild(root_node, child_node);

  xmlNodePtr found_node = xml::DepthFirstSearch(root_node, "NoSuchode");
  EXPECT_EQ(nullptr, found_node);

  xmlFreeNode(root_node);
}

}  // namespace
}  // namespace xml
}  // namespace xmpmeta
