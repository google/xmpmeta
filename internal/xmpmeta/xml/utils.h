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

#ifndef XMPMETA_XML_UTILS_H_
#define XMPMETA_XML_UTILS_H_

#include <string>

#include <libxml/tree.h>

#include "base/port.h"

namespace xmpmeta {
namespace xml {

// Convenience function to convert an xmlChar* to a char*
inline const char* FromXmlChar(const xmlChar* in) {
  return reinterpret_cast<const char*>(in);
}

// Convenience function to convert a char* to an xmlChar*.
inline const xmlChar* ToXmlChar(const char* in) {
  return reinterpret_cast<const xmlChar*>(in);
}

// Returns the first rdf:Description node; null if not found.
xmlNodePtr GetFirstDescriptionElement(xmlDocPtr parent);

// Returns the first rdf:Seq element found in the XML document.
xmlNodePtr GetFirstSeqElement(xmlDocPtr parent);

// Returns the first rdf:Seq element found in the given node.
// Returns {@code parent} if that is itself an rdf:Seq node.
xmlNodePtr GetFirstSeqElement(xmlNodePtr parent);

// Returns the ith (zero-indexed) rdf:li node in the given rdf:Seq node.
// Returns null if either of {@code index} < 0, {@code node} is null, or is
// not an rdf:Seq node.
xmlNodePtr GetElementAt(xmlNodePtr node, int index);

// Returns the value in an rdf:li node. This is for a node whose value
// does not have a name, e.g. <rdf:li>value</rdf:li>.
// If the given rdf:li node has a nested node, it returns the string
// representation of the contents of those nodes, which replaces the XML
// tags with one whitespace character for each tag character.
// This is treated as undefined behavior; it is the caller's responsibility
// to remove any whitespace and newlines.
const string GetLiNodeContent(xmlNodePtr node);

// Returns the given XML doc serialized to a string.
// For debugging purposes.
const string XmlDocToString(const xmlDocPtr doc);

}  // namespace xml
}  // namespace xmpmeta

#endif  // XMPMETA_XML_UTILS_H_
