CROSS_COMPILE = arm-linux-gnueabihf-

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS = -g -Wall -Wa,-mimplicit-it=thumb -nostdlib -fno-builtin -c -O2 -I$(shell pwd)/include
LDFLAGS = -lgcc -L /home/zhangxu/study/imx6ull/cross_compiler/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/4.9.4
# LDFLAGS =
OBJCOPYFLAGS =

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := kernel.bin


obj-y += init/
obj-y += kernel/
obj-y += drivers/
obj-y += lib/


all : 
	make -C ./ -f $(TOPDIR)/Makefile.build

	$(LD) -T kernel.lds -o kernel.elf built-in.o $(LDFLAGS)
	$(OBJCOPY) $(OBJCOPYFLAGS) -O binary kernel.elf kernel.bin
	$(OBJDUMP) -D -m arm kernel.elf > kernel.dis


clean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(shell find -name "*.bin")
	rm -f $(shell find -name "*.elf")
	rm -f $(shell find -name "*.dis")

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	