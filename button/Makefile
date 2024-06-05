obj-m += driver.o
KDIR = /home/rick_eos/Desktop/my-linux-6.1
all:
	make -C $(KDIR) M=$(shell pwd) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules
	gcc driver_ctl.c -o driver_ctl