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
# Extract xdmlib version from <XDMLIB_SOURCE_ROOT>/include/xdmlib/version.h
# so that we only have a single definition of the xdmlib version, not two
# one in the source and one in CMakeLists.txt.
macro(read_xdmlib_version_from_source XDMLIB_SOURCE_ROOT)
  set(XDMLIB_VERSION_FILE ${XDMLIB_SOURCE_ROOT}/includes/xdmlib/version.h)
  if (NOT EXISTS ${XDMLIB_VERSION_FILE})
    message(FATAL_ERROR "Cannot find xdmlib version.h file in specified "
      "xdmlib source directory: ${XDMLIB_SOURCE_ROOT}, it is not here: "
      "${XDMLIB_VERSION_FILE}")
  endif()

  file(READ ${XDMLIB_VERSION_FILE} XDMLIB_VERSION_FILE_CONTENTS)

  string(REGEX MATCH "#define XDMLIB_VERSION_MAJOR [0-9]+"
    XDMLIB_VERSION_MAJOR "${XDMLIB_VERSION_FILE_CONTENTS}")
  string(REGEX REPLACE "#define XDMLIB_VERSION_MAJOR ([0-9]+)" "\\1"
    XDMLIB_VERSION_MAJOR "${XDMLIB_VERSION_MAJOR}")
  # NOTE: if (VAR) is FALSE if VAR is numeric and <= 0, as such we cannot use
  #       it for testing version numbers, which might well be zero, at least
  #       for the patch version, hence check for empty string explicitly.
  if ("${XDMLIB_VERSION_MAJOR}" STREQUAL "")
    message(FATAL_ERROR "Failed to extract xdmlib major version from "
      "${XDMLIB_VERSION_FILE}")
  endif()

  string(REGEX MATCH "#define XDMLIB_VERSION_MINOR [0-9]+"
    XDMLIB_VERSION_MINOR "${XDMLIB_VERSION_FILE_CONTENTS}")
  string(REGEX REPLACE "#define XDMLIB_VERSION_MINOR ([0-9]+)" "\\1"
    XDMLIB_VERSION_MINOR "${XDMLIB_VERSION_MINOR}")
  if ("${XDMLIB_VERSION_MINOR}" STREQUAL "")
    message(FATAL_ERROR "Failed to extract xdmlib minor version from "
      "${XDMLIB_VERSION_FILE}")
  endif()

  string(REGEX MATCH "#define XDMLIB_VERSION_REVISION [0-9]+"
    XDMLIB_VERSION_PATCH "${XDMLIB_VERSION_FILE_CONTENTS}")
  string(REGEX REPLACE "#define XDMLIB_VERSION_REVISION ([0-9]+)" "\\1"
    XDMLIB_VERSION_PATCH "${XDMLIB_VERSION_PATCH}")
  if ("${XDMLIB_VERSION_PATCH}" STREQUAL "")
    message(FATAL_ERROR "Failed to extract xdmlib patch version from "
      "${XDMLIB_VERSION_FILE}")
  endif()

  # This is on a single line s/t CMake does not interpret it as a list of
  # elements and insert ';' separators which would result in 3.;2.;0 nonsense.
  set(XDMLIB_VERSION "${XDMLIB_VERSION_MAJOR}.${XDMLIB_VERSION_MINOR}.${XDMLIB_VERSION_PATCH}")

  message(STATUS "Detected xdmlib version: ${XDMLIB_VERSION} from "
    "${XDMLIB_VERSION_FILE}")
endmacro()
