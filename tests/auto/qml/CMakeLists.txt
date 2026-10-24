# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_qml tst_qml.cpp)
target_link_libraries(tst_qml PRIVATE Qt6::QuickTest Qt6::QuickControls2)
target_compile_definitions(tst_qml PRIVATE
    QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
add_dependencies(tst_qml QCatUiKitplugin)
qcat_add_test(NAME qml_contracts COMMAND tst_qml -import ${QT_QML_OUTPUT_DIRECTORY})
set_tests_properties(qml_contracts PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QSG_RHI_BACKEND=software;QT_QUICK_BACKEND=software")
qcat_add_test(NAME qml_busy_timing COMMAND tst_qml
    -input ${CMAKE_CURRENT_SOURCE_DIR}/data/tst_busyindicator.qml
    -import ${QT_QML_OUTPUT_DIRECTORY})
set_tests_properties(qml_busy_timing PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QSG_RENDER_LOOP=basic;QCATUIKIT_TEST_FAST_ANIMATION_DRIVER=1")
