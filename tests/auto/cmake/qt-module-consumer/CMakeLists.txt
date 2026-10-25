# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.24)
project(QCatUiKitQtModuleConsumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QCatUiKit)
qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(qt-module-consumer main.cpp)
qt_add_qml_module(qt-module-consumer
    URI InstallCheck
    VERSION 1.0
    QML_FILES Main.qml)
target_link_libraries(qt-module-consumer PRIVATE Qt6::QCatUiKit Qt6::Quick)

# Deliberately avoid linking the backing library: this must load the QML plugin.
qt_add_executable(qml-plugin-consumer main.cpp)
target_compile_definitions(qml-plugin-consumer PRIVATE QCAT_PLUGIN_ONLY)
target_link_libraries(qml-plugin-consumer PRIVATE Qt6::Quick)
