# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.24)
project(QCatVectorConsumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(QCatUiKit 1 REQUIRED)
add_executable(vector-consumer main.cpp)
target_link_libraries(vector-consumer PRIVATE QCatUiKit::QCatUiKit)
target_compile_definitions(vector-consumer PRIVATE QCAT_INSTALLED_QML_PATH="${QCatUiKit_QML_IMPORT_PATH}")
