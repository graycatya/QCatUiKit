# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.24)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(QCatMascotPlayer LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    find_package(QCatUiKit 1 REQUIRED)
    qt_standard_project_setup(REQUIRES 6.8)
endif()
qt_add_executable(qcat-mascot-player main.cpp)
target_link_libraries(qcat-mascot-player PRIVATE QCatUiKit::QCatUiKit Qt6::Quick)
qt_add_resources(qcat-mascot-player player PREFIX "/mascot-player" FILES Main.qml
    catgray/manifest.json catgray/spritesheet.png catgray/LICENSE.txt
    catgray-lottie/mascot.json
    catgray-lottie/idle.json catgray-lottie/thinking.json catgray-lottie/working.json
    catgray-lottie/waiting.json catgray-lottie/speaking.json catgray-lottie/success.json
    catgray-lottie/error.json catgray-lottie/walk.json catgray-lottie/run.json
    catgray-lottie/jump.json catgray-lottie/fallback.png catgray-lottie/LICENSE.txt)
if(QT_QML_OUTPUT_DIRECTORY)
    target_compile_definitions(qcat-mascot-player PRIVATE QCAT_MASCOT_IMPORT_PATH="${QT_QML_OUTPUT_DIRECTORY}")
    add_dependencies(qcat-mascot-player QCatUiKitplugin)
endif()
