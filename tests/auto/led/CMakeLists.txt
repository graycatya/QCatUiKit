# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_ledsource tst_ledsource.cpp)
target_link_libraries(tst_ledsource PRIVATE QCatUiKit::QCatUiKit Qt6::Test)
add_test(NAME led_source COMMAND tst_ledsource)

qt_add_executable(tst_ledrender tst_ledrender.cpp)
target_link_libraries(tst_ledrender PRIVATE QCatUiKit::QCatUiKit Qt6::Test Qt6::Quick)
target_compile_definitions(tst_ledrender PRIVATE QCAT_QML_IMPORT_PATH="${QT_QML_OUTPUT_DIRECTORY}")
add_dependencies(tst_ledrender QCatUiKitplugin)
if(QCATUIKIT_BUILD_EXAMPLES)
    target_link_libraries(tst_ledrender PRIVATE qcat-gallery-ui)
    target_compile_definitions(tst_ledrender PRIVATE QCAT_TEST_LED_DEMO)
endif()
# Real GPU tests are opt-in: software/offscreen contract tests cannot validate QRhi.
option(QCATUIKIT_TEST_LED_GPU "Register LED tests requiring a graphics window" OFF)
if(QCATUIKIT_TEST_LED_GPU)
    add_test(NAME led_render COMMAND tst_ledrender)
    set_tests_properties(led_render PROPERTIES TIMEOUT 90)
endif()

qt_add_executable(tst_ledpipeline tst_ledpipeline.cpp)
target_link_libraries(tst_ledpipeline PRIVATE QCatUiKit::QCatUiKit Qt6::Test)
add_test(NAME led_pipeline COMMAND tst_ledpipeline)
set_tests_properties(led_pipeline PROPERTIES TIMEOUT 60)
