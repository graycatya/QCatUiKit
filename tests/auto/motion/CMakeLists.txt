# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_tiltcontroller tst_tiltcontroller.cpp)
target_link_libraries(tst_tiltcontroller PRIVATE QCatUiKit::QCatUiKit Qt6::Test)
qcat_add_test(NAME tilt_controller COMMAND tst_tiltcontroller)
