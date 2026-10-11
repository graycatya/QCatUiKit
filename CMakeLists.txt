# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.24)
include(.cmake.conf)
project(QCatUiKit VERSION "${QCATUIKIT_REPO_MODULE_VERSION}" LANGUAGES CXX)

# Release identity is independent of Debug/Release compiler configurations.
set(QCATUIKIT_RELEASE_CHANNEL "experimental" CACHE STRING "Release channel: stable or experimental")
set_property(CACHE QCATUIKIT_RELEASE_CHANNEL PROPERTY STRINGS stable experimental)
if(NOT QCATUIKIT_RELEASE_CHANNEL STREQUAL "stable" AND
   NOT QCATUIKIT_RELEASE_CHANNEL STREQUAL "experimental")
    message(FATAL_ERROR "QCATUIKIT_RELEASE_CHANNEL must be stable or experimental")
endif()
set(QCATUIKIT_ABI_VERSION 1)
set(QCATUIKIT_VERSION_STRING "${PROJECT_VERSION}")
set(QCATUIKIT_IS_EXPERIMENTAL 0)
if(QCATUIKIT_RELEASE_CHANNEL STREQUAL "experimental")
    string(APPEND QCATUIKIT_VERSION_STRING "-experimental")
    set(QCATUIKIT_IS_EXPERIMENTAL 1)
endif()
message(STATUS "QCatUiKit ${QCATUIKIT_VERSION_STRING} (ABI ${QCATUIKIT_ABI_VERSION})")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)
option(QCATUIKIT_BUILD_EXAMPLES "Build the component gallery" ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")
set(QML_IMPORT_PATH "${QT_QML_OUTPUT_DIRECTORY}" CACHE STRING "QML tooling import path")

add_subdirectory(src)
if(QCATUIKIT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if(BUILD_TESTING)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test QuickTest)
    add_subdirectory(tests)
endif()

configure_package_config_file(cmake/QCatUiKitConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/QCatUiKitConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QCatUiKit")
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/QCatUiKitConfigVersion.cmake"
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(EXPORT QCatUiKitTargets NAMESPACE QCatUiKit::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QCatUiKit")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/QCatUiKitConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/QCatUiKitConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QCatUiKit")
install(FILES LICENSE NOTICE DESTINATION "${CMAKE_INSTALL_DATADIR}/QCatUiKit")
install(DIRECTORY LICENSES/ DESTINATION "${CMAKE_INSTALL_DATADIR}/QCatUiKit/licenses")
