
SUBDIRS := $(wildcard */Makefile)
# SUBDIRS is something like "pkt_send/Makefile pkt_recv/Makefile ..."

SUBDIRS := $(dir $(SUBDIRS))
# SUBDIRS is now something like "pkt_send/ pkt_recv/ ..."

all clean fmt fmt-diff: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all clean fmt fmt-diff $(SUBDIRS)
