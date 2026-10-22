# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_docknative tst_docknative.cpp
    ${PROJECT_SOURCE_DIR}/src/uikit/qcatdocknativedrag.cpp
    ${PROJECT_SOURCE_DIR}/src/uikit/qcatdocknativedrag_p.h)
target_link_libraries(tst_docknative PRIVATE Qt6::Quick Qt6::Qml Qt6::Test)
target_include_directories(tst_docknative PRIVATE ${PROJECT_SOURCE_DIR}/src/uikit)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(QCAT_TEST_WAYLAND_CLIENT QUIET IMPORTED_TARGET wayland-client)
        if(TARGET PkgConfig::QCAT_TEST_WAYLAND_CLIENT)
            target_include_directories(tst_docknative PRIVATE ${QCAT_TEST_WAYLAND_CLIENT_INCLUDE_DIRS})
            target_compile_definitions(tst_docknative PRIVATE QCATUIKIT_HAS_WAYLAND_CLIENT)
        endif()
    endif()
endif()
add_test(NAME dock_native COMMAND tst_docknative)
set_tests_properties(dock_native PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
