#include <stdio.h>
#include <stdlib.h>
#include "RAMFileLib.h"
#include "structs.h"
#include <vector>

using namespace std;

// Declaring global variables
std::vector<FD_entry*> fd_Table;


#ifdef KERNERLREADY
int fd;
fd = open ("/proc/ramdisk_ioctl", O_RDONLY);
#endif


int rd_creat(char *pathname) {
	RAM_path rampath;
	rampath.name = pathname;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_CREATE, &rampath);	
#endif  	
	return rampath.ret;
}

int rd_mkdir(char *pathname) {
	RAM_path rampath;
	rampath.name = pathname;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_MKDIR, &rampath);	
#endif  	
	return rampath.ret;
}

int rd_open(char *pathname) {
	RAM_path rampath;
	rampath.name = pathname;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_OPEN, &rampath);	
#endif  	
	return rampath.ret;
}

int rd_read(int fd, char *address, int num_bytes) {
	RAM_accessFile file;
	file.fd = fd;
	file.address=address;
	file.numBytes = num_bytes;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_READ, &file);	
#endif  	
	return file.ret;
}

int rd_write(int fd, char *address, int num_bytes) {
	RAM_accessFile file;
	file.fd = fd;
	file.address=address;
	file.numBytes = num_bytes;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_WRITE, &file);	
#endif  	
	return file.ret;
}

int rd_lseek(int fd, int offset) {
	RAM_file file;
	file.fd = fd;
	file.offset=offset;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_LSEEK, &file);	
#endif  		
	return file.ret;
}

int rd_unlink(char *pathname) {
	RAM_path rampath;
	rampath.name = pathname;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_UNLINK, &rampath);	
#endif  	
	return rampath.ret;
}

int rd_readdir(int fd, char *address) {
	RAM_accessFile file;
	file.fd = fd;
	file.address=address;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_READDIR, &file);	
#endif  	
	return file.ret;
}

int main () {


}