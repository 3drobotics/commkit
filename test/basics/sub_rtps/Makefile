BASE := $(abspath ../../..)
include $(BASE)/make/project_defs.mk

VPATH += ..
INCS += -I..

EXE := test_sub_rtps

SRCS_CPP = test_sub_rtps.cpp test_config.cpp
SRCS_C = time_util.c

LDLIBS += -lfastrtps

include $(BASE)/make/project_rules.mk
