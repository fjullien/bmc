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
EXEC=built-in.o

all: $(EXEC)
	@:

$(EXEC): $(OBJ)
	@echo '   [LD]' $@
	@$(LD) -r $^ -o $@ $(LDFLAGS)

%.o: %.c
	@echo '   [CC]' $@
	@$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean

clean:
	@rm -rf *.o *.d

-include $(SRC:.c=.d)
