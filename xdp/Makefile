# SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)

# Departing from the implicit _user.c scheme
XDP_TARGETS  := xdp_packet_kern
USER_TARGETS := xdp_packet_load

COMMON_DIR := ../common

COMMON_OBJS := $(COMMON_DIR)/common_user_bpf_xdp.o
include $(COMMON_DIR)/common.mk
