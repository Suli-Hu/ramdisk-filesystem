#include <linux/module.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

/****************************IOCTL DECLARATIONS*******************************/

#define CREATE _IORW(0, 6, struct path)
#define MKDIR _IORW(1, 7, struct path)
#define OPEN _IORW(1, 8, struct path)
#define CLOSE _IORW(1, 9, struct file)
#define READ _IORW(1, 10, struct accessFile)
#define WRITE _IORW(1, 11, struct accessFile)
#define LSEEK _IORW(1, 12, struct file)
#define UNLINK _IORW(1, 13, struct path)
#define READDIR _IORW(1, 14, struct accessFile)

/*********************FILE SYSTEM STRUCTURE************************/
#define BLOCK_SIZE 256  // Size in bytes
#define INDEX_NODE_SIZE 64  // Size in bytes
#define INDEX_NODE_ARRAY_LENGTH 256  // Number of blocks
#define INDEX_NODE_COUNT INDEX_NODE_ARRAY_LENGTH*(INDEX_NODE_ARRAY_LENGTH/INDEX_NODE_SIZE)
#define BLOCK_BITMAP_SIZE 4*BLOCK_SIZE

#define SUPERBLOCK_OFFSET 0
#define INDEX_NODE_ARRAY_OFFSET BLOCK_SIZE

// I'm indexing the fs via block size
#define BLOCK_BITMAP_OFFSET (INDEX_NODE_ARRAY_LENGTH+1)*BLOCK_SIZE

// This is the index into the root dir in bytes
#define ROOT_DIR BLOCK_SIZE*(BLOCK_BITMAP_OFFSET+BLOCK_BITMAP_SIZE)
#define ROOT_INDEX_NODE 0 // Simply to make this access clearer

/*********************INDEX NODE STRUCTURE************************/
// Indexes into an inode are in bytes, must be cast into an int or pointer
// before used, but I don't take that into account here.  To access these
// elements, get the inode offset, and then add in these values
#define INODE_TYPE 0
#define INODE_SIZE 4
#define DIRECT_1 8
#define DIRECT_2 12
#define DIRECT_3 16
#define DIRECT_4 20
#define DIRECT_5 24
#define DIRECT_6 28
#define DIRECT_7 32
#define DIRECT_8 36
#define SINGLE_INDIR 40
#define DOUBLE_INDIR 44

// I add this custom value to keep track of directory file count
// I use 2 bytes ( a short ) for this
#define FILE_COUNT 48

// I also keep within here the filename for convenience, so that it
// isn't necessary to view memory to access current files name
#define INODE_FILE_NAME 50

/*********************FILE SYSTEM DIRECTORY STRUCTURE************************/
// Files are just raw data pointed to from the index node, directories on the 
// othere hand are groups of 16 byte "structs", where 14 bytes is filename and
// 2 bytes index node number

#define FILE_INFO_SIZE 16
#define INODE_NUM_OFFSET 14 // Offset in file_info to get the inode


/*****************************IOCTL STRUCTURES*******************************/

struct path{
	char *name;  // Pathname for the file
	int ret;          // Return value, will be used for a variety of reasons
};

struct file {
	int fd;       // File descriptor
	int offset;  // Only used for seek, not for close.  Offset into data requested
	int ret;      // Return value
};

struct accessFile {
	int fd;               // File descriptor
	char *address;  // User space address to which to send data
	int numBytes;    // Number of bytes to transfer into userspace (Used if regular file)
	int ret;              // Return value
};
