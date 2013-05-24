# Read in config
-include $(srctree)/include/config/auto.conf

SRC=

ifdef CONFIG_DRIVER_TEST
SRC += test_i2c.c
endif

ifdef CONFIG_DRIVER_TEST1
SRC += test1_i2c.c
endif

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
