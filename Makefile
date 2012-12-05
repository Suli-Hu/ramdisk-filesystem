# Compile RAMDISK module under linux 2.6.x
obj-m += ramdisk_ioctl.o

all:
	make -C /home/student/linux-2.6.32-131.12.1.el6.x86_64 -r SUBDIRS=$$PWD modules