# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_gallery tst_gallery.cpp)
target_link_libraries(tst_gallery PRIVATE qcat-gallery-uiplugin QCatUiKit::QCatUiKit Qt6::QuickTest Qt6::QuickControls2)
target_compile_definitions(tst_gallery PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
add_test(NAME gallery_contracts COMMAND tst_gallery -import ${QT_QML_OUTPUT_DIRECTORY})
set_tests_properties(gallery_contracts PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")