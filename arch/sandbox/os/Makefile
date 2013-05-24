# Read in config
-include $(srctree)/include/config/auto.conf

SRC=

ifdef XXXXX

endif

OBJ= $(SRC:.c=.o)

ifneq ($(OBJ),)
EXEC=built-in.o
endif

.PHONY: modules

all: modules $(EXEC)
	@:

$(EXEC): $(OBJ)
	@echo '   [LD]' $(shell pwd)/$@ | sed -e 's;:\?'$(srctree)/';;' -e 's;'$(srctree)/':\?;;'
	@$(LD) -r $^ -o $@ $(LDFLAGS)

%.o: %.c
	@echo '   [CC]' $(shell pwd)/$@ | sed -e 's;:\?'$(srctree)/';;' -e 's;'$(srctree)/':\?;;'
	@$(CC) -o $@ -c $< $(CFLAGS)

modules:
	@$(MAKE) -w -C $@ $(MAKECMDGOALS) $(MAKEFLAGS)

.PHONY: clean

clean distclean:
	@rm -rf *.o *.d
	@$(MAKE) -w -C modules $(MAKECMDGOALS) $(MAKEFLAGS)

-include $(SRC:.c=.d)
