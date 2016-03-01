
BASE := $(abspath ../..)
include $(BASE)/make/common_defs.mk
include $(BASE)/make/project_defs.mk

PROJ := commkit

SRCS_CPP := \
	src/node.cpp \
	src/nodeimpl.cpp \
	src/publisher.cpp \
	src/publisherimpl.cpp \
	src/subscriber.cpp \
	src/subscriberimpl.cpp

INCS += -Iinclude

LIBCOMMKIT := $(INSTALL)/lib/libcommkit.$(DYLIB_SUFFIX)

LDLIBS += -lfastrtps

all: $(LIBCOMMKIT)

$(LIBCOMMKIT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

fmt:
	@python $(BASE)/tools/clang-format-run.py --apply

fmt-diff:
	@python $(BASE)/tools/clang-format-run.py

clean:
	rm -rf $(BUILD) $(INSTALL) $(BOOST)

.PHONY: default all clean

-include $(OBJS:.o=.d)
