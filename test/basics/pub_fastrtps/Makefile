BASE := $(abspath ../../..)
include $(BASE)/make/project_defs.mk

VPATH += ..
INCS += -I..

EXE := test_pub_fastrtps

SRCS_CPP = test_pub_fastrtps.cpp test_config.cpp
SRCS_C = time_util.c

LDLIBS += -lfastrtps

include $(BASE)/make/project_rules.mk
