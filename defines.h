#define DEBUG  /* For now, while developing on mac */
#ifdef DEBUG

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#define PRINT printf

#else

	#include <linux/module.h>
	#include <linux/kernel.h>
	#include <linux/init.h>
	#include <linux/vmalloc.h>
	#include <linux/errno.h> /* error codes */
	#include <linux/proc_fs.h>
	#include <asm/uaccess.h>
	#include <linux/tty.h>
	#include <linux/sched.h>
	#include <linux/string.h>
	#include <linux/interrupt.h>

	#define PRINT printK
#endif

#include "structs.h"

/****************************IOCTL DECLARATIONS*******************************/

#define RAM_CREATE _IOWR(0, 6, struct path)
#define RAM_MKDIR _IOWR(1, 7, struct path)
#define RAM_OPEN _IOWR(1, 8, struct path)
#define RAM_READ _IOWR(1, 10, struct RAM_accessFile)
#define RAM_WRITE _IOWR(1, 11, struct RAM_accessFile)
#define RAM_LSEEK _IOWR(1, 12, struct file)
#define RAM_UNLINK _IOWR(1, 13, struct path)
#define RAM_READDIR _IOWR(1, 14, struct RAM_accessFile)

/*********************FILE SYSTEM STRUCTURE************************/
#define FS_SIZE 2097152 // Exactly 2 MB
#define MAX_FILE_SIZE 1067008

#define RAM_BLOCK_SIZE 256  // Size in bytes

#define INDEX_NODE_SIZE 64  // Size in bytes
#define INDEX_NODE_ARRAY_LENGTH 256  // Number of blocks
#define INDEX_NODE_COUNT INDEX_NODE_ARRAY_LENGTH*(RAM_BLOCK_SIZE/INDEX_NODE_SIZE)
#define BLOCK_BITMAP_SIZE 4*RAM_BLOCK_SIZE
#define BLOCK_BITMAP_BLOCK_COUNT 4

#define SUPERBLOCK_OFFSET 0
#define INODE_COUNT_OFFSET 4
#define INDEX_NODE_ARRAY_OFFSET RAM_BLOCK_SIZE

// I'm indexing the fs via block size
#define BLOCK_BITMAP_OFFSET RAM_BLOCK_SIZE*(INDEX_NODE_ARRAY_LENGTH+1)

// This is the index into the root dir in bytes, or the first data block in the ramdisk
#define DATA_BLOCKS_OFFSET BLOCK_BITMAP_OFFSET+BLOCK_BITMAP_SIZE
#define ROOT_INDEX_NODE 0 // Simply to make this access clearer

// The total number of available blocks in the filesystem, excluding the Root Dir since that is always occupied
#define TOT_AVAILABLE_BLOCKS FS_SIZE/RAM_BLOCK_SIZE-1-INDEX_NODE_ARRAY_LENGTH-BLOCK_BITMAP_BLOCK_COUNT

#define MAX_BLOCKS_ALLOCATABLE 4168

/*********************INDEX NODE STRUCTURE************************/
// Indexes into an inode are in bytes, must be cast into an int or pointer
// before used, but I don't take that into account here.  To access these
// elements, get the inode offset, and then add in these values
#define INODE_TYPE 0
#define INODE_SIZE 4
#define DIRECT_1 8
#define SINGLE_INDIR 40
#define DOUBLE_INDIR 44

#define NUM_DIRECT 8

// I add this custom value to keep track of directory file count
// I use 2 bytes ( a short ) for this
#define INODE_FILE_COUNT 48

// I also keep within here the filename for convenience, so that it
// isn't necessary to view memory to access current files name
#define INODE_FILE_NAME 50

/*********************FILE SYSTEM DIRECTORY STRUCTURE************************/
// Files are just raw data pointed to from the index node, directories on the
// othere hand are groups of 16 byte "structs", where 14 bytes is filename and
// 2 bytes index node number

#define FILE_INFO_SIZE 16
#define INODE_NUM_OFFSET 14 // Offset in file_info to get the inode



/**
 * Get free block from memory region
 *
 * @return  blocknumber
 */
int getFreeBlock(void);

void freeBlock(int blockindex);

void allocMemoryForIndexNode(int indexNodeNumber, int numberOfBlocks);

void negateIndexNodePointers(int indexNodeNumber);

int createIndexNode(char *type, char *pathname, int memorysize);

int getIndexNodeNumberFromPathname(char *pathname, int dirFlag);

int insertFileIntoDirectoryNode(int directoryNodeNum, int fileNodeNum, char *filename);

void printIndexNode(int nodeIndex);

char *getFileNameFromPath(char *pathname);

int allocateNewBlockForIndexNode(int indexNode, int current);

void printSuperblock(void);