obj-m += i2c-openbmc-stub.o
KVERSION = $(shell uname -r)

all:
	@echo '   [CC]' $(shell pwd)/i2c-openbmc-stub.o | sed -e 's;:\?'$(srctree)/';;' -e 's;'$(srctree)/':\?;;'
	@export ARCH=x86 ;\
	make -s -C /lib/modules/$(KVERSION)/build M=$(shell pwd) modules
clean distclean:
	@export ARCH=x86 ;\
	make -s $(MAKEFLAGS) -C /lib/modules/$(KVERSION)/build M=$(shell pwd) clean