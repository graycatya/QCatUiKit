# SPDX-License-Identifier: Apache-2.0
include_guard(GLOBAL)
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    return()
endif()
if(NOT QCATUIKIT_PYTHON_EXECUTABLE)
    include("${CMAKE_CURRENT_LIST_DIR}/QCatUiKitBuildTools.cmake")
endif()
# Never put an undecoded localized byte string into CMake's UTF-8 Ninja files.
# The launcher normalizes actual compiler output and preserves existing launchers.
set(QCATUIKIT_MSVC_LAUNCHER "${CMAKE_CURRENT_LIST_DIR}/msvc_compiler_launcher.py")
list(PREPEND CMAKE_CXX_COMPILER_LAUNCHER
    "${QCATUIKIT_PYTHON_EXECUTABLE}" "${QCATUIKIT_MSVC_LAUNCHER}" --)
set(CMAKE_CL_SHOWINCLUDES_PREFIX "Note: including file: ")
