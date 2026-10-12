# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_gallery tst_gallery.cpp)
target_link_libraries(tst_gallery PRIVATE qcat-gallery-uiplugin QCatUiKit::QCatUiKit Qt6::QuickTest Qt6::QuickControls2)
target_compile_definitions(tst_gallery PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
add_test(NAME gallery_contracts COMMAND tst_gallery -import ${QT_QML_OUTPUT_DIRECTORY})
set_tests_properties(gallery_contracts PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")

qt_add_executable(tst_gallerychartdata tst_gallerychartdata.cpp)
target_link_libraries(tst_gallerychartdata PRIVATE qcat-gallery-ui QCatUiKit::QCatUiKit Qt6::Test)
add_test(NAME gallery_chart_data COMMAND tst_gallerychartdata)
set_tests_properties(gallery_chart_data PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software" TIMEOUT 180)

qt_add_executable(tst_gallerydatademo tst_gallerydatademo.cpp)
target_link_libraries(tst_gallerydatademo PRIVATE qcat-gallery-ui QCatUiKit::QCatUiKit Qt6::Test)
add_test(NAME gallery_data_demo COMMAND tst_gallerydatademo)
set_tests_properties(gallery_data_demo PROPERTIES TIMEOUT 60)
