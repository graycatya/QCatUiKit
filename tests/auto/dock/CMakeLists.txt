# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_dockcontroller tst_dockcontroller.cpp)
target_link_libraries(tst_dockcontroller PRIVATE QCatUiKit::QCatUiKit Qt6::Test)
qcat_add_test(NAME dock_controller COMMAND tst_dockcontroller)
