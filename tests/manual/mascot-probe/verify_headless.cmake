# SPDX-License-Identifier: Apache-2.0
# A headless run must replace stale successful evidence with Pending.
file(WRITE "${REPORT}" "{\"prototypeAcceptance\":\"Passed\"}")
if(NOT DEFINED CHECK_FLAG)
    set(CHECK_FLAG --windows-check)
endif()
execute_process(COMMAND "${PROBE}" "${CHECK_FLAG}" "${REPORT}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT result EQUAL 2)
    message(FATAL_ERROR "Expected headless refusal (2), got ${result}: ${error}")
endif()
file(READ "${REPORT}" report)
string(JSON prototype GET "${report}" prototypeAcceptance)
string(JSON acceptance GET "${report}" acceptance)
string(JSON platform GET "${report}" qpaPlatform)
string(JSON transparency GET "${report}" capabilities transparency status)
if(NOT prototype STREQUAL "Pending" OR NOT acceptance STREQUAL "Pending"
        OR NOT platform STREQUAL "offscreen" OR NOT transparency STREQUAL "Unknown")
    message(FATAL_ERROR "Invalid headless acceptance evidence: ${report}")
endif()
