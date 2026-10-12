# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.24)
project(QCatLedConsumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(QCatUiKit 1 REQUIRED)
add_executable(led-consumer main.cpp)
target_link_libraries(led-consumer PRIVATE QCatUiKit::QCatUiKit)
