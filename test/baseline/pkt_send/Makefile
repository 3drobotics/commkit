BASE := $(abspath ../../..)
include $(BASE)/make/project_defs.mk

VPATH += ..
INCS += -I..

EXE := pkt_send

SRCS_CPP =
SRCS_C = pkt_send.c sender.c time_util.c

LDLIBS += -lpthread

include $(BASE)/make/project_rules.mk
