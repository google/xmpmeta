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

#ifndef XMPMETA_XML_SERIALIZER_H_
#define XMPMETA_XML_SERIALIZER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/port.h"

namespace xmpmeta {
namespace xml {

// Serializes properties for a hierarchy of objects.
// Example:
//  BookSerializer serializer();
//  // Serialize a list of objects.
//  std::unique_ptr<Serializer> book_list_serializer =
//      serializer->CreateListSerializer("Books");
//  for (Book *book : book_list) {
//    std::unique_ptr<Serializer> book_serializer =
//        cameras_serializer->CreateItemSerializer("Book");
//    success &= book->Serialize(book_serializer.get());
//
//    // Write properties in an object.
//    // This would be called from the Book class.
//    string book_name("Book");
//    std::unique_ptr<Serializer> book_info_serializer =
//        book_serializer->CreateSerializer("Info");
//    book_info_serializer->WriteProperty("Author", "Cereal Eyser");
//    book_info_serializer->WriteProperty("ISBN", "314159265359");
//    std::unique_ptr<Serializer> genre_serializer =
//        book_serializer->CreateSeralizer("Genre", true);
//    std::unique_ptr<Serializer> fantasy_serializer =
//        genre_serializer->CreateSerialzer("Fantasy");
//    // Serialize genre properties here.
//  }

class Serializer {
 public:
  virtual ~Serializer() {}

  // Returns a Serializer for an object that is an item in a list.
  virtual std::unique_ptr<Serializer>
      CreateItemSerializer(const string& prefix,
                           const string& item_name) const = 0;

  // Returns a Serializer for a list of objects.
  virtual std::unique_ptr<Serializer>
      CreateListSerializer(const string& prefix,
                           const string& list_name) const = 0;

  // Creates a serializer from the current serializer.
  // node_ns_name is the XML namespace to which the newly created node belongs.
  // If this parameter is an empty string, the new node will not belong to a
  // namespace.
  // node_name is the name of the new node. This paramter cannot be an empty
  // string.
  virtual std::unique_ptr<Serializer>
      CreateSerializer(const string& node_ns_name,
                       const string& node_name) const = 0;

  // Serializes a property with the given prefix.
  // Example: <NodeName PropertyPrefix:PropertyName="PropertyValue" />
  virtual bool WriteBoolProperty(const string& prefix, const string& name,
                                 bool value) const = 0;
  virtual bool WriteProperty(const string& prefix, const string& name,
                             const string& value) const = 0;

  // Serializes the collection of values.
  virtual bool WriteIntArray(const string& prefix, const string& array_name,
                             const std::vector<int>& values) const = 0;
  virtual bool WriteDoubleArray(const string& prefix, const string& array_name,
                                const std::vector<double>& values) const = 0;
};

}  // namespace xml
}  // namespace xmpmeta

#endif  // XMPMETA_XML_SERIALIZER_H_
