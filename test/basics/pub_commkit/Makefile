BASE := $(abspath ../../..)
include $(BASE)/make/project_defs.mk

VPATH += ..
INCS += -I..

CPPFLAGS += -DCOMMKIT_NO_CAPNP

EXE := test_pub_commkit

SRCS_CPP = test_pub_commkit.cpp test_config.cpp
SRCS_C = time_util.c

# -lfastrtps needed for call to set verbosity
LDLIBS += -lcommkit -lfastrtps

include $(BASE)/make/project_rules.mk
