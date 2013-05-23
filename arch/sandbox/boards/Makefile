SRC= main.c

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
