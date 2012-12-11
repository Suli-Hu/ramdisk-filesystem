# Compile RAMDISK module under linux 2.6.x

ifeq ($(MAKECMDGOALS), kernel)
	obj-m += ramdisk_ioctl.o
endif

kernel:
	make -C /usr/src/kernels/2.6.32-131.12.1.el6.x86_64/ -r SUBDIRS=$$PWD modules

debug:
	gcc ramdisk_ioctl.c  -DDEBUG=1 -o ram -ggdb

clean:
	rm -f ramdisk_ioctl.k* ramdisk_ioctl.m* ramdisk_ioctl.o Module.* modules.* 
	rm -rf ram ram.dSYM