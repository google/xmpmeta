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
# This file installs both xmpmeta and xdmlib.

cmake_minimum_required(VERSION 2.8.0)
project(xmpmeta C CXX)

# C++ 11 check.
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Make CMake aware of the cmake folder for local FindXXX scripts,
# append rather than set in case the user has passed their own
# additional paths via -D.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(UpdateCacheVariable)

# On OS X, add the Homebrew prefix to the set of prefixes searched by
# CMake in find_path & find_library.  This should ensure that we can
# still build Xmpmeta even if Homebrew is installed in a non-standard
# location (not /usr/local).
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  find_program(HOMEBREW_EXECUTABLE brew)
  mark_as_advanced(FORCE HOMEBREW_EXECUTABLE)
  if (HOMEBREW_EXECUTABLE)
    # Detected a Homebrew install, query for its install prefix.
    execute_process(COMMAND ${HOMEBREW_EXECUTABLE} --prefix
      OUTPUT_VARIABLE HOMEBREW_INSTALL_PREFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Detected Homebrew with install prefix: "
      "${HOMEBREW_INSTALL_PREFIX}, adding to CMake search paths.")
    list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_INSTALL_PREFIX}")
  endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# Set postfixes for generated libraries based on buildtype.
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_DEBUG_POSTFIX "-debug")

# Read the xmpmeta version from the source, such that we only ever have a single
# definition of the xmpmeta version.
include(ReadXmpmetaVersionFromSource)
read_xmpmeta_version_from_source(${CMAKE_SOURCE_DIR})

# Read the xdmlib version from the source, such that we only ever have a single
# definition of the xdmlib version.
include(ReadXdmlibVersionFromSource)
read_xdmlib_version_from_source(${CMAKE_SOURCE_DIR})


option(MINIGLOG "Use a stripped down version of glog." OFF)
option(EXPORT_BUILD_DIR
  "Export build directory using CMake (enables external use without install)." OFF)
option(BUILD_TESTING "Enable tests" ON)

# Libxml.
find_package(Libxml REQUIRED)
if (LIBXML_FOUND)
  if (LIBXML_VERSION VERSION_LESS 2.9.1)
    message(FATAL_ERROR "-- xmpmeta requires libxml version >= 2.9.3, "
      "detected version of libxml is:${LIBXML_VERSION}")
  endif (LIBXML_VERSION VERSION_LESS 2.9.1)

  message("-- Found Libxml version ${LIBXML_VERSION}: ${LIBXML_INCLUDE_DIRS}")
  # Ensure that only MPL2 licensed code is part of the default build.
  message("")
endif (LIBXML_FOUND)


# MiniGLog.
if (MINIGLOG)
  message("-- Compiling minimal glog substitute into Xmpmeta.")
  set(GLOG_INCLUDE_DIRS internal/xmpmeta/external/miniglog)
  message("-- Using minimal glog substitute (include): ${GLOG_INCLUDE_DIRS}")

  # Mark as advanced (remove from default GUI view) the glog search
  # variables in case user disables MINIGLOG, FindGlog did not find it, so
  # made search variables visible in GUI for user to set, but then user enables
  # MINIGLOG instead of setting them.
  mark_as_advanced(FORCE GLOG_INCLUDE_DIRS
                         GLOG_LIBRARY)
else (MINIGLOG)
  # Don't search with REQUIRED so that configuration continues if not found and
  # we can output an error messages explaining MINIGLOG option.
  find_package(Glog)
  if (GLOG_FOUND)
    message("-- Found Google Log header in: ${GLOG_INCLUDE_DIRS}")
  else (GLOG_FOUND)
    set(GLOG_INCLUDE_DIRS internal/xmpmeta/external/miniglog)
    #    message(FATAL_ERROR "Can't find Google Log. Please set GLOG_INCLUDE_DIRS & "
    # "GLOG_LIBRARY or enable MINIGLOG option to use minimal glog "
    #  "implementation.")
  endif (GLOG_FOUND)
endif (MINIGLOG)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_HAS_CXX11_FLAG)
if (CXX11 AND COMPILER_HAS_CXX11_FLAG)
  # Update CMAKE_REQUIRED_FLAGS used by CheckCXXSourceCompiles to include
  # -std=c++11 s/t we will detect the C++11 versions of unordered_map &
  # shared_ptr if they exist.
  set(CMAKE_REQUIRED_FLAGS -std=c++11)
endif (CXX11 AND COMPILER_HAS_CXX11_FLAG)

