/**************************************************************************/
/**** Ramdisk IOCTL module for HW3 in Operating systems                ****/
/**** The actual file system functions are stored in a separate file   ****/
/**** This file contains initialization and the entry point             ****/
/**************************************************************************/

/**
  * @author Raphael Landaverde
  * @author Chenkai Liu
  */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include "defines.h"

MODULE_LICENSE("GPL");

static int ramdisk_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static struct file_operations pseudo_dev_proc_operations;
static struct proc_dir_entry *proc_entry;

// @var The ramdisk memory in the kernel */
static char *RAM_memory; 

// void my_printk(char *string)
// {
//   struct tty_struct *my_tty;

//   my_tty = current->signal->tty;

//   if (my_tty != NULL) {
//     (*my_tty->driver->ops->write)(my_tty, string, strlen(string));
//     (*my_tty->driver->ops->write)(my_tty, "\015\012", 2);
//   }
// } 

/**
 * Utility function to set a specified bit within a byte
 *
 * @param[in]  index  The byte index into RAM_memory for which to set the bit
 * @param[in]  bit  the specified bit to set
 * @remark  The most significant bit is 7, while the least significant bit is 0
 */
void setBit(int index, int bit) {
  int mask; 
  mask = 1;// 00000001b
  mask = mask << bit;  // Shift the one to specified position
  RAM_memory[index] |= mask; // Set the bit using OR
}

/**
 * Utility function to check if a specified bit is set within a byte
 *
 * @param[in]  index  The byte index into RAM_memory for which to check the bit
 * @param[in]  bit  the specified bit to check
 * @remark  The most significant bit is 7, while the least significant bit is 0
 */
void checkBit(int index, int bit) {
  //printk ("Checking bit\n");
}

/**
 * RAMDISK initialization
 * Initializes the ramdisk superblock, indexnodes, and bitmap to include the root directory and nothing else
 *
 */
void init_ramdisk(void) {
  // First, we must clear all of the bits of RAM_memory to ensure they are all 0
  int ii, data;
  for (ii = 0 ; ii < FS_SIZE ; ii++)
    RAM_memory[ii] = '\0';  // Null terminator is 0

  /****** Set up the superblock *******/
  // Consists of two values, a 4 byte value containing the free block count
  // and a 4 byte value containing the number of free index nodes
  data = TOT_AVAILABLE_BLOCKS;
  memcpy(RAM_memory, &data, sizeof(int));
  data = INDEX_NODE_COUNT - 1;
  memcpy(RAM_memory+4, &data, sizeof(int));
  // For now, thats all that our superblock contains, may expand more in the future

  /****** Set up the root index node *******/
  // Set the type
  strcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+INODE_TYPE,"dir");
  // Transfer 4 bytes into char array for the size
  data = 0;
  memcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+INODE_SIZE, &data, sizeof(int));
  // Set the file count
  memcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+FILE_COUNT, &data, sizeof(int));
  strcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+INODE_FILE_NAME, "/");

  /****** Set up the block bitmap *******/
  // Set the first bit to be 1 to indicate that this spot is full, endianness won't matter since we 
  // will be consistent with out assignment of bits here
  setBit(BLOCK_BITMAP_OFFSET, 7);  

  /****** At start, root directory has no files, so its block is empty (but claimed) at the moment ******/
}

/************************INIT AND EXIT ROUTINES*****************************/

/** 
* The main init routine for the kernel module.  Initializes proc entry 
*/
static int __init initialization_routine(void) {
  printk("<1> Loading RAMDISK filesystem\n");

  pseudo_dev_proc_operations.ioctl = &ramdisk_ioctl;

  /* Start create proc entry */
  proc_entry = create_proc_entry("ramdisk", 0444, NULL);
  if(!proc_entry)
  {
    printk("<1> Error creating /proc entry for ramdisk.\n");
    return 1;
  }

  //proc_entry->owner = THIS_MODULE; <-- This is now deprecated
  proc_entry->proc_fops = &pseudo_dev_proc_operations;

  // Initialize the ramdisk here now
  RAM_memory = (char *)vmalloc(FS_SIZE);

  // Initialize the superblock and all other memory segments
  init_ramdisk();

  // Verify that memory is correctly set up initially

  return 0;
}

/**
* Clean up routine 
*/
static void __exit cleanup_routine(void) {

  printk("<1> Dumping RAMDISK module\n");
  remove_proc_entry("ramdisk", NULL);

  return;
}

/****************************IOCTL ENTRY POINT********************************/

static int ramdisk_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{ 
  switch (cmd){

    case RAM_CREATE:
      printk ("Creating file...\n");
      break;

    case RAM_MKDIR:
      printk ("Making directory...\n");
      break;

    case RAM_OPEN:
      printk ("Opening file...\n");
      break;
    
    case RAM_CLOSE:
      printk ("Closing file...\n");
      break;
  
    case RAM_READ:
      printk ("Reading file...\n");
      break;

    case RAM_WRITE:
      printk ("Writing file...\n");
      break;
    
    case RAM_LSEEK:
      printk ("Seeking into file...\n");
      break;

    case RAM_UNLINK:
      printk ("Unlinking file...\n");
      break;

    case RAM_READDIR:
      printk ("Reading file from directory...\n");
      break;

    default:
      return -EINVAL;
      break;
  }
  
  return 0;
}

// Init and Exit declaration
module_init(initialization_routine); 
module_exit(cleanup_routine); 