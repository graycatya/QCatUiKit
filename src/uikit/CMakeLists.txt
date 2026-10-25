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
    docking/qcatdockcontroller.h
    motion/qcattiltcontroller.h
    core/qcatuikitglobal.h
    core/qcatuikitversion.h
    theme/qcatthemepalette.h
    models/qcatchatmodel.h
    models/qcattablemodel.h
    charts/qcatchartcolorscale.h charts/qcatchartbubble.h charts/qcatchartgrid.h charts/qcatchartcontour.h
    charts/qcatchartohlc.h charts/qcatchartboxplot.h
    charts/qcatwaterfall.h charts/qcatchartfit.h charts/qcatcharthistogram.h charts/qcatchartstack.h
    charts/qcatchartseries3d.h charts/qcatchartscene3d.h charts/qcatchartaxis.h charts/qcatchartseries.h charts/qcatchartitem.h led/qcatledsource.h led/qcatledpreview.h led/qcatledlayoutmodel.h led/qcatlightingframe.h led/qcatledeffectregistry.h led/qcatledstorage.h led/qcatleddeck.h led/qcatledmixer.h led/qcatledoutput.h led/qcatledproject.h)
set(qcat_compat_headers DockController.h TiltController.h Export.h Version.h ThemePalette.h ChatModel.h TableModel.h ChartColorScale.h ChartBubble.h ChartGrid.h ChartContour.h ChartOhlc.h ChartBoxPlot.h ChartWaterfall.h ChartFit.h ChartHistogram.h ChartStack.h ChartSeries3D.h ChartScene3D.h ChartAxis.h ChartSeries.h ChartItem.h LedSource.h LedPreview.h LedLayoutModel.h LightingFrame.h LedEffectRegistry.h LedStorage.h LedDeck.h LedMixer.h LedOutput.h LedProject.h)
set(qcat_class_headers DockController TiltController Export Version ThemePalette ChatModel TableModel ChartColorScale ChartBubble ChartGrid ChartContour ChartOhlc ChartBoxPlot ChartWaterfall ChartFit ChartHistogram ChartStack ChartSeries3D ChartScene3D ChartAxis ChartSeries ChartItem LedSource LedPreview LedLayoutModel LightingFrame LedEffectRegistry LedStorage LedDeck LedMixer LedOutput LedProject)
set(qcat_include_dir "${QCatUiKit_BINARY_DIR}/include/QCatUiKit")
set(qcat_forwarding_dir "${CMAKE_CURRENT_BINARY_DIR}/forwarding")
file(MAKE_DIRECTORY "${qcat_include_dir}" "${qcat_forwarding_dir}")
foreach(qcat_header qcat_compat_header qcat_class_header IN ZIP_LISTS
        qcat_public_headers qcat_compat_headers qcat_class_headers)
    get_filename_component(qcat_header_name "${qcat_header}" NAME)
    file(WRITE "${qcat_include_dir}/${qcat_header_name}"
        "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${qcat_header}\"\n")
    foreach(qcat_forwarding_header IN ITEMS "${qcat_compat_header}" "${qcat_class_header}")
        file(WRITE "${qcat_forwarding_dir}/${qcat_forwarding_header}"
            "#include \"${qcat_header_name}\"\n")
        configure_file("${qcat_forwarding_dir}/${qcat_forwarding_header}"
            "${qcat_include_dir}/${qcat_forwarding_header}" COPYONLY)
        install(FILES "${qcat_forwarding_dir}/${qcat_forwarding_header}"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QCatUiKit")
    endforeach()
endforeach()
install(FILES ${qcat_public_headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QCatUiKit")
configure_file(core/qcatuikitversiondata_p.h.in qcatuikitversiondata_p.h @ONLY)

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
        docking/qcatdockcontroller.cpp
        docking/qcatdocknativedrag.cpp docking/qcatdocknativedrag_p.h
        motion/qcattiltcontroller.cpp
        theme/qcatthemepalette.cpp models/qcatchatmodel.cpp models/qcattablemodel.cpp
        charts/qcatchartmapping.cpp charts/qcatchartcontour.cpp
        charts/qcatchartfinancial.cpp
        charts/qcatwaterfall.cpp charts/qcatchartfit.cpp charts/qcatcharthistogram.cpp charts/qcatchartstack.cpp charts/qcatchartstatistics_p.h
        charts/qcatchartaxis.cpp charts/qcatchartseries.cpp charts/qcatchartitem.cpp charts/qcatchartmath_p.h charts/qcatchartcurves_p.h charts/qcatchartlabels_p.h charts/qcatchartlod_p.h charts/qcatchartdelegates_p.h charts/qcatchartpicking_p.h
        charts/qcatchartrhiitem.cpp charts/qcatchartrhiitem_p.h
        charts/qcatchartseries3d.cpp charts/qcatchartscene3d.cpp charts/qcatchart3d_p.h charts/qcatchartrhiitem3d.cpp
        led/qcatledsource.cpp led/qcatledsource_p.h led/qcatledpreview.cpp led/qcatledlayoutmodel.cpp led/qcatlightingframe.cpp led/qcatledeffectregistry.cpp led/qcatledstorage.cpp led/qcatleddeck.cpp led/qcatledmixer.cpp led/qcatledoutput.cpp led/qcatledproject.cpp
        models/qcattreemodel_p.h models/qcattreemodel.cpp
        core/qcatuikitversion.cpp core/qcatuikitinfo_p.h core/qcatuikitinfo.cpp
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
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/theme
        ${CMAKE_CURRENT_SOURCE_DIR}/models
        ${CMAKE_CURRENT_SOURCE_DIR}/docking
        ${CMAKE_CURRENT_SOURCE_DIR}/motion
        ${CMAKE_CURRENT_SOURCE_DIR}/charts
        ${CMAKE_CURRENT_SOURCE_DIR}/led
        ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(QCatUiKit PUBLIC Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick PRIVATE Qt6::QuickControls2 Qt6::GuiPrivate)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(QCAT_WAYLAND_CLIENT QUIET IMPORTED_TARGET wayland-client)
        if(TARGET PkgConfig::QCAT_WAYLAND_CLIENT)
            target_include_directories(QCatUiKit PRIVATE ${QCAT_WAYLAND_CLIENT_INCLUDE_DIRS})
            target_compile_definitions(QCatUiKit PRIVATE QCATUIKIT_HAS_WAYLAND_CLIENT)
        endif()
    endif()
endif()
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
