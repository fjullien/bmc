VERSION= 0.1

ARCH            ?= sandbox
CROSS_COMPILE   ?=

MAKEFLAGS = --no-print-directory

srctree		:= $(CURDIR)

INCLUDES       := -I$(srctree)/include \
                  -I$(srctree)/include/freeipmi/spec \
                  -I$(srctree)/arch/$(ARCH)/include \
                  -include $(srctree)/include/generated/autoconf.h \
                  -include $(srctree)/include/kconfig.h

CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
CFLAGS=-W -Wall -MMD $(INCLUDES)
LDFLAGS=
EXEC=hello
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)
BOARD=

LD_LIBRARY_PATH=$(srctree)/scripts/kconfig/lib

MCONF=./scripts/kconfig/bin/kconfig-mconf
CONF=./scripts/kconfig/bin/kconfig-conf

# Read in config
-include $(srctree)/include/config/auto.conf

#include $(srctree)/arch/$(ARCH)/Makefile

SRCARCH 	:= $(ARCH)

export CC CFLAGS LDFLAGS BOARD MAKEFLAGS SRCARCH ARCH VERSION srctree

SUBDIRS := drivers/i2c/ \
           common \
           arch/$(ARCH)/

.PHONY: $(SUBDIRS) bmc

all: brdconfig $(SUBDIRS) bmc
	@:

brdconfig:
	@cp $(srctree)/arch/$(ARCH)/boards/$(BOARD)/config.h $(srctree)/include/config.h

$(SUBDIRS):
	@$(MAKE) -w -C $@ $(MAKECMDGOALS) $(MAKEFLAGS)

bmc:
	@echo '   [CC]' $(shell pwd)/$@ | sed -e 's;:\?'$(srctree)/';;' -e 's;'$(srctree)/':\?;;'
	@$(CC) -E -Wp,-MD,$(srctree)/arch/$(ARCH)/boards/.barebox.lds.d  -nostdinc $(CFLAGS) -C -P -D__ASSEMBLY__ -o arch/sandbox/boards/bmc.lds arch/sandbox/boards/bmc.lds.S
	@$(CC) -o bmc -Wl,-T,$(srctree)/arch/$(ARCH)/boards/bmc.lds $(shell find . | grep built-in.o)

clean: $(SUBDIRS)
	@rm -rf bmc

distclean: clean
	@rm -rf include/config include/generated

.PHONY: menuconfig

menuconfig: _menuconfig config

_menuconfig:

ifneq ($(MCONF),)
	export LD_LIBRARY_PATH; \
	$(MCONF) Kconfig
endif

.PHONY: config

config:
ifneq ($(CONF),)
	@mkdir -p include/
	@mkdir -p include/config include/generated
	@export LD_LIBRARY_PATH; \
	$(CONF) --silentoldconfig Kconfig
endif

savedefconfig:

ifneq ($(CONF),)
	@export LD_LIBRARY_PATH; \
	$(CONF) --savedefconfig defconfig Kconfig
endif
