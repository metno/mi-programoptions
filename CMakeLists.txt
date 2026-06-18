# mi-programoptions
#
# Copyright (C) 2019 met.no
#
# Contact information:
# Norwegian Meteorological Institute
# Box 43 Blindern
# 0313 OSLO
# NORWAY
# email: diana@met.no
#
# This file is part of mi-programoptions.
#
# mi-programoptions is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# mi-programoptions is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mi-programoptions; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

PROJECT(mi-programoptions LANGUAGES CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(MI_PROGRAMOPTIONS_MASTER_PROJECT OFF)
IF(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  SET(MI_PROGRAMOPTIONS_MASTER_PROJECT ON)
ENDIF()

INCLUDE(CMakePackageConfigHelpers)
INCLUDE(GNUInstallDirs)
SET(CMAKE_CXX_STANDARD 11)

# Enable all warnings
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  ADD_COMPILE_OPTIONS(-Wall -Wextra -Wpedantic -Wconversion -Wformat -Wformat=2 -Wformat-security -Werror=format-security)
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  ADD_COMPILE_OPTIONS(/W4 /WX)
ENDIF()

SET(MI_PROGRAMOPTIONS_HEADERS
  mi_programoptions.h
  mi_programoptions_version.h
)

IF(MI_PROGRAMOPTIONS_MASTER_PROJECT)
  SET(MI_PROGRAMOPTIONS_LIBRARY_TYPE SHARED)
ELSE()
  MESSAGE(STATUS "Add mi-programoptions subdirectory from ${CMAKE_CURRENT_LIST_DIR}")
  SET(MI_PROGRAMOPTIONS_LIBRARY_TYPE STATIC)
ENDIF()

ADD_LIBRARY(mi-programoptions ${MI_PROGRAMOPTIONS_LIBRARY_TYPE}
  mi_po_option.cc
  mi_po_option_set.cc
  mi_po_parse.cc
  mi_po_value_set.cc
  ${MI_PROGRAMOPTIONS_HEADERS}
)

SET(MI_PROGRAMOPTIONS_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

TARGET_INCLUDE_DIRECTORIES(mi-programoptions
  PUBLIC
  $<BUILD_INTERFACE:${MI_PROGRAMOPTIONS_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

IF(MI_PROGRAMOPTIONS_MASTER_PROJECT)
  # see https://cmake.org/Wiki/CMake_RPATH_handling#Always_full_RPATH
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" INSTALL_LIBDIR_IS_SYSTEM_DIR)
  IF("${INSTALL_LIBDIR_IS_SYSTEM_DIR}" STREQUAL "-1")
    LIST(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  ENDIF()


  FILE(STRINGS "mi_programoptions_version.h" version_defines
    REGEX "#define .*_VERSION_(MAJOR|MINOR|PATCH) ")
  FOREACH(ver ${version_defines})
    IF (ver MATCHES "#define .*_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
      SET(MI_PROGRAMOPTIONS_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    ENDIF()
  ENDFOREACH()

  SET_TARGET_PROPERTIES(mi-programoptions PROPERTIES
    VERSION "${MI_PROGRAMOPTIONS_VERSION_MAJOR}.${MI_PROGRAMOPTIONS_VERSION_MINOR}.${MI_PROGRAMOPTIONS_VERSION_PATCH}"
    SOVERSION "${MI_PROGRAMOPTIONS_VERSION_MAJOR}"
  )

  OPTION(ENABLE_TESTS "Enable unit tests" ON)
  IF(ENABLE_TESTS)
    SET (MI_CPPTEST_3RDPARTY "third_party/mi-cpptest")
    IF ((NOT mi-cpptest_DIR) AND (EXISTS "${CMAKE_SOURCE_DIR}/${MI_CPPTEST_3RDPARTY}/CMakeLists.txt"))
      MESSAGE(STATUS "Including mi-cpptest project from ${MI_CPPTEST_3RDPARTY}")
      ADD_SUBDIRECTORY(${MI_CPPTEST_3RDPARTY})
    ELSE ()
      FIND_PACKAGE(mi-cpptest REQUIRED)
    ENDIF ()

    ENABLE_TESTING()
    ADD_EXECUTABLE(test_programoptions test_programoptions.cc)
    TARGET_LINK_LIBRARIES(test_programoptions PRIVATE mi-programoptions mi-cpptest-main)
    ADD_TEST(NAME test_programoptions COMMAND test_programoptions)
  ENDIF()

  INSTALL(TARGETS mi-programoptions
    EXPORT mi-programoptions
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  INSTALL (
    FILES ${MI_PROGRAMOPTIONS_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  SET(MI_PROGRAMOPTIONS_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/mi-programoptions")

  INSTALL (
    EXPORT mi-programoptions
    DESTINATION "${MI_PROGRAMOPTIONS_CMAKE_DIR}"
    FILE mi-programoptions-targets.cmake
  )

  WRITE_BASIC_PACKAGE_VERSION_FILE(
    "${CMAKE_CURRENT_BINARY_DIR}/mi-programoptions-config-version.cmake"
    VERSION
    "${MI_PROGRAMOPTIONS_VERSION_MAJOR}.${MI_PROGRAMOPTIONS_VERSION_MINOR}.${MI_PROGRAMOPTIONS_VERSION_PATCH}"
    COMPATIBILITY
    SameMajorVersion
  )

  INSTALL(
    FILES
      mi-programoptions-config.cmake
      "${CMAKE_CURRENT_BINARY_DIR}/mi-programoptions-config-version.cmake"
    DESTINATION "${MI_PROGRAMOPTIONS_CMAKE_DIR}"
  )

ENDIF (MI_PROGRAMOPTIONS_MASTER_PROJECT)
