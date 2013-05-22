obj-m += i2c-openbmc-stub.o
KVERSION = $(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
install:
	sudo insmod i2c-openbmc-stub.ko chip_addr=0x23
remove:
	sudo rmmod i2c-openbmc-stub.ko
