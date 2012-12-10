# Compile RAMDISK module under linux 2.6.x
obj-m += ramdisk_ioctl.o

all:
	make -C /usr/src/kernels/2.6.32-131.12.1.el6.x86_64/ -r SUBDIRS=$$PWD modules

test:
	gcc testingRandom.c -o  testingRandom

clean:
	rm -f ramdisk_ioctl.k* ramdisk_ioctl.m* ramdisk_ioctl.o Module.* modules.*