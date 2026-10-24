# SPDX-License-Identifier: Apache-2.0

# Clear old evidence before launching: a loader error must not show a previous
# run's passing log. Each CTest name/configuration owns a separate log file.
file(WRITE "${test_log}" "")
if(WIN32)
    # execute_process hides the first GUI window in a directly launched Windows
    # child. Let cmd consume that startup flag so native exposure tests stay real.
    execute_process(COMMAND cmd /d /c call ${test_command} -o "${test_log},txt"
        RESULT_VARIABLE test_result)
else()
    execute_process(COMMAND ${test_command} -o "${test_log},txt"
        RESULT_VARIABLE test_result)
endif()
file(READ "${test_log}" test_output)
if(NOT test_output STREQUAL "")
    message("${test_output}")
endif()
if(NOT test_result STREQUAL "0")
    message(FATAL_ERROR "QtTest exited with ${test_result}. Log: ${test_log}")
endif()
if(test_output STREQUAL "")
    message(FATAL_ERROR "QtTest exited without a text report. Log: ${test_log}")
endif()