include_directories(
  includes
  internal
  internal/xdmlib
  internal/xmpmeta
  internal/xmpmeta/xml
  internal/xmpmeta/external
  ${GLOG_INCLUDE_DIRS})

include_directories(SYSTEM ${LIBXML_INCLUDE_DIRS})

add_subdirectory(internal/xdmlib)
add_subdirectory(internal/xmpmeta)

# Setup installation of Xmpmeta public headers.
file(GLOB XMPMETA_HDRS ${CMAKE_SOURCE_DIR}/include/xmpmeta/*.h)
install(FILES ${XMPMETA_HDRS} DESTINATION include/xmpmeta)

file(GLOB XMPMETA_PUBLIC_INTERNAL_HDRS ${CMAKE_SOURCE_DIR}/includes/xmpmeta/internal/*.h)
install(FILES ${XMPMETA_PUBLIC_INTERNAL_HDRS} DESTINATION includes/xmpmeta/internal)

# Setup installation of Xmpmeta public headers.
file(GLOB XDMLIB_HDRS ${CMAKE_SOURCE_DIR}/include/xdmlib/*.h)
install(FILES ${XDMLIB_HDRS} DESTINATION include/xdmlib)

file(GLOB XDMLIB_PUBLIC_INTERNAL_HDRS ${CMAKE_SOURCE_DIR}/includes/xdmlib/internal/*.h)
install(FILES ${XDMLIB_PUBLIC_INTERNAL_HDRS} DESTINATION includes/xdmlib/internal)


if (MINIGLOG)
  # Install miniglog header if being used as logging #includes appear in
  # installed public Xmpmeta headers.
  install(FILES ${CMAKE_SOURCE_DIR}/internal/xmpmeta/external/miniglog/glog/logging.h
          DESTINATION includes/xmpmeta/internal/miniglog/glog)
endif (MINIGLOG)

# Xmpmeta supports two mechanisms by which it can be detected & imported into
# client code which uses CMake via find_package(Xmpmeta):
#
#   1) Installation (e.g. to /usr/local), using CMake's install() function.
#
#   2) (Optional) Export of the current build directory into the local CMake
#      package registry, using CMake's export() function.  This allows use of
#      Xmpmeta from other projects without requiring installation.
#
# In both cases, we need to generate a configured XmpmetaConfig.cmake which
# includes additional autogenerated files which in concert create an imported
# target for Xmpmeta in a client project when find_package(Xmpmeta) is invoked.
# The key distinctions are where this file is located, and whether client code
# references installed copies of the compiled Xmpmeta headers/libraries,
# (option #1: installation), or the originals in the source/build directories
# (option #2: export of build directory).
#
# NOTE: If Xmpmeta is both exported and installed, provided that the installation
#       path is present in CMAKE_MODULE_PATH when find_package(Xmpmeta) is called,
#       the installed version is preferred.

# Create a XmpmetaConfigVersion.cmake file containing the version information,
# used by both export() & install().
configure_file("${CMAKE_SOURCE_DIR}/cmake/XmpmetaConfigVersion.cmake.in"
               "${CMAKE_BINARY_DIR}/XmpmetaConfigVersion.cmake" @ONLY)

# Install method #1: Put Xmpmeta in CMAKE_INSTALL_PREFIX: /usr/local or equivalent.

# Set the install path for the installed XmpmetaConfig.cmake configuration file
# relative to CMAKE_INSTALL_PREFIX.
if (WIN32)
  set(RELATIVE_CMAKECONFIG_INSTALL_DIR CMake)
else ()
  set(RELATIVE_CMAKECONFIG_INSTALL_DIR lib/cmake/Xmpmeta)
endif ()

# This "exports" for installation all targets which have been put into the
# export set "XmpmetaExport". This generates a XmpmetaTargets.cmake file which,
# when read in by a client project as part of find_package(Xmpmeta) creates
# imported library targets for Xmpmeta (with dependency relations) which can be
# used in target_link_libraries() calls in the client project to use Xmpmeta.
install(EXPORT XmpmetaExport
        DESTINATION ${RELATIVE_CMAKECONFIG_INSTALL_DIR} FILE XmpmetaTargets.cmake)

# Save the relative path from the installed XmpmetaConfig.cmake file to the
# install prefix.  We do not save an absolute path in case the installed package
# is subsequently relocated after installation (on Windows).
file(RELATIVE_PATH INSTALL_ROOT_REL_CONFIG_INSTALL_DIR
     ${CMAKE_INSTALL_PREFIX}/${RELATIVE_CMAKECONFIG_INSTALL_DIR}
     ${CMAKE_INSTALL_PREFIX})

# Configure a XmpmetaConfig.cmake file for an installed version of Xmpmeta from the
# template, reflecting the current build options.
#
# NOTE: The -install suffix is necessary to distinguish the install version from
#       the exported version, which must be named XmpmetaConfig.cmake in
#       CMAKE_BINARY_DIR to be detected.  The suffix is removed when
#       it is installed.
set(SETUP_XMPMETA_CONFIG_FOR_INSTALLATION TRUE)
configure_file("${CMAKE_SOURCE_DIR}/cmake/XmpmetaConfig.cmake.in"
               "${CMAKE_BINARY_DIR}/XmpmetaConfig-install.cmake" @ONLY)

# Configure a XdmlibConfig.cmake file for an installed version of Xdmlib from the
# template, reflecting the current build options.
#
# NOTE: The -install suffix is necessary to distinguish the install version from
#       the exported version, which must be named XmpmetaConfig.cmake in
#       CMAKE_BINARY_DIR to be detected.  The suffix is removed when
#       it is installed.
set(SETUP_XDMLIB_CONFIG_FOR_INSTALLATION TRUE)
configure_file("${CMAKE_SOURCE_DIR}/cmake/XdmlibConfig.cmake.in"
               "${CMAKE_BINARY_DIR}/XdmlibConfig-install.cmake" @ONLY)

# Install the configuration files into the same directory as the autogenerated
# XmpmetaTargets.cmake file.  We include the find_package() scripts for libraries
# whose headers are included in the public API of Xmpmeta and should thus be
# present in XMPMETA_INCLUDE_DIRS.
install(FILES "${CMAKE_BINARY_DIR}/XmpmetaConfig-install.cmake"
        RENAME XmpmetaConfig.cmake
        DESTINATION ${RELATIVE_CMAKECONFIG_INSTALL_DIR})
install(FILES "${CMAKE_BINARY_DIR}/XdmlibConfig-install.cmake"
        RENAME XdmlibConfig.cmake
        DESTINATION ${RELATIVE_CMAKECONFIG_INSTALL_DIR})
install(FILES "${CMAKE_BINARY_DIR}/XmpmetaConfigVersion.cmake"
              "${CMAKE_BINARY_DIR}/XdmlibConfigVersion.cmake"
              "${CMAKE_SOURCE_DIR}/cmake/FindGlog.cmake"
              "${CMAKE_SOURCE_DIR}/cmake/FindLibxml.cmake"
        DESTINATION ${RELATIVE_CMAKECONFIG_INSTALL_DIR})

# Create an uninstall target to remove all installed files.
configure_file("${CMAKE_SOURCE_DIR}/cmake/uninstall.cmake.in"
               "${CMAKE_BINARY_DIR}/cmake/uninstall.cmake"
               @ONLY)
add_custom_target(uninstall
                  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake/uninstall.cmake)

if (MINIGLOG)
  # Install miniglog header if being used as logging #includes appear in
  # installed public Ceres headers.
  install(FILES ${CMAKE_SOURCE_DIR}/internal/xmpmeta/external/miniglog/glog/logging.h
          DESTINATION includes/xmpmeta/internal/miniglog/glog)
endif (MINIGLOG)


# Install method #2: Put Xmpmeta build into local CMake registry.
#
# Optionally export the Xmpmeta build directory into the local CMake package
# registry (~/.cmake/packages on *nix & OS X).  This allows the detection &
# use of Xmpmeta without requiring that it be installed.
if (EXPORT_BUILD_DIR)
  message("-- Export Xmpmeta build directory to local CMake package registry.")

  # Save the relative path from the build directory to the source directory.
  file(RELATIVE_PATH INSTALL_ROOT_REL_CONFIG_INSTALL_DIR
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR})

  # Analogously to install(EXPORT ...), export the Xmpmeta target from the build
  # directory as a package called Xmpmeta into the local CMake package registry.
  export(TARGETS xmpmeta FILE ${CMAKE_BINARY_DIR}/XmpmetaTargets.cmake)
  export(PACKAGE ${CMAKE_PROJECT_NAME})

  # Configure a XmpmetaConfig.cmake file for the export of the Xmpmeta build
  # directory from the template, reflecting the current build options.
  set(SETUP_XMPMETA_CONFIG_FOR_INSTALLATION FALSE)
  configure_file("${CMAKE_SOURCE_DIR}/cmake/XmpmetaConfig.cmake.in"
                 "${CMAKE_BINARY_DIR}/XmpmetaConfig.cmake" @ONLY)

endif (EXPORT_BUILD_DIR)
