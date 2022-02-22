#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include($ENV{TS_ROOT}/tools/cmake/compiler/config_iface.cmake REQUIRED)

# Set compiler warning level for the root build context. External components
# are responsible for setting their own warning level.
if(DEFINED TS_ROOT)
    string(APPEND CMAKE_C_FLAGS_INIT " -Wall")
    string(APPEND CMAKE_CXX_FLAGS_INIT " -Wall")
endif()
