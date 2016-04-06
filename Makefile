BASE := $(abspath ../..)
include $(BASE)/make/project_defs.mk

PROJ := commkit

SRCS_CPP := \
	src/node.cpp \
	src/nodeimpl.cpp \
	src/publisher.cpp \
	src/publisherimpl.cpp \
	src/subscriber.cpp \
	src/subscriberimpl.cpp

# ensure public API is exported, see visibility.h
CPPFLAGS += -DCOMMKIT_DLL -DCOMMKIT_DLL_EXPORTS

LIBCOMMKIT := $(INSTALL)/lib/libcommkit.$(DYLIB_SUFFIX)

LDLIBS += -lfastrtps

all: $(LIBCOMMKIT)

$(LIBCOMMKIT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

COMMKIT_INCS_DIR := $(INSTALL)/include/commkit
$(COMMKIT_INCS_DIR):
	mkdir -p $(COMMKIT_INCS_DIR)

COMMKIT_INCS := $(wildcard include/commkit/*.h)

COMMKIT_INCS := $(addprefix $(INSTALL)/,$(COMMKIT_INCS))

$(COMMKIT_INCS): $(COMMKIT_INCS_DIR) $(wildcard include/commkit/*.h)
	cp $(wildcard include/commkit/*.h) $(COMMKIT_INCS_DIR)

$(BUILD)/%.o: %.c $(COMMKIT_INCS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.o: %.cpp $(COMMKIT_INCS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

fmt:
	@python $(BASE)/tools/clang-format-run.py --apply

fmt-diff:
	@python $(BASE)/tools/clang-format-run.py

# extend
CLEAN_DIRS += $(COMMKIT_INCS_DIR)
# override
CLEAN_FILES = $(LIBCOMMKIT)

-include $(OBJS:.o=.d)

include $(BASE)/make/common_rules.mk
