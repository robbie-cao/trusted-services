#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ts-root-dir := ../../../

global-incdirs-y += $(ts-root-dir)
global-incdirs-y += $(ts-root-dir)/components
global-incdirs-y += $(ts-root-dir)/components/rpc/common/interface
global-incdirs-y += $(ts-root-dir)/components/service/common/include/

srcs-y += $(ts-root-dir)/components/rpc/ffarpc/endpoint/sel1_sp/sel1_sp_ffarpc_call_ep.c
srcs-y += $(ts-root-dir)/components/service/common/provider/service_provider.c

srcs-y += $(ts-root-dir)/components/service/log/provider/log_provider.c

srcs-y += sp_optee_log.c
