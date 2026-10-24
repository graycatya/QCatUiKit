# SPDX-License-Identifier: Apache-2.0
qt_add_executable(tst_version tst_version.cpp)
target_link_libraries(tst_version PRIVATE QCatUiKit::QCatUiKit Qt6::Test)
target_compile_definitions(tst_version PRIVATE
    EXPECTED_BASE_VERSION="${PROJECT_VERSION}"
    EXPECTED_VERSION_STRING="${QCATUIKIT_VERSION_STRING}"
    EXPECTED_RELEASE_CHANNEL="${QCATUIKIT_RELEASE_CHANNEL}"
    EXPECTED_EXPERIMENTAL=${QCATUIKIT_IS_EXPERIMENTAL}
    EXPECTED_ABI_VERSION=${QCATUIKIT_ABI_VERSION})
qcat_add_test(NAME version COMMAND tst_version)
set_tests_properties(version PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QSG_RHI_BACKEND=software;QT_QUICK_BACKEND=software")
