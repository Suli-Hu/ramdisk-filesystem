ramdisk-filesystem
==================

Custom file system created for the linux 2.6 kernel.  Final project for CS552. 

Consists of a kernel loadable module with an interface into a library, RAMFileLib.  The
module initializes, a set amount of memory in the kernel, and creates a data structure
to manage blocks of data.

The RAMFileLib interfaces with the module using IOCTL calls.  Each process which uses
the library creates their own process file handle block, although this was not the focus of
this project.

The file system provides support for creating files, creating directories, reading files from
a directory, writing data to files, and reading data from files.  All of the functions provide
clear error conditions.

Running
==================

To work with the filesystem, run the remake.sh script as root.  This creates and builds the filesystem, loads it into memory, and also builds the test_file for testing.  If you want to create your own test file, or work with the filesystem in any other way, in any file that
uses RAMFileLib, the following extern calls must be included:

	extern int proc;
	extern int currentFdNum;
	extern std::vector<FD_entry> fd_Table;

This is required for the library to work correctly.  We intended to remove this dependency but it was beyond the time we had available.

To test the filesystem, simply run

	./user

and the test script will run and verify that the filesystem is working correctly.  Different tests can be turned on or off within test_file.cpp

Remarks
==================

When looking at the commit history, the reason for the absurd number of commits in a short time span is because when developing the module, most debugging was done in mac, but when it was necessary to load and test, github and git was used to transfer all of the updates onto a linux virtual machine.  We apologize if this make it hard to look at how the module changed over development, though you may be entertained by some of the commit messages!


