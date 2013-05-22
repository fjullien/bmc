BOARD= boards

.PHONY: $(BOARD)

all clean: $(BOARD)
	@:

$(BOARD):
	@$(MAKE) -w -C $@ $(MAKECMDGOALS) $(MAKEFLAGS)
