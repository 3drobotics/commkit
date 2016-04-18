BASE := $(abspath ../../..)
include $(BASE)/make/project_defs.mk

VPATH += ..
INCS += -I..

EXE := pkt_recv

SRCS_CPP =
SRCS_C = pkt_recv.c receiver.c time_util.c

LDLIBS += -lpthread

include $(BASE)/make/project_rules.mk
