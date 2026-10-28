# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.24)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(QCatMascotProbe LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Quick)
    qt_standard_project_setup(REQUIRES 6.8)
    include(CTest)
endif()
qt_add_executable(qcat-mascot-probe main.cpp)
target_link_libraries(qcat-mascot-probe PRIVATE Qt6::Quick)
qt_add_resources(qcat-mascot-probe probe PREFIX "/" FILES Main.qml)
if(WIN32)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets)
    target_sources(qcat-mascot-probe PRIVATE windowsprobe.h windowsprobe.cpp)
    target_link_libraries(qcat-mascot-probe PRIVATE Qt6::Widgets user32 gdi32)
    qt_add_resources(qcat-mascot-probe windows_probe PREFIX "/"
        FILES WindowsProbe.qml WindowsOverlay.qml)
endif()
if(BUILD_TESTING)
    add_test(NAME mascot_probe_classification COMMAND qcat-mascot-probe --self-test)
    set_tests_properties(mascot_probe_classification PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
    if(WIN32)
        add_test(NAME mascot_probe_reject_headless_windows COMMAND ${CMAKE_COMMAND}
            "-DPROBE=$<TARGET_FILE:qcat-mascot-probe>"
            "-DREPORT=${CMAKE_CURRENT_BINARY_DIR}/headless-windows-report.json"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_headless.cmake")
        set_tests_properties(mascot_probe_reject_headless_windows PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
    endif()
endif()
