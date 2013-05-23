ARCH            ?= sandbox
CROSS_COMPILE   ?=

MAKEFLAGS = --no-print-directory

INCLUDES = -I$(shell pwd)/include

CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
CFLAGS=-W -Wall -ansi -pedantic -MMD $(INCLUDES)
LDFLAGS=
EXEC=hello
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)
BOARD=

export CC CFLAGS LDFLAGS BOARD SUBDIRS Q MAKEFLAGS

SUBDIRS := drivers/i2c/ \
           arch/$(ARCH)/

.PHONY: $(SUBDIRS)

all: $(SUBDIRS) bmc
	@:

$(SUBDIRS):
	@$(MAKE) -w -C $@ $(MAKECMDGOALS) $(MAKEFLAGS)

bmc:
	@echo [CC] $@
	@$(CC) $(shell find . | grep built-in.o) -o bmc

clean: $(SUBDIRS)
	@rm -rf bmc
