# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_mascot tst_mascot.cpp ${PROJECT_SOURCE_DIR}/src/uikit/mascot/qcatmascotpackage.cpp)
target_link_libraries(tst_mascot PRIVATE QCatUiKit::QCatUiKit Qt6::Quick Qt6::Test)
if(WIN32)
    add_executable(mascot-input-target windowsinputtarget.cpp)
    target_link_libraries(mascot-input-target PRIVATE user32 gdi32)
    target_link_libraries(tst_mascot PRIVATE user32 gdi32)
    add_dependencies(tst_mascot mascot-input-target)
endif()
target_include_directories(tst_mascot PRIVATE ${PROJECT_SOURCE_DIR}/src/uikit/mascot)
qt_add_resources(tst_mascot fixtures PREFIX "/mascot-test" FILES data/mascot.json data/atlas.png)
qt_add_resources(tst_mascot root_fixtures PREFIX "/" BASE "${CMAKE_CURRENT_SOURCE_DIR}/data"
    FILES data/mascot.json data/atlas.png)
target_compile_definitions(tst_mascot PRIVATE
    QCAT_MASCOT_SAMPLE="${PROJECT_SOURCE_DIR}/tests/manual/mascot-player/catgray"
    QCAT_MASCOT_LOTTIE_SAMPLE="${PROJECT_SOURCE_DIR}/tests/manual/mascot-player/catgray-lottie")
qcat_add_test(NAME mascot_contracts COMMAND tst_mascot)
set_tests_properties(mascot_contracts PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")

option(QCATUIKIT_TEST_MASCOT_GPU "Register mascot Lottie QRhi checks" OFF)
if(QCATUIKIT_TEST_MASCOT_GPU AND QCATUIKIT_ENABLE_THORVG)
    qcat_add_test(NAME mascot_render COMMAND tst_mascot lottieRendering catgrayLottieRendering asynchronousLottieHandoff actionCacheAndLatency configurableActionCache)
    set_tests_properties(mascot_render PROPERTIES ENVIRONMENT
        "QCAT_MASCOT_GPU=1;QCAT_CATGRAY_LOTTIE_PACKAGE=${PROJECT_SOURCE_DIR}/tests/manual/mascot-player/catgray-lottie")
endif()

if(APPLE)
    enable_language(OBJCXX)
    qt_add_executable(mascot-macos-input-target MACOSX_BUNDLE macosinputtarget.mm)
    set_target_properties(mascot-macos-input-target PROPERTIES
        OBJCXX_STANDARD 17 MACOSX_BUNDLE_GUI_IDENTIFIER org.qcatuikit.mascot-input-target)
    target_link_libraries(mascot-macos-input-target PRIVATE Qt6::Gui "-framework AppKit")
    qt_add_executable(tst_mascotmacosinput MACOSX_BUNDLE macosinputtest.mm)
    set_target_properties(tst_mascotmacosinput PROPERTIES
        OBJCXX_STANDARD 17 MACOSX_BUNDLE_GUI_IDENTIFIER org.qcatuikit.mascot-input-tests)
    target_link_libraries(tst_mascotmacosinput PRIVATE QCatUiKit::QCatUiKit Qt6::Quick Qt6::Test
        "-framework AppKit" "-framework ApplicationServices")
    target_compile_definitions(tst_mascotmacosinput PRIVATE
        QCAT_MACOS_INPUT_TARGET="$<TARGET_FILE:mascot-macos-input-target>")
    qt_add_resources(tst_mascotmacosinput macos_fixtures PREFIX "/mascot-macos-test"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}/data" FILES data/mascot.json data/atlas.png)
    add_dependencies(tst_mascotmacosinput mascot-macos-input-target)
    add_test(NAME mascot_macos_reject_headless COMMAND ${CMAKE_COMMAND}
        "-DPROBE=$<TARGET_FILE:tst_mascotmacosinput>"
        "-DREPORT=${CMAKE_CURRENT_BINARY_DIR}/headless-input-report.json"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_macos_headless.cmake")
    set_tests_properties(mascot_macos_reject_headless PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
endif()
