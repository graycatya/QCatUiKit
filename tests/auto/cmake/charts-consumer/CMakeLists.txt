# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.24)
project(QCatUiKitChartsConsumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(QCatUiKit 1 REQUIRED)
add_executable(charts-consumer main.cpp)
target_link_libraries(charts-consumer PRIVATE QCatUiKit::QCatUiKit)
