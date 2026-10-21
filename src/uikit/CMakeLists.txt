# SPDX-License-Identifier: Apache-2.0
set_source_files_properties(qml/DockFloatingWindow.qml PROPERTIES QT_QML_INTERNAL_TYPE TRUE)
set_source_files_properties(qml/DockTabButton.qml qml/DockDropOverlay.qml qml/DockDragArea.qml qml/DockDragPreview.qml PROPERTIES QT_QML_INTERNAL_TYPE TRUE)
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/SurfaceImpl.qml PROPERTIES QT_QML_INTERNAL_TYPE TRUE)
set_source_files_properties(qml/MouseScrollImpl.qml PROPERTIES QT_QML_INTERNAL_TYPE TRUE)
set_source_files_properties(qml/SliderTrackImpl.qml qml/SliderHandleImpl.qml PROPERTIES QT_QML_INTERNAL_TYPE TRUE)
# Public headers live beside their implementations, as in Qt module sources.
# Generate the build-tree include facade; install the real headers and portable
# forwarding headers rather than the absolute build-tree forwarding headers.
set(qcat_public_headers
    qcatdockcontroller.h
    qcattiltcontroller.h
    qcatuikitglobal.h
    qcatuikitversion.h
    qcatthemepalette.h
    qcatchatmodel.h
    qcattablemodel.h
    qcatchartcolorscale.h qcatchartbubble.h qcatchartgrid.h qcatchartcontour.h
    qcatchartohlc.h qcatchartboxplot.h
    qcatwaterfall.h qcatchartfit.h qcatcharthistogram.h qcatchartstack.h
    qcatchartseries3d.h qcatchartscene3d.h qcatchartaxis.h qcatchartseries.h qcatchartitem.h qcatledsource.h qcatledpreview.h qcatledlayoutmodel.h qcatlightingframe.h qcatledeffectregistry.h qcatledstorage.h qcatleddeck.h qcatledmixer.h qcatledoutput.h qcatledproject.h)
