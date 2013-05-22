SRC= test_i2c.c \
     test1_i2c.c

OBJ= $(SRC:.c=.o)
EXEC=built-in.o

all: $(EXEC)
	@:

$(EXEC): $(OBJ)
	@echo [LD] $@
	@$(LD) -r $^ -o $@ $(LDFLAGS)

%.o: %.c
	@echo [CC] $@
	@$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean

clean:
	@rm -rf *.o
