
SUBDIRS := $(wildcard */Makefile)
# SUBDIRS is something like "pub_commkit/Makefile pub_fastrtps/Makefile ..."

SUBDIRS := $(dir $(SUBDIRS))
# SUBDIRS is now something like "pub_commkit/ pub_fastrtps/ ..."

all clean fmt fmt-diff: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all clean fmt fmt-diff $(SUBDIRS)
