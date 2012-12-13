#include <stdio.h>
#include <stdlib.h>
#include "RAMFileLib.h"
#include "structs.h"
#include <list>

using namespace std;

// Declaring global variables
list<FD_entry> fd_Table;


#ifdef KERNERLREADY
int fd;
fd = open ("/proc/ramdisk_ioctl", O_RDONLY);
#endif
static int currentFdNum = 0;
// currentFdNum=0;

int rd_creat(char *pathname) {
	RAM_path rampath;
	rampath.name = pathname;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_CREATE, &rampath);	
#endif  	

  	// kr_creat(rampath);

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
	RAM_file file;
	file.name = pathname;
#ifdef KERNERLREADY
  	ioctl (fd, RAM_OPEN, &file);	
#endif

  	// If the file open failed, return an error
  	if (file.ret<0)
  		return file.ret;

	// If this file is not currently open, create new entry
  	FD_entry entry;

	// If there is already an index node relating to this file, do not create new entry
  	int fileAlreadyOpen;
  	fileAlreadyOpen=checkIfFileExists(file.indexNode);
  	entry.fd = fdFromIndexNode(file.indexNode);

	if (fileAlreadyOpen==-1) {
		entry.indexNode = file.indexNode;
		entry.offset = 0;	// Default file pointer to the start of file
		entry.fileSize=-1;	// -1 indicates the file does not have size yet
		entry.fd = currentFdNum++;	// Set file descriptor
		entry.fileSize = file.fileSize;
		fd_Table.push_back(entry);
		return entry.fd;
	}

	return entry.fd;
}

int rd_read(int fd, char *address, int num_bytes) {
	RAM_accessFile file;
	file.fd = fd;
	file.address=address;
	file.numBytes = num_bytes;
	file.indexNode = indexNodeFromfd(fd);

	// Make sure the file exists
	if (checkIfFileExists(fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

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
	file.indexNode = indexNodeFromfd(fd);

	// Make sure the file exists
	if (checkIfFileExists(fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

#ifdef KERNERLREADY
  	ioctl (fd, RAM_WRITE, &file);	
#endif  	
	return file.ret;
}

int rd_lseek(int fd, int offset) {
	RAM_file file;
	file.offset=offset;

	// Make sure the file exists
	if (checkIfFileExists(fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

	// If the offset the user specifies is greater than the file size
	// move the pointer to end of file
	FD_entry entry;
	entry = getEntryFromFd(fd);
	if (offset>entry.fileSize) {
		file.offset = entry.fileSize;
	}


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

	// Make sure the file exists
	if (checkIfFileExists(fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

#ifdef KERNERLREADY
  	ioctl (fd, RAM_READDIR, &file);	
#endif  	
	return file.ret;
}

/******************* HELPER FUNCTION ********************/
int checkIfFileExists(int fd) {
	list<FD_entry>::iterator it;
	for (it = fd_Table.begin() ; it != fd_Table.end() ; it++) 
	{
		if (it->fd==fd) 
			return 1;
		
	}	
	return -1;
}

FD_entry getEntryFromFd(int fd) {
	list<FD_entry>::iterator it;
	FD_entry entry;

	for (it = fd_Table.begin() ; it != fd_Table.end() ; it++) 
	{
		if (it->fd==fd) 
			return *it;
		
	}	
	return entry;	
}

int fdFromIndexNode(int indexNode) {
	list<FD_entry>::iterator it;
	for (it = fd_Table.begin() ; it != fd_Table.end() ; it++) 
	{
		if (it->indexNode==indexNode) 
			return it->fd;
		
	}	
	return -1;	
}

int indexNodeFromfd(int fd) {
	list<FD_entry>::iterator it;
	for (it = fd_Table.begin() ; it != fd_Table.end() ; it++) 
	{
		if (it->fd==fd) 
			return it->indexNode;
		
	}	
	return -1;	
}

int main () {
	printf("Hello world\n");
	rd_creat("/file.txt");
}