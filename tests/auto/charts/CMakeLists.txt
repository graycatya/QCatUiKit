# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_charts tst_charts.cpp)
target_link_libraries(tst_charts PRIVATE QCatUiKit::QCatUiKit Qt6::Test)
target_include_directories(tst_charts PRIVATE ${QCatUiKit_SOURCE_DIR}/src/uikit)
target_compile_definitions(tst_charts PRIVATE QCAT_QML_IMPORT_PATH="${QT_QML_OUTPUT_DIRECTORY}")
add_dependencies(tst_charts QCatUiKitplugin)
add_test(NAME charts COMMAND tst_charts)
set_tests_properties(charts PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software" TIMEOUT 90)
