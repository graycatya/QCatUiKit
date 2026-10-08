# SPDX-License-Identifier: Apache-2.0
qt_add_executable(qcat-gallery main.cpp)
qt_add_qml_module(qcat-gallery URI QCatUiKit.Gallery VERSION 1.0 QML_FILES Main.qml)
target_link_libraries(qcat-gallery PRIVATE QCatUiKit::QCatUiKit Qt6::Quick Qt6::QuickControls2)
