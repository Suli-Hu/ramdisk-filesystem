#include <linux/module.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

/*********************FILE SYSTEM FUNCTION INCLUDE**************************/


/*********************FILE SYSTEM STRUCTURE************************/
#define BLOCK_SIZE 256
#define INDEX_NODE_SIZE 64
#define INDEX_NODE_ARRAY_LENGTH 256
#define BLOCK_BITMAP_SIZE 4

#define SUPERBLOCK_OFFSET 0
#define INDEX_NODE_ARRAY_OFFSET 1

// I'm indexing the fs via block size
#define BLOCK_BITMAP_OFFSET INDEX_NODE_SIZE+INDEX_NODE_ARRAY_OFFSET 
#define DATA_START_OFFSET BLOCK_BITMAP_OFFSET+BLOCK_BITMAP_SIZE

/*********************INDEX NODE STRUCTURE************************/


/*********************FILE SYSTEM FILE STRUCTURE************************/




/**************************IOCTL DECLARATIONS*******************************/