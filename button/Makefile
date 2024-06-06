# Usage: make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- KDIR=...

obj-m += kb-driver.o

KDIR = /lib/modules/$(shell uname -r)/build

CC = $(CROSS_COMPILE)gcc

all: kb-driver driver_ctl

kb-driver:
ifndef CROSS_COMPILE
	make -C $(KDIR)  M=$(shell pwd) modules
else
	make -C $(KDIR)  M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
endif

driver_ctl: driver_ctl.c
	$(CC) -o driver_ctl driver_ctl.c

kb-test: kb-test.c Keyboard.c
	$(CC) $(CFLAGS) -pthread -o $@ $^

clean:
	rm -f kb-test
	make -C $(KDIR)  M=$(shell pwd) clean
