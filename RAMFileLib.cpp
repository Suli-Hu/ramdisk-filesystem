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

int currentFdNum;

int rd_creat(char *pathname) {
	struct RAM_path rampath;
	rampath.name = pathname;
#if 1
  	ioctl (fd, RAM_CREATE, &rampath);	
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
	
	if (fileAlreadyOpen==-1) {
		entry.indexNode = file.indexNode;
		entry.offset = 0;	// Default file pointer to the start of file
		entry.fd = currentFdNum++;	// Set file descriptor
		entry.fileSize = file.fileSize;
		entry.dirIndex = 0;	// Initially the file pointer is 0 (first file in dir)
		fd_Table.push_back(entry);

		printf("Inserting fd entry with fd=%d inode=%d\n", entry.fd, entry.indexNode);
		return entry.fd;
	}
	else {
  		entry.fd = fdFromIndexNode(file.indexNode);
		printf("File already open\n");
	}

	return entry.fd;
}

void kr_close(int fd) {

	// Closing a file simply means removing a file from the FD table
	deleteFileFromFDTable(fd);
}

int rd_read(int file_fd, char *address, int num_bytes) {

	// Make sure the file exists
	if (checkIfFileExists(file_fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

	// Update the offset after reading the file
	FD_entry *entry;
	entry = getEntryFromFd(file_fd);

	struct RAM_accessFile file;
	file.fd = file_fd;
	file.address=address;
	file.numBytes = num_bytes;
	file.indexNode = indexNodeFromfd(file_fd);
	file.offset = entry->offset;

#if 1
  	ioctl (fd, RAM_READ, &file);	
#endif

  	// Update the offset after reading the file
	entry->offset = file.offset;

	return file.ret;
}

int rd_write(int file_fd, char *address, int num_bytes) {

	// Make sure the file exists
	if (checkIfFileExists(file_fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

	struct RAM_accessFile file;
	
	FD_entry *entry;
	entry = getEntryFromFd(file_fd);

	file.fd = file_fd;
	file.address=address;
	file.numBytes = num_bytes;
	file.indexNode = indexNodeFromfd(file_fd);
	file.offset = entry->offset;


#if 1
  	ioctl (fd, RAM_WRITE, &file);	
#endif  	

  	// Update the offset after reading the file
	entry->offset = file.offset;

	return file.ret;
}

int rd_lseek(int file_fd, int offset) {

	// Make sure the file exists
	if (checkIfFileExists(file_fd)==-1){
		printf("fd does not exist in the file descriptor table.\n");
		return -1;
	}

	// If the offset the user specifies is greater than the file size
	// move the pointer to end of file
	struct FD_entry *entry;
	entry = getEntryFromFd(file_fd);
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

int rd_readdir(int file_fd, char *address) {
	struct RAM_accessFile file;
	file.fd = file_fd;
	file.address=address;

	struct FD_entry *entry;
	entry = getEntryFromFd(file_fd);

	file.indexNode = entry->indexNode;
	file.dirIndex = entry->dirIndex;

	// Make sure the file exists
	if (checkIfFileExists(file_fd)==-1){
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
	currentFdNum = 0;
//	printf("Hello world\n");

	FD_entry entry;
	entry.indexNode = 1;
	entry.offset = 0;	// Default file pointer to the start of file
	entry.fileSize=10;	// -1 indicates the file does not have size yet
	entry.fd = 1;	// Set file descriptor
//	fd_Table.push_back(entry);

	int inode, ret;
	// ret = rd_creat("/mytxt.txt");
	// printf("New Index Node: %d\n", inode);

	// printfdTable();

	// inode =rd_open("/mytxt.txt");	

	// printfdTable();

	// printf("fd: %d\n", inode);
	// ret =rd_write(inode, "hello world\n", 10);
	// printf("Write ret: %d\n", ret);
	char output[12];

	// printfdTable();
	// ret = rd_read(inode, output, 10);
	// printf("Read data: %s inode: %d\n", output, inode);

	// printfdTable();

	rd_creat("/mytxt.txt\0");
	inode = rd_open("/mytxt.txt\0");
	rd_mkdir("/folder/\0");
	rd_write(inode, "hello world\n", 12);
	rd_read(inode, output, 12);
	printf("The file has - %s\n", output);
	rd_readdir(inode, output);
	printf("The file in the dir - %s\n", output);
	rd_unlink("/mytxt.txt\0");

}