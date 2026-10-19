# SPDX-License-Identifier: Apache-2.0
find_package(Qt6 6.8 REQUIRED COMPONENTS LinguistTools)
qt_add_library(qcat-gallery-ui STATIC)
add_dependencies(qcat-gallery-ui QCatUiKitplugin)
set_source_files_properties(GalleryCatalog.qml GalleryChartCatalog.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(qcat-gallery-ui
    URI QCatUiKit.Gallery
    VERSION 1.0
    DEPENDENCIES QCatUiKit
    RESOURCE_PREFIX /qt/qml
    OUTPUT_DIRECTORY ${QT_QML_OUTPUT_DIRECTORY}/QCatUiKit/Gallery
    SOURCES gallerylanguage.h gallerylanguage.cpp gallerychartdata.h gallerychartdata.cpp
        gallerydatademo.h gallerydatademo.cpp
        galleryleddemo.h galleryleddemo.cpp
    QML_FILES
        Main.qml GalleryCatalog.qml GalleryChartCatalog.qml GalleryChartExamples.js GalleryChartBrowser.qml GalleryIcon.qml GalleryLogo.qml GalleryComboBox.qml
        GalleryNavigation.qml GalleryEntryCard.qml GalleryHome.qml
        GalleryChartExport.qml GalleryChartRangeControls.qml GalleryChartAxisRange.qml
        GalleryAttitudePreview.qml GalleryTiltPreview.qml
        GallerySliderPreview.qml
        GalleryWaterfallPreview.qml GalleryChartM4Preview.qml GalleryChartFitPreview.qml
        GalleryChartStackPreview.qml GalleryHistogramPreview.qml
        GallerySettingsPage.qml GalleryComponentPage.qml GalleryPreview.qml GalleryExtendedPreview.qml GalleryChartPreview.qml
        GalleryDataPreview.qml GalleryDataStressPanel.qml GalleryChartAdvancedPreview.qml GalleryChartAxesPreview.qml GalleryChart3DPreview.qml GalleryChartRadialPreview.qml GalleryChartAreaOptions.qml GalleryChartStrokeOptions.qml GalleryChartInteractionOptions.qml GalleryChartDelegateOptions.qml GalleryChartMotionOptions.qml
        GalleryLedPreview.qml
    RESOURCES assets/preview-art.svg assets/chart-weave.png assets/chart-marker.png)
set(qcat_gallery_brand_dir "${QCatUiKit_SOURCE_DIR}/src/uikit/doc/images/brand/qcat-uikit")
set(qcat_gallery_brand_files)
foreach(qcat_gallery_icon_size IN ITEMS 16 24 32 48 64 128 256 512)
    list(APPEND qcat_gallery_brand_files
        "${qcat_gallery_brand_dir}/png/qcat-uikit-${qcat_gallery_icon_size}.png")
endforeach()
qt_add_resources(qcat-gallery-ui qcat_gallery_brand
    PREFIX "/qt/qml/QCatUiKit/Gallery/assets/brand"
    BASE "${qcat_gallery_brand_dir}"
    FILES ${qcat_gallery_brand_files})
target_include_directories(qcat-gallery-ui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(qcat-gallery-ui PRIVATE QCatUiKit::QCatUiKit Qt6::Quick Qt6::QuickControls2)

qt_add_translations(qcat-gallery-ui
    SOURCE_TARGETS qcat-gallery-ui QCatUiKit
    TS_FILES translations/qcatgallery_en.ts translations/qcatgallery_zh_CN.ts
    RESOURCE_PREFIX /i18n
    LUPDATE_TARGET qcat-gallery_lupdate
    LRELEASE_TARGET qcat-gallery_lrelease
    LUPDATE_OPTIONS -no-obsolete)

qt_add_executable(qcat-gallery main.cpp)
if(WIN32)
    enable_language(RC)
    configure_file(gallery.rc.in "${CMAKE_CURRENT_BINARY_DIR}/qcat-gallery.rc" @ONLY)
    target_sources(qcat-gallery PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/qcat-gallery.rc")
    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/qcat-gallery.rc" PROPERTIES
        OBJECT_DEPENDS "${qcat_gallery_brand_dir}/qcat-uikit.ico")
endif()
target_link_libraries(qcat-gallery PRIVATE qcat-gallery-uiplugin QCatUiKit::QCatUiKit Qt6::Quick Qt6::QuickControls2)
# Resolve the build-tree module directory relative to the executable, including
# the extra configuration directory used by multi-configuration generators.
set(qcat_gallery_runtime_dir "${CMAKE_CURRENT_BINARY_DIR}")
if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(qcat_gallery_runtime_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()
if(CMAKE_CONFIGURATION_TYPES)
    string(APPEND qcat_gallery_runtime_dir "/config")
endif()
file(RELATIVE_PATH qcat_gallery_qml_relative_path
    "${qcat_gallery_runtime_dir}" "${QT_QML_OUTPUT_DIRECTORY}")
target_compile_definitions(qcat-gallery PRIVATE
    QCAT_GALLERY_QML_RELATIVE_PATH="${qcat_gallery_qml_relative_path}")
add_custom_target(qcat-gallery_qmllint DEPENDS qcat-gallery-ui_qmllint)
