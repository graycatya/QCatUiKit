# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.24)
project(QCatUiKitStarter LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QuickControls2)
find_package(QCatUiKit 1 REQUIRED)
qt_standard_project_setup(REQUIRES 6.8)
list(APPEND QML_IMPORT_PATH "${QCatUiKit_QML_IMPORT_PATH}")

qt_add_executable(qcat-ui-starter main.cpp)
qt_add_qml_module(qcat-ui-starter
    URI QCatUiKitStarter
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    # This consumer loads the installed shared QML plugins at runtime.
    NO_IMPORT_SCAN
    IMPORT_PATH "${QCatUiKit_QML_IMPORT_PATH}"
    QML_FILES Main.qml)
target_link_libraries(qcat-ui-starter PRIVATE
    QCatUiKit::QCatUiKit Qt6::Quick Qt6::QuickControls2)
