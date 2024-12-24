# Copyright (c) 2022 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19)

set(MBED_OS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")

# Set default path for mbed_app.json5 (no force-write for override)
set(MBED_APP_JSON_PATH mbed_app.json5 CACHE INTERNAL "")

# Set default path for custom_targets.json5 (no force-write for override)
set(CUSTOM_TARGETS_PATH custom_targets CACHE INTERNAL "")
set(CUSTOM_TARGETS_JSON_PATH ${CUSTOM_TARGETS_PATH}/custom_targets.json5 CACHE INTERNAL "")

# Load Mbed CE toolchain file and basic build system
include(${MBED_OS_PATH}/tools/cmake/app.cmake)

set(APP_PROJECT NUMAKER_MBED_CE_MULTI_CONNECT_TCP_EXAMPLE)
set(APP_TARGET NuMaker-mbed-ce-multi-connect-tcp-example)

# Set up project name
project(${APP_PROJECT})

# Add Nuvoton CMake list files to CMake default module path
#
# Normally, this is added in ${MBED_OS_PATH}, but ${CUSTOM_TARGETS_PATH}
# is in front and needs it.
list(APPEND CMAKE_MODULE_PATH
    ${MBED_OS_PATH}/targets/TARGET_NUVOTON/scripts
)

# Add Mbed OS library
add_subdirectory(${MBED_OS_PATH})

add_executable(${APP_TARGET})

target_sources(${APP_TARGET}
    PRIVATE
        main.cpp
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        mbed-os
        mbed-netsocket-api
        $<$<IN_LIST:COMPONENT_ESPRESSIF_ESP8266=1,${MBED_TARGET_DEFINITIONS}>:mbed-wifi>
)

# Must call this for each target to set up bin file creation, code upload, etc
mbed_set_post_build(${APP_TARGET})

# Make sure this is the last line of the top-level build script
mbed_finalize_build()
