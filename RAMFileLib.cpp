#include <stdio.h>
#include <stdlib.h>
#include "RAMFileLib.h"
#include "structs.h"
#include <vector>

using namespace std;

// Declaring global variables
vector<FD_entry> fd_Table;


#if 1
int fd;
#endif
static int currentFdNum = 0;
// currentFdNum=0;

int rd_creat(char *pathname) {
	struct RAM_path rampath;
	rampath.name = pathname;
#if 1
	printf("IM HERE\n");
  	ioctl (fd, RAM_CREATE, &rampath);
	printf("IM HERE\n");	
#endif  	

	return rampath.ret;
}

int rd_mkdir(char *pathname) {
	struct RAM_path rampath;
	rampath.name = pathname;
#if 1
  	ioctl (fd, RAM_MKDIR, &rampath);
#endif  	
	return rampath.ret;
}

int rd_open(char *pathname) {
	struct RAM_file file;
	file.name = pathname;
#if 1
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
		entry.fd = currentFdNum++;	// Set file descriptor
		entry.fileSize = file.fileSize;
		entry.dirIndex = 0;	// Initially the file pointer is 0 (first file in dir)
		fd_Table.push_back(entry);
		return entry.fd;
	}

	return entry.fd;
}

void kr_close(int fd) {

	// Closing a file simply means removing a file from the FD table
	deleteFileFromFDTable(fd);
}

int rd_read(int fd, char *address, int num_bytes) {
	struct RAM_accessFile file;
	file.fd = fd;
	file.address=address;
	file.numBytes = num_bytes;
	file.indexNode = indexNodeFromfd(fd);

	// Make sure the file exists
	if (checkIfFileExists(fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

#if 1
  	ioctl (fd, RAM_READ, &file);	
#endif

  	// Update the offset after reading the file
	FD_entry *entry;
	entry = getEntryFromFd(fd);
	entry->offset = file.offset;

	return file.ret;
}

int rd_write(int fd, char *address, int num_bytes) {
	struct RAM_accessFile file;
	file.fd = fd;
	file.address=address;
	file.numBytes = num_bytes;
	file.indexNode = indexNodeFromfd(fd);

	// Make sure the file exists
	if (checkIfFileExists(fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

#if 1
  	ioctl (fd, RAM_WRITE, &file);	
#endif  	

  	// Update the offset after reading the file
	FD_entry *entry;
	entry = getEntryFromFd(fd);
	entry->offset = file.offset;

	return file.ret;
}

int rd_lseek(int fd, int offset) {

	// Make sure the file exists
	if (checkIfFileExists(fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

	// If the offset the user specifies is greater than the file size
	// move the pointer to end of file
	struct FD_entry *entry;
	entry = getEntryFromFd(fd);
	if (offset>entry->fileSize) {
		entry->offset= entry->fileSize;
	}

	if (entry->offset<0)
		return -1;
	else
		return 1;

}

int rd_unlink(char *pathname) {
	struct RAM_path rampath;
	rampath.name = pathname;
#if 1
  	ioctl (fd, RAM_UNLINK, &rampath);	
#endif  	
	return rampath.ret;
}

int rd_readdir(int fd, char *address) {
	struct RAM_accessFile file;
	file.fd = fd;
	file.address=address;

	struct FD_entry *entry;
	entry = getEntryFromFd(fd);

	file.indexNode = entry->indexNode;
	file.dirIndex = entry->dirIndex;

	// Make sure the file exists
	if (checkIfFileExists(fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

#if 1
  	ioctl (fd, RAM_READDIR, &file);	
#endif

  	// If the number of files pointer have exceeded total num of files, reset it
  	entry->numOfFiles = file.numOfFiles;
  	entry->dirIndex = entry->dirIndex+1;
  	if (entry->dirIndex==(entry->numOfFiles-1))
  		entry->dirIndex=0;

	return file.ret;
}

/******************* HELPER FUNCTION ********************/
int checkIfFileExists(int fd) {
	vector<FD_entry>::iterator it;
	for (it = fd_Table.begin() ; it != fd_Table.end() ; it++) 
	{
		if (it->fd==fd) 
			return 1;
		
	}	
	return -1;
}

int fdFromIndexNode(int indexNode) {
	vector<FD_entry>::iterator it;
	for (it = fd_Table.begin() ; it != fd_Table.end() ; it++) 
	{
		if (it->indexNode==indexNode) 
			return it->fd;
		
	}	
	return -1;	
}

int indexNodeFromfd(int fd) {
	vector<FD_entry>::iterator it;
	for (it = fd_Table.begin() ; it != fd_Table.end() ; it++) 
	{
		if (it->fd==fd) 
			return it->indexNode;
		
	}	
	return -1;	
}

void printfdTable (){
	vector<FD_entry>::iterator it;
	printf("---------FD Table---------\n");	
	for (it = fd_Table.begin() ; it != fd_Table.end() ; it++) 
	{
		printf("fd: %d  indexNode: %d  offset: %d  fileSize: %d\n", it->fd, it->indexNode, it->offset, it->fileSize);
	}
	printf("---------End of FD Table---------\n");		
}

FD_entry* getEntryFromFd(int fd) {

	FD_entry *entry;
	int i;

	for (i=0; i<fd_Table.size();i++) {
		entry = &(fd_Table[i]);
		if (entry->fd=fd)
			return entry;
	} 
	return entry;	
}

void deleteFileFromFDTable(int fd) {
	vector<FD_entry>::iterator it;
	for (it = fd_Table.begin() ; it != fd_Table.end() ; it++) 
	{
		fd_Table.erase(it);
		return;
	}	
}

int main () {

	fd = open ("/proc/ramdisk", O_RDONLY);
	printf("Hello world\n");

	FD_entry entry;
	entry.indexNode = 1;
	entry.offset = 0;	// Default file pointer to the start of file
	entry.fileSize=10;	// -1 indicates the file does not have size yet
	entry.fd = 1;	// Set file descriptor
	fd_Table.push_back(entry);

	printfdTable();

	int ret;
	ret = rd_creat("/mytxt.txt");
	printf("Index Node: %d\n", ret);
}