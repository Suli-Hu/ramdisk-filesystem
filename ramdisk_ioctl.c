/**************************************************************************/
/**** Ramdisk IOCTL module for HW3 in Operating systems                ****/
/**** The actual file system functions are stored in a separate file   ****/
/**** This file contains initialization and the entry point             ****/
/**************************************************************************/

/**
  * @author Raphael Landaverde
  * @author Chenkai Liu
  */
#include "defines.h"

MODULE_LICENSE("GPL");

static int ramdisk_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static struct file_operations pseudo_dev_proc_operations;
static struct proc_dir_entry *proc_entry;

// @var The ramdisk memory in the kernel */
static char *RAM_memory; 

void my_printk(char *string)
{
  struct tty_struct *my_tty;

  my_tty = current->signal->tty;

  if (my_tty != NULL) {
    (*my_tty->driver->ops->write)(my_tty, string, strlen(string));
    (*my_tty->driver->ops->write)(my_tty, "\015\012", 2);
  }
} 

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
  mask <<= bit;  // Shift the one to specified position
  RAM_memory[index] |= mask; // Set the bit using OR
}

/**
 * Utility function to clear a specified bit within a byte
 *
 * @param[in]  index  The byte index into RAM_memory for which to clear the bit
 * @param[in]  bit  the specified bit to clear
 * @remark  The most significant bit is 7, while the least significant bit is 0
 */
void clearBit(int index, int bit) {
  int mask; 
  mask = 1;// 00000001b
  mask <<= bit;  // Shift the one to specified position
  mask = ~mask;
  RAM_memory[index] &= mask; // Set the bit using OR
}

/**
 * Utility function to check if a specified bit is set within a byte
 *
 * @param[in]  index  The byte index into RAM_memory for which to check the bit
 * @param[in]  bit  the specified bit to check
 * @return  int   returns 1 if the bit is set, 0 if it is empty
 * @remark  The most significant bit is 7, while the least significant bit is 0
 */
int checkBit(int index, int bit) {
  int mask;
  mask = 1;
  mask <<= bit;
  return (mask & RAM_memory[index]);
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

  /****** Set up the block bitmap *******/
  // Set the first bit to be 1 to indicate that this spot is full, endianness won't matter since we 
  // will be consistent with out assignment of bits here
  data = getFreeBlock();
  memcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+DIRECT_1, &data, sizeof(int));

  /****** Set up the root index node *******/
  // Set the type
  strcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+INODE_TYPE,"dir");
  // Transfer 4 bytes into char array for the size
  data = 0;
  memcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+INODE_SIZE, &data, sizeof(int));
  // Set the file count
  memcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+FILE_COUNT, &data, sizeof(int));
  strcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+INODE_FILE_NAME, "/");


  /****** At start, root directory has no files, so its block is empty (but claimed) at the moment ******/
  printk("RAMDISK has been initialized with memory\n");
}

/************************ INTERNAL HELPER FUNCTIONS **************************/
/**
 * Creates a new index node
 *
 * @return  type  description
 * @param[in-out]  type  type of the node, 'dir' or 'reg'
 * @param[in-out]  filename  name of the file or dir, 'dir' or 'reg'
 * @param[in-out]  memorysize  size of the file or dir in bytes
 */

int getNewIndexNodeNumber() {

  int i;
  char *indexNodeType;

  for (i=0; i<INDEX_NODE_COUNT; i++) {

    indexNodeType = RAM_memory+INDEX_NODE_ARRAY_OFFSET+i*INDEX_NODE_SIZE+INODE_TYPE;
    printk("TYPE: %s\n", indexNodeType);
    if (strcmp(indexNodeType,"dir") || strcmp(indexNodeType,"reg")) {
      printk("Index Node %d is occupied\n", i);
    }
    else {
      printk("Found empty index node.\n");
    }

  }
}

int createIndexNode(char *type, char *filename, int memorysize) {

  getNewIndexNodeNumber();
  // int data;
  // int numberOfBlocksRequired = (memorysize/RAM_BLOCK_SIZE)+1;
  // data = getFreeBlock();
  // memcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+DIRECT_1, &data, sizeof(int));

  // /****** Set up the root index node *******/
  // // Set the type
  // strcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+INODE_TYPE,type);
  // // Set indexNode size
  // data = memorysize;
  // memcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+INODE_SIZE, &data, sizeof(int));
  // // Set the file count, default to 0
  // data = 0;
  // memcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+FILE_COUNT, &data, sizeof(int));
  // strcpy(RAM_memory+INDEX_NODE_ARRAY_OFFSET+INODE_FILE_NAME, filename);

  return 0;
}


