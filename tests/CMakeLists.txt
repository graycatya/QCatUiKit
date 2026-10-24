# SPDX-License-Identifier: Apache-2.0

# The Windows offscreen plugin does not use the native font database. Supply real
# fonts instead of letting layout tests measure Qt's missing-glyph rectangles.
set(_qcat_test_font_dir "$ENV{QT_QPA_FONTDIR}")
if(WIN32 AND NOT _qcat_test_font_dir)
    set(_qcat_test_font_dir "$ENV{SystemRoot}/Fonts")
endif()
set(QCATUIKIT_TEST_FONT_DIR "${_qcat_test_font_dir}" CACHE PATH
    "Font directory for offscreen tests (required by the Windows offscreen plugin)")

function(qcat_add_test)
    cmake_parse_arguments(test "" "NAME" "COMMAND" ${ARGN})
    list(GET test_COMMAND 0 executable)
    if(executable MATCHES "^tst_" AND TARGET ${executable})
        # Some Windows Qt builds send the default logger to the debugger even
        # with a console subsystem. Relay an explicit text log through CTest.
        set(command "[==[$<TARGET_FILE:${executable}>]==]")
        list(REMOVE_AT test_COMMAND 0)
        foreach(argument IN LISTS test_COMMAND)
            string(APPEND command " [==[${argument}]==]")
        endforeach()
        set(runner "${CMAKE_CURRENT_BINARY_DIR}/${test_NAME}-$<CONFIG>.cmake")
        file(GENERATE OUTPUT "${runner}" CONTENT
            "set(test_command ${command})\nset(test_log [==[${CMAKE_CURRENT_BINARY_DIR}/${test_NAME}-$<CONFIG>.txt]==])\ninclude([==[${PROJECT_SOURCE_DIR}/tests/runqttest.cmake]==])\n")
        add_test(NAME ${test_NAME} COMMAND ${CMAKE_COMMAND} -P "${runner}")
    else()
        add_test(${ARGN})
    endif()
    # Keep a stalled suite bounded. Individual suites may specify tighter limits.
    set_tests_properties(${test_NAME} PROPERTIES TIMEOUT 300)
    if(QCATUIKIT_TEST_FONT_DIR)
        set_property(TEST ${test_NAME} APPEND PROPERTY ENVIRONMENT_MODIFICATION
            "QT_QPA_FONTDIR=set:${QCATUIKIT_TEST_FONT_DIR}")
    endif()
    if(WIN32)
        if(executable MATCHES "^tst_" AND TARGET ${executable})
            set_target_properties(${executable} PROPERTIES WIN32_EXECUTABLE FALSE)
        endif()
        set_property(TEST ${test_NAME} APPEND PROPERTY ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:QCatUiKit>")
    endif()
    if(test_NAME MATCHES "_render$")
        # Native graphics windows must not steal exposure/input from one another.
        set_tests_properties(${test_NAME} PROPERTIES RESOURCE_LOCK qcat_gpu_window)
    endif()
endfunction()

add_subdirectory(auto)
