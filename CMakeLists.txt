# Copyright (c) 2022 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19)

# Set default path for Mbed OS library (no force-write for override)
set(MBED_OS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")

# Set default path for mbed_app.json5 (no force-write for override)
set(MBED_APP_JSON_PATH mbed_app.json5 CACHE INTERNAL "")

# Set default path for custom_targets.json5 (no force-write for override)
set(CUSTOM_TARGETS_PATH custom_targets CACHE INTERNAL "")
set(CUSTOM_TARGETS_JSON_PATH ${CUSTOM_TARGETS_PATH}/custom_targets.json5 CACHE INTERNAL "")

# Include Mbed toolchain setup file
include(mbed-os/tools/cmake/mbed_toolchain_setup.cmake)

set(APP_PROJECT NUMAKER_MBED_CE_MULTI_CONNECT_TCP_EXAMPLE)
set(APP_TARGET NuMaker-mbed-ce-multi-connect-tcp-example)

# Set up project name
project(${APP_PROJECT} LANGUAGES C CXX ASM)

# Include Mbed project setup file
include(mbed_project_setup)

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
