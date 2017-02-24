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

#include <string>
#include <stack>

#include "glog/logging.h"
#include "xmpmeta/xml/utils.h"

using xmpmeta::xml::FromXmlChar;

namespace xmpmeta {
namespace xml {

xmlNodePtr DepthFirstSearch(const xmlDocPtr parent, const char* name) {
  return DepthFirstSearch(parent, "", name);
}

xmlNodePtr DepthFirstSearch(const xmlDocPtr parent, const char* prefix,
                            const char* name) {
  if (parent == nullptr || parent->children == nullptr) {
    LOG(ERROR) << "XML doc was null or has no XML nodes";
    return nullptr;
  }
  xmlNodePtr result;
  for (xmlNodePtr node = parent->children; node != nullptr;
       node = node->next) {
    result = DepthFirstSearch(node, prefix, name);
    if (result != nullptr) {
      return result;
    }
  }
  LOG(WARNING) << "No node matching " << prefix << ":" << name << " was found";
  return nullptr;
}

xmlNodePtr DepthFirstSearch(const xmlNodePtr parent, const char* name) {
  return DepthFirstSearch(parent, "", name);
}

xmlNodePtr DepthFirstSearch(const xmlNodePtr parent, const char* prefix,
                            const char* name) {
  if (parent == nullptr) {
    LOG(ERROR) << "XML node was null";
    return nullptr;
  }
  std::stack<xmlNodePtr> node_stack;
  node_stack.push(parent);
  while (!node_stack.empty()) {
    const xmlNodePtr current_node = node_stack.top();
    node_stack.pop();
    if (strcmp(FromXmlChar(current_node->name), name) == 0) {
      if (!prefix || strlen(prefix) == 0) {
        return current_node;
      }
      if (current_node->ns && current_node->ns->prefix &&
          strcmp(FromXmlChar(current_node->ns->prefix), prefix) == 0) {
        return current_node;
      }
    }
    std::stack<xmlNodePtr> stack_to_reverse;
    for (xmlNodePtr child = current_node->children; child != nullptr;
         child = child->next) {
      stack_to_reverse.push(child);
    }
    while (!stack_to_reverse.empty()) {
      node_stack.push(stack_to_reverse.top());
      stack_to_reverse.pop();
    }
  }
  return nullptr;
}

}  // namespace xml
}  // namespace xmpmeta
