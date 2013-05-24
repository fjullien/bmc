obj-m += i2c-openbmc-stub.o
KVERSION = $(shell uname -r)

all:
	export ARCH=x86 ;\
	make -C /lib/modules/$(KVERSION)/build M=$(shell pwd) modules
clean distclean:
	export ARCH=x86 ;\
	make -C /lib/modules/$(KVERSION)/build M=$(shell pwd) clean