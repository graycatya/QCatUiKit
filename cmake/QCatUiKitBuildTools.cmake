# SPDX-License-Identifier: Apache-2.0
include_guard(GLOBAL)
include("${CMAKE_CURRENT_LIST_DIR}/QCatUiKitFetchArchive.cmake")
get_filename_component(qcat_dependency_owner "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
option(QCATUIKIT_ENABLE_THORVG "Enable the vector UI QRhi backend" ON)
option(QCATUIKIT_BUILD_THORVG "Build ThorVG with Meson" ${QCATUIKIT_ENABLE_THORVG})
set(QCATUIKIT_THIRDPARTY_DIR "${qcat_dependency_owner}/3rdparty" CACHE PATH "Managed dependency sources")
set(QCATUIKIT_THIRDPARTY_DOWNLOAD_DIR "${qcat_dependency_owner}/3rdparty-download" CACHE PATH "Verified dependency archive cache")
set(QCATUIKIT_BUILD_TOOLS_DIR "${qcat_dependency_owner}/3rdparty-tools" CACHE PATH "Managed build tools")
set(QCATUIKIT_PYTHON_EXECUTABLE "" CACHE FILEPATH "Optional Python interpreter override")
option(QCATUIKIT_BOOTSTRAP_PYTHON "Use managed Python even if Python is installed" OFF)
foreach(path_var QCATUIKIT_THIRDPARTY_DIR QCATUIKIT_THIRDPARTY_DOWNLOAD_DIR QCATUIKIT_BUILD_TOOLS_DIR)
    get_filename_component(${path_var} "${${path_var}}" ABSOLUTE BASE_DIR "${PROJECT_SOURCE_DIR}")
endforeach()

if(MSVC OR QCATUIKIT_BUILD_THORVG OR QCATUIKIT_ENABLE_THORVG)
    if(NOT QCATUIKIT_PYTHON_EXECUTABLE AND NOT QCATUIKIT_BOOTSTRAP_PYTHON)
        find_package(Python3 3.7 QUIET COMPONENTS Interpreter)
        if(Python3_Interpreter_FOUND)
            set(QCATUIKIT_PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
        endif()
    endif()
    if(NOT QCATUIKIT_PYTHON_EXECUTABLE)
        if(CMAKE_HOST_WIN32 AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(AMD64|amd64|x86_64)$")
            set(python_root "${QCATUIKIT_BUILD_TOOLS_DIR}/python-3.12.10-amd64")
            qcat_fetch_archive(
                "https://www.python.org/ftp/python/3.12.10/python-3.12.10-embed-amd64.zip"
                "4acbed6dd1c744b0376e3b1cf57ce906f9dc9e95e68824584c8099a63025a3c3"
                "${QCATUIKIT_THIRDPARTY_DOWNLOAD_DIR}/python-3.12.10-embed-amd64.zip"
                "${python_root}" FALSE)
            set(QCATUIKIT_PYTHON_EXECUTABLE "${python_root}/python.exe")
        else()
            message(FATAL_ERROR "Python >= 3.7 is required on this host; set QCATUIKIT_PYTHON_EXECUTABLE")
        endif()
    endif()
    execute_process(COMMAND "${QCATUIKIT_PYTHON_EXECUTABLE}" -c "import sys; assert sys.version_info >= (3,7)"
        RESULT_VARIABLE python_result)
    if(NOT python_result EQUAL 0)
        message(FATAL_ERROR "QCATUIKIT_PYTHON_EXECUTABLE must be a working Python >= 3.7")
    endif()
endif()

if(QCATUIKIT_BUILD_THORVG OR QCATUIKIT_ENABLE_THORVG)
    set(QCATUIKIT_MESON_EXECUTABLE "" CACHE FILEPATH "Optional Meson executable override")
    if(QCATUIKIT_MESON_EXECUTABLE)
        set(QCATUIKIT_MESON_COMMAND "${QCATUIKIT_MESON_EXECUTABLE}")
    else()
        set(meson_root "${QCATUIKIT_BUILD_TOOLS_DIR}/meson-1.9.2")
        qcat_fetch_archive(
            "https://files.pythonhosted.org/packages/18/ca/0e3c232fb20f0707bbddf7bb639882a3dba8684bf2f9f4eced266775fcbc/meson-1.9.2.tar.gz"
            "3499b59bb23982496e01e57b4103ac2f826f9c3a3f59e507a0a832487fe55e3d"
            "${QCATUIKIT_THIRDPARTY_DOWNLOAD_DIR}/meson-1.9.2.tar.gz"
            "${meson_root}" TRUE)
        set(QCATUIKIT_MESON_COMMAND "${QCATUIKIT_PYTHON_EXECUTABLE}" "${meson_root}/meson.py")
    endif()
    execute_process(COMMAND ${QCATUIKIT_MESON_COMMAND} --version
        RESULT_VARIABLE meson_result OUTPUT_VARIABLE meson_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT meson_result EQUAL 0 OR meson_version VERSION_LESS 1.3)
        message(FATAL_ERROR "Meson >= 1.3 is required")
    endif()
    message(STATUS "ThorVG build tool: Meson ${meson_version}")
endif()
