#include "RAMFileLib.h"

int rd_creat(char *pathname) {
	return 0;
}

int rd_mkdir(char *pathname) {
	return 0;
}

int rd_open(char *pathname) {
	return 1;
}

int rd_close(int fd) {
	return 0;
}

int rd_read(int fd, char *address, int num_bytes) {
	return 1;
}

int rd_write(int fd, char *address, int num_bytes) {
	return 0;
}

int rd_lseek(int fd, int offset) {
	return 0;
}

int rd_unlink(char *pathname) {
	return 0;
}

int rd_readdir(int fd, char *address) {
	return 1;
}