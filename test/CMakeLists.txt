#
# Unit tests
#
# Copyright (C) 2021-2024, HENSOLDT Cyber GmbH
# 
# SPDX-License-Identifier: GPL-2.0-or-later
#
# For commercial licensing, contact: info.cyber@hensoldt.net
#

cmake_minimum_required(VERSION 3.17)

#-------------------------------------------------------------------------------
include("${TEST_MAIN_DIR}/test.cmake")
add_test_target(${PROJECT_NAME}
    SOURCES
        "src/Test_HostEntropy.cpp"
        "src/Test_HostStorage.cpp"
    MOCKS
        ext_mocks
        lib_debug_mocks
)
