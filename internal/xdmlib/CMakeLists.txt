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

set(XDMLIB_INTERNAL_SRC
    audio.cc
    camera.cc
    camera_pose.cc
    cameras.cc
    device.cc
    device_pose.cc
    equirect_model.cc
    image.cc
    mesh.cc
    navigational_connectivity.cc
    point_cloud.cc
    profile.cc
    profiles.cc
    vendor_info.cc
)

# Heuristic for determining LIB_SUFFIX. FHS recommends that 64-bit systems
# install native libraries to lib64 rather than lib. Most distros seem to
# follow this convention with a couple notable exceptions (Debian-based and
# Arch-based distros) which we try to detect here.
if (CMAKE_SYSTEM_NAME MATCHES "Linux" AND
    NOT DEFINED LIB_SUFFIX AND
    NOT CMAKE_CROSSCOMPILING AND
    CMAKE_SIZEOF_VOID_P EQUAL "8" AND
    NOT EXISTS "/etc/debian_version" AND
    NOT EXISTS "/etc/arch-release")
  set(LIB_SUFFIX "64")
endif ()

# Also depend on the header files so that they appear in IDEs.
file(GLOB XDMLIB_INTERNAL_HDRS *.h)

# Build the list of dependencies for xdmlib based on the current configuration.
if (NOT MINIGLOG AND GLOG_FOUND)
  list(APPEND XDMLIB_LIBRARY_PUBLIC_DEPENDENCIES ${GLOG_LIBRARIES})
endif (NOT MINIGLOG AND GLOG_FOUND)

set(XDMLIB_LIBRARY_SOURCE
    ${XDMLIB_INTERNAL_SRC}
    ${XDMLIB_INTERNAL_HDRS}
    ${XMPMETA_INTERNAL_SRC}
    ${XMPMETA_INTERNAL_HDRS})

# Primarily for Android, but optionally for others, compile the minimal
# glog implementation into xdmlib.
if (MINIGLOG)
  list(APPEND XMPMETA_LIBRARY_SOURCE miniglog/glog/logging.cc)
endif (MINIGLOG)

add_library(xdmlib ${XDMLIB_LIBRARY_SOURCE})
set_target_properties(xdmlib PROPERTIES
  VERSION ${XMPMETA_VERSION}
  SOVERSION ${XMPMETA_VERSION_MAJOR})

include(AppendTargetProperty)
# Always build position-independent code (PIC), even when building xdmlib as a
# static library so that shared libraries can link against it, not just
# executables (PIC does not apply on Windows).
if (NOT WIN32 AND NOT BUILD_SHARED_LIBS)
  # Use the explicit POSITION_INDEPENDENT_CODE target property on CMake versions
  # that support it (>= 2.8.9).  Otherwise, manually add the -fPIC flag as an
  # additional compile definitions for the target.
  if (CMAKE_VERSION VERSION_LESS "2.8.9")
    append_target_property(xdmlib COMPILE_FLAGS "-fPIC")
  else()
    # Use set_target_properties() not append_target_property() here as
    # POSITION_INDEPENDENT_CODE is a binary ON/OFF switch.
    set_target_properties(xdmlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
  endif()
endif()

if (CMAKE_VERSION VERSION_LESS "2.8.12")
  # CMake version < 2.8.12 does not support target_compile_options(), warn
  # user that they will have to add compile flags to their own projects
  # manually if required.
  if (CXX11 AND COMPILER_HAS_CXX11_FLAG)
    message("-- Warning: Detected CMake version: ${CMAKE_VERSION} < 2.8.12, "
      "which is the minimum required for compile options to be included in an "
      "exported CMake target, but XMPMETA_USE_CXX11 is enabled and the detected. "
      "compiler requires -std=c++11. The client is responsible for adding "
      "-std=c++11 when using xdmlib.")
  endif()
else()
  # CMake version >= 2.8.12 supports target_compile_options().
  if (CXX11 AND COMPILER_HAS_CXX11_FLAG)
    # If xmpmeta is compiled using C++11, and the compiler requires -std=c++11
    # to be set, then ensure that this requirement is rolled into the exported
    # CMake target, such that client code which uses xdmlib will inherit it (if
    # the CMake version supports it), iff they are NOT compiling for C.  We
    # check for not C, rather than C++ as LINKER_LANGUAGE is often NOTFOUND and
    # then uses the default (C++).
    target_compile_options(xdmlib PUBLIC
      $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,C>>:-std=c++11>)
  endif()
endif()

if (BUILD_SHARED_LIBS)
  # When building a shared library, mark all external libraries as
  # PRIVATE so they don't show up as a dependency.
  target_link_libraries(xdmlib
        LINK_PUBLIC ${XDMLIB_LIBRARY_PUBLIC_DEPENDENCIES}
        LINK_PRIVATE ${XDMLIB_LIBRARY_PRIVATE_DEPENDENCIES})
else (BUILD_SHARED_LIBS)
  # When building a static library, all external libraries are
  # PUBLIC(default) since the user needs to link to them.
  # They will be listed in xdmlibTargets.cmake.
  set(XDMLIB_LIBRARY_DEPENDENCIES
        ${XDMLIB_LIBRARY_PUBLIC_DEPENDENCIES}
        ${XDMLIB_LIBRARY_PRIVATE_DEPENDENCIES})
  target_link_libraries(xdmlib ${XDMLIB_LIBRARY_DEPENDENCIES})
endif (BUILD_SHARED_LIBS)

install(TARGETS xdmlib
        EXPORT  XdmlibExport
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX})

