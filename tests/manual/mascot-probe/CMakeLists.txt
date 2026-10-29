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
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets DBus)
    target_sources(qcat-mascot-probe PRIVATE linuxprobe.h linuxprobe.cpp mascotcompanion.h mascotcompanion.cpp)
    target_link_libraries(qcat-mascot-probe PRIVATE Qt6::Widgets Qt6::DBus)
    find_package(X11 QUIET)
    if(X11_FOUND)
        target_compile_definitions(qcat-mascot-probe PRIVATE QCAT_PROBE_X11)
        target_link_libraries(qcat-mascot-probe PRIVATE X11::X11)
    endif()
    qt_add_resources(qcat-mascot-probe linux_probe PREFIX "/"
        FILES LinuxProbe.qml LinuxOverlay.qml LinuxTarget.qml)
    find_program(QCAT_PROBE_GNOME_EXTENSIONS gnome-extensions)
    if(QCAT_PROBE_GNOME_EXTENSIONS)
        add_custom_target(mascot_probe_gnome_package
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/gnome"
            COMMAND "${QCAT_PROBE_GNOME_EXTENSIONS}" pack --force
                --extra-source=protocol.mjs --extra-source=service.mjs
                "--out-dir=${CMAKE_CURRENT_BINARY_DIR}/gnome" "${CMAKE_CURRENT_SOURCE_DIR}/gnome"
            COMMENT "Package the GNOME prototype; never install or enable it")
    endif()
endif()
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
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_program(QCAT_PROBE_GJS gjs)
        find_program(QCAT_PROBE_DBUS_RUN dbus-run-session)
        find_program(QCAT_PROBE_NODE node)
        if(QCAT_PROBE_NODE)
            add_test(NAME mascot_companion_protocol COMMAND "${QCAT_PROBE_NODE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/companion-tests/protocol.test.mjs")
        endif()
        if(QCAT_PROBE_GJS AND QCAT_PROBE_DBUS_RUN)
            find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
            qt_add_executable(tst_mascotcompanion companion-tests/tst_companion.cpp mascotcompanion.cpp mascotcompanion.h)
            target_include_directories(tst_mascotcompanion PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
            target_link_libraries(tst_mascotcompanion PRIVATE Qt6::Core Qt6::DBus Qt6::Test)
            target_compile_definitions(tst_mascotcompanion PRIVATE
                GJS_EXECUTABLE="${QCAT_PROBE_GJS}"
                FIXTURE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/companion-tests/service-fixture.mjs")
            add_test(NAME mascot_companion_bus COMMAND "${QCAT_PROBE_DBUS_RUN}" -- $<TARGET_FILE:tst_mascotcompanion>)
            set_tests_properties(mascot_companion_bus PROPERTIES TIMEOUT 45)
        endif()
        add_test(NAME mascot_probe_reject_headless_linux COMMAND ${CMAKE_COMMAND}
            "-DPROBE=$<TARGET_FILE:qcat-mascot-probe>"
            "-DREPORT=${CMAKE_CURRENT_BINARY_DIR}/headless-linux-report.json"
            "-DCHECK_FLAG=--linux-check"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_headless.cmake")
        set_tests_properties(mascot_probe_reject_headless_linux PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
    endif()
    if(WIN32)
        add_test(NAME mascot_probe_reject_headless_windows COMMAND ${CMAKE_COMMAND}
            "-DPROBE=$<TARGET_FILE:qcat-mascot-probe>"
            "-DREPORT=${CMAKE_CURRENT_BINARY_DIR}/headless-windows-report.json"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_headless.cmake")
        set_tests_properties(mascot_probe_reject_headless_windows PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
    endif()
endif()

if(APPLE)
    set_target_properties(qcat-mascot-probe PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER org.qcat.mascotprobe)
    enable_language(OBJCXX)
    target_sources(qcat-mascot-probe PRIVATE macosprobe.h macosprobe.mm)
    target_link_libraries(qcat-mascot-probe PRIVATE "-framework AppKit" "-framework ApplicationServices")
    qt_add_resources(qcat-mascot-probe macos_probe PREFIX "/" FILES MacosProbe.qml)
    if(BUILD_TESTING)
        add_test(NAME mascot_probe_reject_headless_macos COMMAND ${CMAKE_COMMAND}
            "-DPROBE=$<TARGET_FILE:qcat-mascot-probe>"
            "-DREPORT=${CMAKE_CURRENT_BINARY_DIR}/headless-macos-report.json"
            "-DCHECK_FLAG=--macos-check"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_headless.cmake")
        set_tests_properties(mascot_probe_reject_headless_macos PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
    endif()
endif()
