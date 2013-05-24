SUBDIRS := boards \
           os

.PHONY: $(SUBDIRS)

all clean distclean: $(SUBDIRS)
	@:

$(SUBDIRS):
	@$(MAKE) -w -C $@ $(MAKECMDGOALS) $(MAKEFLAGS)
