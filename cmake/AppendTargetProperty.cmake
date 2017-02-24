# Copyright 2016 The XMPMeta Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################
#
# Append item(s) to a property on a declared CMake target:
#
#    append_target_property(target property item_to_append1
#                                           [... item_to_appendN])
#
# The set_target_properties() CMake function will overwrite the contents of the
# specified target property.  This function instead appends to it, so can
# be called multiple times with the same target & property to iteratively
# populate it.
function(append_target_property TARGET PROPERTY)
  if (NOT TARGET ${TARGET})
    message(FATAL_ERROR "Invalid target: ${TARGET} cannot append: ${ARGN} "
      "to property: ${PROPERTY}")
  endif()
  if (NOT PROPERTY)
    message(FATAL_ERROR "Invalid property to update for target: ${TARGET}")
  endif()
  # Get the initial state of the specified property for the target s/t
  # we can append to it (not overwrite it).
  get_target_property(INITIAL_PROPERTY_STATE ${TARGET} ${PROPERTY})
  if (NOT INITIAL_PROPERTY_STATE)
    # Ensure that if the state is unset, we do not insert the XXX-NOTFOUND
    # returned by CMake into the property.
    set(INITIAL_PROPERTY_STATE "")
  endif()
  # Delistify (remove ; separators) the potentially set of items to append
  # to the specified target property.
  string(REPLACE ";" " " ITEMS_TO_APPEND "${ARGN}")
  set_target_properties(${TARGET} PROPERTIES ${PROPERTY}
    "${INITIAL_PROPERTY_STATE} ${ITEMS_TO_APPEND}")
endfunction()
