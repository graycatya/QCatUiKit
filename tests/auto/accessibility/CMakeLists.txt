# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_accessibility tst_accessibility.cpp)
target_link_libraries(tst_accessibility PRIVATE QCatUiKit::QCatUiKit Qt6::Test Qt6::Quick)
target_compile_definitions(tst_accessibility PRIVATE
    QCAT_QML_IMPORT_PATH="${QT_QML_OUTPUT_DIRECTORY}")
add_dependencies(tst_accessibility QCatUiKitplugin)

# Query Qt's in-process accessibility interfaces on native window platforms.
# This does not test the OS bridge or an external assistive technology client.
option(QCATUIKIT_TEST_ACCESSIBILITY
    "Register native-window Qt accessibility interface tests" OFF)
if(QCATUIKIT_TEST_ACCESSIBILITY)
    qcat_add_test(NAME accessibility_interfaces COMMAND tst_accessibility)
    set_tests_properties(accessibility_interfaces PROPERTIES
        TIMEOUT 90
        SKIP_REGULAR_EXPRESSION "QCAT_ACCESSIBILITY_UNSUPPORTED_PLATFORM:")
endif()
