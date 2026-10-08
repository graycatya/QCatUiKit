# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_models tst_models.cpp)
target_link_libraries(tst_models PRIVATE QCatUiKit::QCatUiKit Qt6::Test)
add_test(NAME models COMMAND tst_models)
qt_add_executable(tst_version tst_version.cpp)
target_link_libraries(tst_version PRIVATE QCatUiKit::QCatUiKit Qt6::Test)
target_compile_definitions(tst_version PRIVATE
    EXPECTED_BASE_VERSION="${PROJECT_VERSION}"
    EXPECTED_VERSION_STRING="${QCATUIKIT_VERSION_STRING}"
    EXPECTED_RELEASE_CHANNEL="${QCATUIKIT_RELEASE_CHANNEL}"
    EXPECTED_EXPERIMENTAL=${QCATUIKIT_IS_EXPERIMENTAL}
    EXPECTED_ABI_VERSION=${QCATUIKIT_ABI_VERSION})
add_test(NAME version COMMAND tst_version)
qt_add_executable(tst_qml qml_main.cpp)
target_link_libraries(tst_qml PRIVATE Qt6::QuickTest Qt6::QuickControls2)
target_compile_definitions(tst_qml PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")
add_dependencies(tst_qml QCatUiKitplugin)
add_test(NAME qml_contracts COMMAND tst_qml -import ${QT_QML_OUTPUT_DIRECTORY})
set_tests_properties(models version qml_contracts PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QSG_RHI_BACKEND=software;QT_QUICK_BACKEND=software")

if(QCATUIKIT_BUILD_EXAMPLES)
    add_test(NAME gallery_smoke COMMAND qcat-gallery --smoke-test)
    set_tests_properties(gallery_smoke PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
endif()
