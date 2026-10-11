# SPDX-License-Identifier: Apache-2.0
add_subdirectory(models)
add_subdirectory(charts)
add_subdirectory(version)
add_subdirectory(qml)

if(QCATUIKIT_BUILD_EXAMPLES)
    add_subdirectory(gallery)
    add_test(NAME gallery_smoke COMMAND qcat-gallery --smoke-test)
    set_tests_properties(gallery_smoke PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
endif()
