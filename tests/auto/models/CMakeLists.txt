# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_models tst_models.cpp)
target_link_libraries(tst_models PRIVATE QCatUiKit::QCatUiKit Qt6::Test)
qcat_add_test(NAME models COMMAND tst_models)
set_tests_properties(models PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QSG_RHI_BACKEND=software;QT_QUICK_BACKEND=software")
