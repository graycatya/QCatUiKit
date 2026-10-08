# SPDX-License-Identifier: Apache-2.0
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/SurfaceImpl.qml PROPERTIES QT_QML_INTERNAL_TYPE TRUE)
set(qcat_public_headers
    ${PROJECT_SOURCE_DIR}/include/QCatUiKit/Export.h
    ${PROJECT_SOURCE_DIR}/include/QCatUiKit/Version.h
    ${PROJECT_SOURCE_DIR}/include/QCatUiKit/ThemePalette.h
    ${PROJECT_SOURCE_DIR}/include/QCatUiKit/ChatModel.h
    ${PROJECT_SOURCE_DIR}/include/QCatUiKit/TableModel.h)
qt_add_qml_module(QCatUiKit
    SHARED
    URI QCatUiKit
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_DIRECTORY ${QT_QML_OUTPUT_DIRECTORY}/QCatUiKit
    SOURCES ${qcat_public_headers} ThemePalette.cpp ChatModel.cpp TableModel.cpp
        Version.cpp LibraryInfo.h LibraryInfo.cpp
    QML_FILES
        qml/CatAlert.qml
        qml/CatBusyIndicator.qml
        qml/CatButton.qml
        qml/CatCalendar.qml
        qml/CatChatInput.qml
        qml/CatChatView.qml
        qml/CatCircularProgress.qml
        qml/CatConicalGradient.qml
        qml/CatDragHandle.qml
        qml/CatDrawer.qml
        qml/CatEffect.qml
        qml/CatFlipView.qml
        qml/CatFpsCounter.qml
        qml/CatImage.qml
        qml/CatImageMotion.qml
        qml/CatIpAddressField.qml
        qml/CatLabel.qml
        qml/CatMarquee.qml
        qml/CatPagination.qml
        qml/CatPane.qml
        qml/CatPopup.qml
        qml/CatProgressBar.qml
        qml/CatResizeHandle.qml
        qml/CatTableView.qml
        qml/CatTextField.qml
        qml/CatToolTip.qml
        qml/CatTypewriter.qml
        qml/SurfaceImpl.qml
        qml/Theme.qml
)
add_library(QCatUiKit::QCatUiKit ALIAS QCatUiKit)
target_compile_definitions(QCatUiKit PRIVATE QCATUIKIT_BUILD)
target_include_directories(QCatUiKit PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE ${PROJECT_SOURCE_DIR}/include/QCatUiKit
        ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR}/generated)
target_link_libraries(QCatUiKit PUBLIC Qt6::Core Qt6::Gui Qt6::Qml PRIVATE Qt6::Quick Qt6::QuickControls2)
set_target_properties(QCatUiKit PROPERTIES
    VERSION ${PROJECT_VERSION} SOVERSION ${QCATUIKIT_ABI_VERSION}
    CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
if(APPLE)
    set_target_properties(QCatUiKitplugin PROPERTIES INSTALL_RPATH "@loader_path/../..")
elseif(UNIX)
    set_target_properties(QCatUiKitplugin PROPERTIES INSTALL_RPATH "$ORIGIN/../..")
endif()
install(TARGETS QCatUiKit EXPORT QCatUiKitTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS QCatUiKitplugin
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/qml/QCatUiKit
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qml/QCatUiKit)
install(FILES ${QT_QML_OUTPUT_DIRECTORY}/QCatUiKit/qmldir
    ${QT_QML_OUTPUT_DIRECTORY}/QCatUiKit/QCatUiKit.qmltypes
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/qml/QCatUiKit)
install(DIRECTORY qml/ DESTINATION ${CMAKE_INSTALL_LIBDIR}/qml/QCatUiKit/qml FILES_MATCHING PATTERN "*.qml")