/************************ MEMORY MANAGEMENT *****************************/
/**
 * Get free block from memory region
 *
 * @return  blocknumber
 * @param[in-out]  
 */
int getFreeBlock() {

  int i, j;

  for (i=0; i<BLOCK_BITMAP_SIZE; i++) {
    for (j=7; j>=0; j--) {
      if (!checkBit(BLOCK_BITMAP_OFFSET+i, j)) {
        // Return block number
        setBit(BLOCK_BITMAP_OFFSET+i, j);

        return i*8 + (7-j);
      }
        
    }

  }


  // No free blocks
  return -1;
}

void freeBlock(int blockindex) {
  int major, minor;
  major = blockindex / 8;
  minor = blockindex % 8;
  minor = 7 - minor;
  clearBit(BLOCK_BITMAP_OFFSET+major, minor);
}

/************************ DEBUGGING FUNCTIONS *****************************/

/**
 * This debugging function prints the specified number of bitmap numberOfBits
 *
 * @return  void prints the bitmap bits in 25 bit chunks
 * @param[in-out]  numberOfBits specifies the number of bits to print
 */
void printBitmap(int numberOfBits) {
  int i, j, bitCount;
  
  bitCount = 0;

  for (i=0; i<BLOCK_BITMAP_SIZE; i++) {
    for (j=7; j>=0; j--) {

      // We have printed the right number of bits
      if (bitCount>=numberOfBits)
        return;

      if (bitCount%25==0)
          printk("Printing %d - %d bitmaps\n", bitCount, bitCount+24);

      if (!checkBit(BLOCK_BITMAP_OFFSET+i, j)) 
        printk("0 ");
      else 
        printk("1 ");

      bitCount++;
      
      if (bitCount%25==0)
        printk("\n");
        
    }

  }
}

void printIndexNode(int nodeIndex) {

  char *indexNodeStart;
  char *singleIndirectStart;
  char *doubleIndirectStart;
  int i;

  indexNodeStart = RAM_memory+INDEX_NODE_ARRAY_OFFSET+nodeIndex*INDEX_NODE_SIZE;
  printk("--Printing indexNode %d--\n", nodeIndex);
  printk("NODE TYPE:%.4s\n", indexNodeStart+INODE_TYPE);
  printk("NODE SIZE:%d\n", (int)(*(indexNodeStart+INODE_SIZE)));
  printk("FILE COUNT:%d\n", (int)(*(indexNodeStart+FILE_COUNT)));  
  printk("FILE NAME: %s\n", indexNodeStart+INODE_FILE_NAME);

  // Prints the Direct memory channels
  printk("MEM DIRECT: ");
  for (i=0; i<=7;i++) 
      printk("%d  ", (int)(*(indexNodeStart+DIRECT_1 + 4*i)));
  printk("\n");

  // Prints the Single indirect channels 
  // BROKEN, FIX BEFORE COMPILE, WILL FREEZE WILL VM!!!

  // singleIndirectStart = RAM_memory+ROOT_DIR_OFFSET+(RAM_BLOCK_SIZE*((int)(*(indexNodeStart+SINGLE_INDIR))));
  // printk("MEM SINGLE INDIR: ");
  // for (i=0; i<RAM_BLOCK_SIZE/4;i++)
  //   printk("%d  ", (int)(*(singleIndirectStart+4*i)));
  // printk("\n");

  // Prints the Double indirect channels


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

  // Debugging indexNode
  printIndexNode(0);
  printBitmap(200);

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
      my_printk("Creating file...\n");
      break;

    case RAM_MKDIR:
      my_printk("Making directory...\n");
      break;

    case RAM_OPEN:
      my_printk("Opening file...\n");
      break;
    
    case RAM_CLOSE:
      my_printk("Closing file...\n");
      break;
  
    case RAM_READ:
      my_printk("Reading file...\n");
      break;

    case RAM_WRITE:
      my_printk("Writing file...\n");
      break;
    
    case RAM_LSEEK:
      my_printk("Seeking into file...\n");
      break;

    case RAM_UNLINK:
      my_printk("Unlinking file...\n");
      break;

    case RAM_READDIR:
      my_printk("Reading file from directory...\n");
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