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
# Findlibxml.cmake - Find libxml library, version >= 3.
#
# This module defines the following variables:
#
# LIBXML_FOUND: TRUE iff libxml is found.
# LIBXML_INCLUDE_DIRS: Include directories for libxml.
#
# LIBXML_VERSION: Extracted from libxml/src/Core/util/Macros.h
# LIBXML_WORLD_VERSION: Equal to 3 if LIBXML_VERSION = 3.2.0
# LIBXML_MAJOR_VERSION: Equal to 2 if LIBXML_VERSION = 3.2.0
# LIBXML_MINOR_VERSION: Equal to 0 if LIBXML_VERSION = 3.2.0
#
# The following variables control the behaviour of this module:
#
# LIBXML_INCLUDE_DIR_HINTS: List of additional directories in which to
#                          search for libxml includes, e.g: /timbuktu/libxml3.
#
# The following variables are also defined by this module, but in line with
# CMake recommended FindPackage() module style should NOT be referenced directly
# by callers (use the plural variables detailed above instead).  These variables
# do however affect the behaviour of the module via FIND_[PATH/LIBRARY]() which
# are NOT re-called (i.e. search for library is not repeated) if these variables
# are set with valid values _in the CMake cache_. This means that if these
# variables are set directly in the cache, either by the user in the CMake GUI,
# or by the user passing -DVAR=VALUE directives to CMake when called (which
# explicitly defines a cache variable), then they will be used verbatim,
# bypassing the HINTS variables and other hard-coded search locations.
#
# LIBXML_INCLUDE_DIR: Include directory for CXSparse, not including the
#                    include directory of any dependencies.

# Called if we failed to find libxml or any of it's required dependencies,
# unsets all public (designed to be used externally) variables and reports
# error message at priority depending upon [REQUIRED/QUIET/<NONE>] argument.
macro(LIBXML_REPORT_NOT_FOUND REASON_MSG)
  unset(LIBXML_FOUND)
  unset(LIBXML_INCLUDE_DIRS)
  # Make results of search visible in the CMake GUI if libxml has not
  # been found so that user does not have to toggle to advanced view.
  mark_as_advanced(CLEAR LIBXML_INCLUDE_DIR)
  # Note <package>_FIND_[REQUIRED/QUIETLY] variables defined by FindPackage()
  # use the camelcase library name, not uppercase.
  if (libxml_FIND_QUIETLY)
    message(STATUS "Failed to find libxml - " ${REASON_MSG} ${ARGN})
  elseif (libxml_FIND_REQUIRED)
    message(FATAL_ERROR "Failed to find libxml - " ${REASON_MSG} ${ARGN})
  else()
    # Neither QUIETLY nor REQUIRED, use no priority which emits a message
    # but continues configuration and allows generation.
    message("-- Failed to find libxml - " ${REASON_MSG} ${ARGN})
  endif ()
  return()
endmacro(LIBXML_REPORT_NOT_FOUND)

# Search user-installed locations first, so that we prefer user installs
# to system installs where both exist.
#
# TODO: Add standard Windows search locations for libxml.
list(APPEND LIBXML_CHECK_INCLUDE_DIRS
  /usr/local/include
  /usr/local/homebrew/include # Mac OS X
  /opt/local/var/macports/software # Mac OS X.
  /opt/local/include
  /usr/include)
# Additional suffixes to try appending to each search path.
list(APPEND LIBXML_CHECK_PATH_SUFFIXES
  libxml2 # Default root directory for libxml.
  libxml2/src/include/libxml ) # Windows (for C:/Program Files prefix).

# Search supplied hint directories first if supplied.
find_path(LIBXML_INCLUDE_DIR
  NAMES libxml
  PATHS ${LIBXML_INCLUDE_DIR_HINTS}
  ${LIBXML_CHECK_INCLUDE_DIRS}
  PATH_SUFFIXES ${LIBXML_CHECK_PATH_SUFFIXES})

if (NOT LIBXML_INCLUDE_DIR OR
    NOT EXISTS ${LIBXML_INCLUDE_DIR})
  libxml_report_not_found(
    "Could not find libxml include directory, set LIBXML_INCLUDE_DIR to "
    "path to libxml include directory, e.g. /usr/local/include/libxml2.")
endif (NOT LIBXML_INCLUDE_DIR OR
       NOT EXISTS ${LIBXML_INCLUDE_DIR})

# Mark internally as found, then verify. LIBXML_REPORT_NOT_FOUND() unsets
# if called.
set(LIBXML_FOUND TRUE)

# Extract libxml version from libxml/src/Core/util/Macros.h
if (LIBXML_INCLUDE_DIR)
  set(LIBXML_VERSION_FILE ${LIBXML_INCLUDE_DIR}/libxml/xmlversion.h)
  if (NOT EXISTS ${LIBXML_VERSION_FILE})
    libxml_report_not_found(
      "Could not find file: ${LIBXML_VERSION_FILE} "
      "containing version information in libxml install located at: "
      "${LIBXML_INCLUDE_DIR}.")
  else (NOT EXISTS ${LIBXML_VERSION_FILE})
    file(READ ${LIBXML_VERSION_FILE} LIBXML_VERSION_FILE_CONTENTS)

    string(REGEX MATCH "#define LIBXML_DOTTED_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+\""
      LIBXML_VERSION "${LIBXML_VERSION_FILE_CONTENTS}")
    string(REGEX REPLACE "#define LIBXML_DOTTED_VERSION \"(.*)\"" "\\1"
      LIBXML_VERSION "${LIBXML_VERSION}")
  endif (NOT EXISTS ${LIBXML_VERSION_FILE})
endif (LIBXML_INCLUDE_DIR)

# Set standard CMake FindPackage variables if found.
if (LIBXML_FOUND)
  set(LIBXML_INCLUDE_DIRS ${LIBXML_INCLUDE_DIR})
endif (LIBXML_FOUND)

# Handle REQUIRED / QUIET optional arguments and version.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libxml
  REQUIRED_VARS LIBXML_INCLUDE_DIRS
  VERSION_VAR LIBXML_VERSION)

# Only mark internal variables as advanced if we found libxml, otherwise
# leave it visible in the standard GUI for the user to set manually.
if (LIBXML_FOUND)
  mark_as_advanced(FORCE LIBXML_INCLUDE_DIR)
endif (LIBXML_FOUND)
