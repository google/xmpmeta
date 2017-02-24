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
# Extract xmpmeta version from <XMPMETA_SOURCE_ROOT>/include/xmpmeta/version.h
# so that we only have a single definition of the xmpmeta version, not two
# one in the source and one in CMakeLists.txt.
macro(read_xmpmeta_version_from_source XMPMETA_SOURCE_ROOT)
  set(XMPMETA_VERSION_FILE ${XMPMETA_SOURCE_ROOT}/includes/xmpmeta/version.h)
  if (NOT EXISTS ${XMPMETA_VERSION_FILE})
    message(FATAL_ERROR "Cannot find xmpmeta version.h file in specified "
      "xmpmeta source directory: ${XMPMETA_SOURCE_ROOT}, it is not here: "
      "${XMPMETA_VERSION_FILE}")
  endif()

  file(READ ${XMPMETA_VERSION_FILE} XMPMETA_VERSION_FILE_CONTENTS)

  string(REGEX MATCH "#define XMPMETA_VERSION_MAJOR [0-9]+"
    XMPMETA_VERSION_MAJOR "${XMPMETA_VERSION_FILE_CONTENTS}")
  string(REGEX REPLACE "#define XMPMETA_VERSION_MAJOR ([0-9]+)" "\\1"
    XMPMETA_VERSION_MAJOR "${XMPMETA_VERSION_MAJOR}")
  # NOTE: if (VAR) is FALSE if VAR is numeric and <= 0, as such we cannot use
  #       it for testing version numbers, which might well be zero, at least
  #       for the patch version, hence check for empty string explicitly.
  if ("${XMPMETA_VERSION_MAJOR}" STREQUAL "")
    message(FATAL_ERROR "Failed to extract xmpmeta major version from "
      "${XMPMETA_VERSION_FILE}")
  endif()

  string(REGEX MATCH "#define XMPMETA_VERSION_MINOR [0-9]+"
    XMPMETA_VERSION_MINOR "${XMPMETA_VERSION_FILE_CONTENTS}")
  string(REGEX REPLACE "#define XMPMETA_VERSION_MINOR ([0-9]+)" "\\1"
    XMPMETA_VERSION_MINOR "${XMPMETA_VERSION_MINOR}")
  if ("${XMPMETA_VERSION_MINOR}" STREQUAL "")
    message(FATAL_ERROR "Failed to extract xmpmeta minor version from "
      "${XMPMETA_VERSION_FILE}")
  endif()

  string(REGEX MATCH "#define XMPMETA_VERSION_REVISION [0-9]+"
    XMPMETA_VERSION_PATCH "${XMPMETA_VERSION_FILE_CONTENTS}")
  string(REGEX REPLACE "#define XMPMETA_VERSION_REVISION ([0-9]+)" "\\1"
    XMPMETA_VERSION_PATCH "${XMPMETA_VERSION_PATCH}")
  if ("${XMPMETA_VERSION_PATCH}" STREQUAL "")
    message(FATAL_ERROR "Failed to extract xmpmeta patch version from "
      "${XMPMETA_VERSION_FILE}")
  endif()

  # This is on a single line s/t CMake does not interpret it as a list of
  # elements and insert ';' separators which would result in 3.;2.;0 nonsense.
  set(XMPMETA_VERSION "${XMPMETA_VERSION_MAJOR}.${XMPMETA_VERSION_MINOR}.${XMPMETA_VERSION_PATCH}")

  message(STATUS "Detected xmpmeta version: ${XMPMETA_VERSION} from "
    "${XMPMETA_VERSION_FILE}")
endmacro()
