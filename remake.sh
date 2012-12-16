#!/bin/bash

# Simple script which remakes everythin after a pull
rmmod ramdisk_ioctl
make clean
make user
make all
insmod ramdisk_ioctl.ko