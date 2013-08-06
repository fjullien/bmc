# Read in config
-include $(srctree)/include/config/auto.conf

SRC=

SRC += cmd_core.c

OBJ= $(SRC:.c=.o)

ifneq ($(OBJ),)
EXEC=built-in.o
endif

all: $(EXEC)
	@:

$(EXEC): $(OBJ)
	@echo '   [LD]' $(shell pwd)/$@ | sed -e 's;:\?'$(srctree)/';;' -e 's;'$(srctree)/':\?;;'
	@$(LD) -r $^ -o $@ $(LDFLAGS)

%.o: %.c
	@echo '   [CC]' $(shell pwd)/$@ | sed -e 's;:\?'$(srctree)/';;' -e 's;'$(srctree)/':\?;;'
	@$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean

clean distclean:
	@rm -rf *.o *.d

-include $(SRC:.c=.d)
