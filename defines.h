#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <string>

/****************************IOCTL DECLARATIONS*******************************/

#define RAM_CREATE _IOWR(0, 6, struct path)
#define RAM_MKDIR _IOWR(1, 7, struct path)
#define RAM_OPEN _IOWR(1, 8, struct path)
/** @todo Close may not need a kernel call? */
#define RAM_CLOSE _IOWR(1, 9, struct file)
#define RAM_READ _IOWR(1, 10, struct RAM_accessFile)
#define RAM_WRITE _IOWR(1, 11, struct RAM_accessFile)
#define RAM_LSEEK _IOWR(1, 12, struct file)
#define RAM_UNLINK _IOWR(1, 13, struct path)
#define RAM_READDIR _IOWR(1, 14, struct RAM_accessFile)

/*********************FILE SYSTEM STRUCTURE************************/
#define FS_SIZE 2097152 // Exactly 2 MB

#define RAM_BLOCK_SIZE 256  // Size in bytes

#define INDEX_NODE_SIZE 64  // Size in bytes
#define INDEX_NODE_ARRAY_LENGTH 256  // Number of blocks
#define INDEX_NODE_COUNT INDEX_NODE_ARRAY_LENGTH*(RAM_BLOCK_SIZE/INDEX_NODE_SIZE)
#define BLOCK_BITMAP_SIZE 4*BLOCK_SIZE
#define BLOCK_BITMAP_BLOCK_COUNT 4

#define SUPERBLOCK_OFFSET 0
#define INDEX_NODE_ARRAY_OFFSET BLOCK_SIZE

// I'm indexing the fs via block size
#define BLOCK_BITMAP_OFFSET BLOCK_SIZE*(INDEX_NODE_ARRAY_LENGTH+1)

// This is the index into the root dir in bytes
#define ROOT_DIR_OFFSET BLOCK_SIZE*(BLOCK_BITMAP_OFFSET+BLOCK_BITMAP_SIZE)
#define ROOT_INDEX_NODE 0 // Simply to make this access clearer

// The total number of available blocks in the filesystem, excluding the Root Dir since that is always occupied
#define TOT_AVAILABLE_BLOCKS FS_SIZE/BLOCK_SIZE-1-INDEX_NODE_ARRAY_LENGTH-BLOCK_BITMAP_BLOCK_COUNT-1

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

struct RAM_path{
	char *name;  /** Pathname for the file */
	int ret;          /** Return value, will be used for a variety of reasons */
};

struct RAM_file {
	int fd;       /** File descriptor */
	int offset;  /** Only used for seek, not for close.  Offset into data requested */
	int ret;      /** Return value */
};

struct RAM_accessFile {
	int fd;               /** File descriptor */
	int numBytes;    /** Number of bytes to transfer into userspace (Used if regular file) */
	int ret;              /** Return value */
	char *address;  /** User space address to which to send data */
};

/***************************KERNEL FS FUNCTION PROTOTYPES********************/

/**
 * Kernel pair for the create function
 *
 * @param[in]	input	The RAM_path struct for creating the file
 */
void kr_creat(struct RAM_path input);

/**
 * Kernel pair for making a new directory
 *
 * @param[in]	input	The RAM_path struct for creating the file
 */
void kr_mkdir(struct RAM_path input);

/**
 * Kernel pair for opening a file
 *
 * @param[in]	input	The RAM_path struct for opening the file
 */
void kr_open(struct RAM_path input);

/**
 * Kernel pair for closing a file
 * @todo may not be necessary, not sure yet
 *
 * @param[in]	input	The file struct for closing the file
 */
void kr_close(struct RAM_file input);

/**
 * Kernel pair for reading a file
 *
 * @param[in]	input	The accessfile struct.  Output read is placed into this struct
 */
void kr_read(struct RAM_accessFile input);

/**
 * Kernel pair for the write function
 *
 * @param[in]	input	The accessfile struct.  Input for writing is in this struct
 */
void kr_write(struct RAM_accessFile input);

/**
 * Kernel pair for the seeking function
 *
 * @param[in]	input	input, use offset in here to index into file
 */
void kr_lseek(struct RAM_file input);

/**
 * Kernel pair for unlinking a file from the filesystem
 *
 * @param[in]	input	Path struct.  Will delete file at the desired RAM_path
 */
void kr_unlink(struct RAM_path input);

/**
 * Kernel pair for the readdir function
 *
 * @param[in]	input	Accessfile struct.  Used to read the relevant directory
 */
void kr_readdir(struct RAM_accessFile input);

/*************** FUNCTION DECLARATIONS **********************/

int getFreeBlock();
void freeBlock(int blockindex);