set(qcat_compat_headers DockController.h TiltController.h Export.h Version.h ThemePalette.h ChatModel.h TableModel.h ChartColorScale.h ChartBubble.h ChartGrid.h ChartContour.h ChartOhlc.h ChartBoxPlot.h ChartWaterfall.h ChartFit.h ChartHistogram.h ChartStack.h ChartSeries3D.h ChartScene3D.h ChartAxis.h ChartSeries.h ChartItem.h LedSource.h LedPreview.h LedLayoutModel.h LightingFrame.h LedEffectRegistry.h LedStorage.h LedDeck.h LedMixer.h LedOutput.h LedProject.h)
set(qcat_class_headers DockController TiltController Export Version ThemePalette ChatModel TableModel ChartColorScale ChartBubble ChartGrid ChartContour ChartOhlc ChartBoxPlot ChartWaterfall ChartFit ChartHistogram ChartStack ChartSeries3D ChartScene3D ChartAxis ChartSeries ChartItem LedSource LedPreview LedLayoutModel LightingFrame LedEffectRegistry LedStorage LedDeck LedMixer LedOutput LedProject)
set(qcat_include_dir "${QCatUiKit_BINARY_DIR}/include/QCatUiKit")
set(qcat_forwarding_dir "${CMAKE_CURRENT_BINARY_DIR}/forwarding")
file(MAKE_DIRECTORY "${qcat_include_dir}" "${qcat_forwarding_dir}")
foreach(qcat_header qcat_compat_header qcat_class_header IN ZIP_LISTS
        qcat_public_headers qcat_compat_headers qcat_class_headers)
    file(WRITE "${qcat_include_dir}/${qcat_header}"
        "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${qcat_header}\"\n")
    foreach(qcat_forwarding_header IN ITEMS "${qcat_compat_header}" "${qcat_class_header}")
        file(WRITE "${qcat_forwarding_dir}/${qcat_forwarding_header}"
            "#include \"${qcat_header}\"\n")
        configure_file("${qcat_forwarding_dir}/${qcat_forwarding_header}"
            "${qcat_include_dir}/${qcat_forwarding_header}" COPYONLY)
        install(FILES "${qcat_forwarding_dir}/${qcat_forwarding_header}"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QCatUiKit")
    endforeach()
endforeach()
install(FILES ${qcat_public_headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QCatUiKit")
configure_file(qcatuikitversiondata_p.h.in qcatuikitversiondata_p.h @ONLY)

qt_add_qml_module(QCatUiKit
    SHARED
    URI QCatUiKit
    VERSION 1.0
    # All QML files share qml/. Keep their implicit directory import local so
    # internal helpers remain visible; QTP0004's extra qmldir redirects it to
    # the module root, where those helpers cannot be resolved in Qt 6.10.
    NO_GENERATE_EXTRA_QMLDIRS
    RESOURCE_PREFIX /qt/qml
    OUTPUT_DIRECTORY ${QT_QML_OUTPUT_DIRECTORY}/QCatUiKit
    SOURCES ${qcat_public_headers}
        qcatdockcontroller.cpp
        qcattiltcontroller.cpp
        qcatthemepalette.cpp qcatchatmodel.cpp qcattablemodel.cpp
        qcatchartmapping.cpp qcatchartcontour.cpp
        qcatchartfinancial.cpp
        qcatwaterfall.cpp qcatchartfit.cpp qcatcharthistogram.cpp qcatchartstack.cpp qcatchartstatistics_p.h
        qcatchartaxis.cpp qcatchartseries.cpp qcatchartitem.cpp qcatchartmath_p.h qcatchartcurves_p.h qcatchartlabels_p.h qcatchartlod_p.h qcatchartdelegates_p.h qcatchartpicking_p.h
        qcatchartrhiitem.cpp qcatchartrhiitem_p.h
        qcatchartseries3d.cpp qcatchartscene3d.cpp qcatchart3d_p.h qcatchartrhiitem3d.cpp
        qcatledsource.cpp qcatledsource_p.h qcatledpreview.cpp qcatledlayoutmodel.cpp qcatlightingframe.cpp qcatledeffectregistry.cpp qcatledstorage.cpp qcatleddeck.cpp qcatledmixer.cpp qcatledoutput.cpp qcatledproject.cpp
        qcattreemodel_p.h qcattreemodel.cpp
        qcatuikitversion.cpp qcatuikitinfo_p.h qcatuikitinfo.cpp
    QML_FILES
        qml/CatDockWorkspace.qml qml/CatDockPanel.qml qml/DockGeometry.js qml/DockFloatingWindow.qml
        qml/DockTabButton.qml qml/DockDropOverlay.qml qml/DockDragArea.qml qml/DockDragPreview.qml
        qml/CatAttitudeIndicator.qml qml/CatTiltControlPanel.qml
        qml/CatAlert.qml
        qml/CatBusyIndicator.qml
        qml/CatButton.qml
        qml/CatCheckBox.qml
        qml/CatRadioButton.qml
        qml/CatSwitch.qml
        qml/CatSlider.qml
        qml/CatRangeSlider.qml
        qml/CatSliderField.qml
        qml/SliderMath.js
        qml/SliderTrackImpl.qml
        qml/SliderHandleImpl.qml
        qml/CatComboBox.qml
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
        qml/LedPreview.qml
        qml/LedPreviewSettings.qml qml/LedLayoutSettings.qml qml/LedStorageBrowser.qml
        qml/LedDeck.qml qml/LedMixer.qml qml/LedOutputSettings.qml
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
        qml/CatAccordion.qml
        qml/CatAutoSuggestBox.qml
        qml/CatCoachMark.qml
        qml/CatColorPicker.qml
        qml/CatCommandBar.qml
        qml/CatCommandBarFlyout.qml
        qml/CatCompoundButton.qml
        qml/CatContentDialog.qml
        qml/CatDialog.qml
        qml/CatDivider.qml
        qml/CatDropDownButton.qml
        qml/CatExpander.qml
        qml/CatField.qml
        qml/CatFileDropZone.qml
        qml/CatFlyout.qml
        qml/CatFontIcon.qml
        qml/CatHyperlinkButton.qml
        qml/CatMenu.qml
        qml/CatMenuBar.qml
        qml/CatMenuBarItem.qml
        qml/CatMenuItem.qml
        qml/CatMenuSeparator.qml
        qml/CatMultiSelectComboBox.qml
        qml/CatNumberBox.qml
        qml/CatPasswordField.qml
        qml/CatRatingControl.qml
        qml/CatRepeatButton.qml
        qml/CatSpinBox.qml
        qml/CatSplitButton.qml
        qml/CatTeachingTip.qml
        qml/CatTextArea.qml
        qml/CatToggleButton.qml
        qml/CatToggleSplitButton.qml
        qml/CatAnnotatedScrollBar.qml
        qml/CatAvatar.qml
        qml/CatBreadcrumb.qml
        qml/CatCalendarDatePicker.qml
        qml/CatCarousel.qml
        qml/CatDatePicker.qml
        qml/CatFileListView.qml
        qml/CatFlowView.qml
        qml/CatGridView.qml
        qml/CatInfoBadge.qml
        qml/CatListView.qml
        qml/CatNavigationView.qml
        qml/CatParticleBackdrop.qml
        qml/CatPipsPager.qml
        qml/CatPivot.qml
        qml/CatProgressRing.qml
        qml/CatScrollBar.qml
        qml/CatScrollView.qml
        qml/CatSelectorBar.qml
        qml/CatShimmer.qml
        qml/CatSpatialItem.qml
        qml/CatSpatialView.qml
        qml/SpatialTransform.js
        qml/CatSplashScreen.qml
        qml/CatSplitView.qml
        qml/CatStackView.qml
        qml/CatStepper.qml
        qml/CatTabView.qml
        qml/CatTimePicker.qml
        qml/CatTimeline.qml
        qml/CatTitleBar.qml
        qml/CatToast.qml
        qml/CatTreeView.qml
        qml/CatWindow.qml
        qml/SurfaceImpl.qml
        qml/MouseScrollImpl.qml
        qml/Theme.qml
)
add_library(QCatUiKit::QCatUiKit ALIAS QCatUiKit)
target_compile_definitions(QCatUiKit PRIVATE QCATUIKIT_BUILD)
target_include_directories(QCatUiKit PUBLIC
    $<BUILD_INTERFACE:${QCatUiKit_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(QCatUiKit PUBLIC Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick PRIVATE Qt6::QuickControls2 Qt6::GuiPrivate)
qt_add_shaders(QCatUiKit ledpreview_shaders
    PREFIX /QCatUiKit
    FILES shaders/ledpreview.vert shaders/ledpreview.frag)
qt_add_shaders(QCatUiKit chart_shaders
    PREFIX /QCatUiKit
    FILES shaders/chart3d.vert shaders/chart3d.frag shaders/chart.vert shaders/chart.frag shaders/chartinstance.vert shaders/chartinstance.frag)
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
install(DIRECTORY qml/ DESTINATION ${CMAKE_INSTALL_LIBDIR}/qml/QCatUiKit/qml FILES_MATCHING PATTERN "*.qml" PATTERN "*.js")