if (BUILD_TESTING AND GFLAGS)
  # The XMPMETA_GFLAGS_NAMESPACE compile definition is NOT stored in
  # XMPMETA_COMPILE_OPTIONS (and thus config.h) as xmpmeta itself does not
  # require gflags, only the tests and examples do.
  add_definitions(-DXMPMETA_GFLAGS_NAMESPACE=${GFLAGS_NAMESPACE})

  add_library(gtest gmock_gtest_all.cc gmock_main.cc)
  if (BUILD_SHARED_LIBS)
    # Define gtest-specific shared library flags for compilation.
    append_target_property(gtest COMPILE_DEFINITIONS
      GTEST_CREATE_SHARED_LIBRARY)
  endif()

  add_library(test_util
              ../xmpmeta/test_xmp_creator.cc
              ../xmpmeta/test_util.cc)

  if (MINIGLOG)
    # When using miniglog, it is compiled into xdmlib, thus xdmlib becomes
    # the library against which other libraries should link for logging.
    target_link_libraries(xmpmeta test_util xdmlib)
  else (MINIGLOG)
    target_link_libraries(${GLOG_LIBRARIES} test_util xdmlib)
  endif (MINIGLOG)

  macro (XDMLIB_TEST NAME)
    add_executable(${NAME}_test ${NAME}_test.cc)
    target_link_libraries(${NAME}_test test_util xdmlib)
    if (BUILD_SHARED_LIBS)
      # Define test-specific shared library flags for linking.
      append_target_property(${NAME}_test COMPILE_DEFINITIONS
        GTEST_LINKED_AS_SHARED_LIBRARY)
    endif()
    add_test(NAME ${NAME}_test
             COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_test
             --test_srcdir
             ${CMAKE_SOURCE_DIR}/xdm/testdata)
  endmacro (XDMLIB_TEST)

  xdmlib_test(audio)
  xdmlib_test(camera)
  xdmlib_test(camera_pose)
  xdmlib_test(cameras)
  xdmlib_test(device)
  xdmlib_test(device_pose)
  xdmlib_test(equirect_model)
  xdmlib_test(image)
  xdmlib_test(mesh)
  xdmlib_test(navigational_connectivity)
  xdmlib_test(point_cloud)
  xdmlib_test(profile)
  xdmlib_test(profiles)
  xdmlib_test(vendor_info)

 endif (BUILD_TESTING AND GFLAGS)
