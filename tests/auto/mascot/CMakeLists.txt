# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_mascot tst_mascot.cpp ${PROJECT_SOURCE_DIR}/src/uikit/mascot/qcatmascotpackage.cpp)
target_link_libraries(tst_mascot PRIVATE QCatUiKit::QCatUiKit Qt6::Quick Qt6::Test)
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
