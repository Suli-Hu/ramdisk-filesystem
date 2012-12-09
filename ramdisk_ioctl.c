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
  short shortData;
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
  createIndexNode("dir\0", "/\0",  0);

  /****** At start, root directory has no files, so its block is empty (but claimed) at the moment ******/
  printk("RAMDISK has been initialized with memory\n");
}

/************************ INTERNAL HELPER FUNCTIONS **************************/
/**
 * Get the next free IndexNodeNumber. Also clears up the block pointers.
 *
 * @return  int return index node number of a free index node
 */

int getNewIndexNodeNumber(void) {

  int ii;
  char *indexNodeType;

  for (ii=0; ii<INDEX_NODE_COUNT; ii++) {

    indexNodeType = RAM_memory+INDEX_NODE_ARRAY_OFFSET+ii*INDEX_NODE_SIZE+INODE_TYPE;
    if (!(strlen(indexNodeType)>1)) {
      /* Clear up this index node before giving it back */
      negateIndexNodePointers(ii);
      return ii;
    }
  }

  return -1; /* No index node was found, so return this as an error */
}

/**
 * Helper method to clear an index node when a file/dir is removed
 * TO DO, FREE MEMORY FOR INDEX NODE
 * @return  void
 * @param[indexNodeNumber]  int index node number to clear
 */
void clearIndexNode(int IndexNodeNumber) {

  int i, j, blocknumber, blocknumberInner;
  char *indexNodeStart;
  char *singleIndirectBlockStart;
  char *doubleIndirectBlockStart;

  indexNodeStart = RAM_memory+INDEX_NODE_ARRAY_OFFSET+IndexNodeNumber*INDEX_NODE_SIZE;

  /****** Free memory used by index node *****/
  // Direct memory freeing
  for (i=0; i<8; i++) {
    blocknumber = (int) *(indexNodeStart+DIRECT_1+i*4);

    // If we received an unallocated block, we are done freeing memory
    if (blocknumber<=0)
  break;

       freeBlock(blocknumber);
  }

  // Single indirect memory freeing
  blocknumber = (int) * (int *)(indexNodeStart+SINGLE_INDIR);
  singleIndirectBlockStart =  RAM_memory + DATA_BLOCKS_OFFSET + (blocknumber*RAM_BLOCK_SIZE);
  freeBlock(blocknumber);

  for (i=0; i<64; i++) {
    blocknumber = (int) * (int *)(singleIndirectBlockStart+i*4);

    // If we received an unallocated block, we are done freeing memory
    if (blocknumber==-1)
  break;

    freeBlock(blocknumber);
  }

  // Double indirect memory freeing
  blocknumber = (int) * (int *)(indexNodeStart+DOUBLE_INDIR);
  singleIndirectBlockStart =  RAM_memory + DATA_BLOCKS_OFFSET + (blocknumber*RAM_BLOCK_SIZE);
  freeBlock(blocknumber);

  for (i=0; i<64; i++) {
    blocknumber = (int) * (int *)(singleIndirectBlockStart+i*4);

    // If we received an unallocated block, we are done freeing memory
    if (blocknumber<=0)
  break;

    doubleIndirectBlockStart = RAM_memory + DATA_BLOCKS_OFFSET + (blocknumber*RAM_BLOCK_SIZE);

    for (j=0; j<64; j++) {
        blocknumberInner = (int) * (int *)(doubleIndirectBlockStart+j*4);

  if (blocknumberInner<=0)
      break;
  freeBlock(blocknumberInner);
    }
    freeBlock(blocknumber);
  }  

 /****** End of Free memory used by index node *****/

  // Clear index node bits
  for (i=0; i<INDEX_NODE_SIZE; i++)
    indexNodeStart[i] = '\0';
}

/**
 * Helper function for setting memory region to -1 for easier tracking 
 * Set Direct 1-8, single indirect 9 and double indirect 10 byte to -1
 *
 * @param[in-out]  indexNodeNumber - id of the index number
 */
void negateIndexNodePointers(int indexNodeNumber) {
  int ii, negate;
  char *indexNodeMemoryRegion;
  negate = -1;
  indexNodeMemoryRegion = RAM_memory+INDEX_NODE_ARRAY_OFFSET+indexNodeNumber*INDEX_NODE_SIZE+DIRECT_1;
  for (ii = 0 ; ii < 10 ; ii++) {
    memcpy(indexNodeMemoryRegion + ii*4, &negate, sizeof(int));
  }
}

/**
 * Creates a new index node
 *
 * @return  type  description
 * @param[in-out]  type  type of the node, 'dir' or 'reg'
 * @param[in-out]  filename  name of the file or dir, 'dir' or 'reg'
 * @param[in-out]  memorysize  size of the file or dir in bytes
 */

int createIndexNode(char *type, char *filename, int memorysize) {

  if (memorysize>MAX_FILE_SIZE) {
    printk("File too large!\n");
    return -1;
  }

  int indexNodeNumber;
  int data;
  int numberOfBlocksRequired;
  short shortData;
  char *indexNodeStart;

  indexNodeNumber = getNewIndexNodeNumber();
  numberOfBlocksRequired = (memorysize/RAM_BLOCK_SIZE)+1;
  allocMemoryForIndexNode(indexNodeNumber, numberOfBlocksRequired);
  indexNodeStart = RAM_memory+INDEX_NODE_ARRAY_OFFSET+indexNodeNumber*INDEX_NODE_SIZE;


  /****** Set up the root index node *******/
  // Set the type
  strcpy(indexNodeStart+INODE_TYPE,type);
  // Set indexNode size
  data = memorysize;
  memcpy(indexNodeStart+INODE_SIZE, &data, sizeof(int));
  // Set the file count, default to 0
  shortData = 0;
  memcpy(indexNodeStart+FILE_COUNT, &shortData , sizeof(short));
  strcpy(indexNodeStart+INODE_FILE_NAME, filename);

  printk("New index node: %d created\n", indexNodeNumber);

  return indexNodeNumber;
}

/**
 * Allocate memory for indexNode
 *
 * @param[in-out]  indexNodeNumber - index Node number to allocate memory for
 * @param[in-out]  numberOfBlocks - number of blocks to allocate
 */

void allocMemoryForIndexNode(int indexNodeNumber, int numberOfBlocks) {

  char *indexNodeStart;
  char *singleIndirectBlockStart;
  char *doubleIndirectBlockStart;
  int i, j, blockNumber, singleIndirectMemBlock, doubleIndirectMemBlock;
  indexNodeStart = RAM_memory+INDEX_NODE_ARRAY_OFFSET+indexNodeNumber*INDEX_NODE_SIZE;

  // Allocate memory for direct blocks first
  for (i=0; i<8; i++) {

    blockNumber = getFreeBlock();
    // @todo Weird situation, pointers are 8 bytes, so storing them doesn't make sense, store number instead
    memcpy(indexNodeStart+DIRECT_1+ 4*i, &blockNumber, sizeof(int));

    numberOfBlocks--;

    // If number of blocks have reached 0, we are done allocating memory (works for now....)
    if (numberOfBlocks==0)
      return;
  }

  // Allocate memory for single indirect block second
   singleIndirectMemBlock = getFreeBlock();
   memcpy(indexNodeStart+ SINGLE_INDIR, &singleIndirectMemBlock, sizeof(int));
   singleIndirectBlockStart =  RAM_memory + DATA_BLOCKS_OFFSET + (singleIndirectMemBlock*RAM_BLOCK_SIZE);

  // Each data block hold 64 pointers to further memory blocks
   for (i=0;i<64;i++) {

    blockNumber = getFreeBlock();
    memcpy(singleIndirectBlockStart+ 4*i, &blockNumber, sizeof(int));

    numberOfBlocks--;

    if (numberOfBlocks==0)
      return;
   }

  // Allocate memory for double indirect block third
  doubleIndirectMemBlock = getFreeBlock();
  memcpy(indexNodeStart+ DOUBLE_INDIR, &doubleIndirectMemBlock, sizeof(int));  
  doubleIndirectBlockStart = RAM_memory + DATA_BLOCKS_OFFSET + (doubleIndirectMemBlock*RAM_BLOCK_SIZE);

  // For each data block, we will allocate another data block of 64 pointers
  for (i=0;i<64;i++) {

  singleIndirectMemBlock = getFreeBlock();
  singleIndirectBlockStart =  RAM_memory + DATA_BLOCKS_OFFSET + (singleIndirectMemBlock*RAM_BLOCK_SIZE);
  memcpy(doubleIndirectBlockStart+ 4*i, &singleIndirectMemBlock, sizeof(int));

     // Each data block hold 64 pointers to further memory blocks
     for (j=0;j<64;j++) {

      blockNumber = getFreeBlock();
      memcpy(singleIndirectBlockStart+ 4*j, &blockNumber, sizeof(int));

      numberOfBlocks--;

      if (numberOfBlocks==0)
        return;
     }
  }

}

/************************ MEMORY MANAGEMENT *****************************/
int getFreeBlock(void) {

  int i, j;

  for (i=0; i<BLOCK_BITMAP_SIZE; i++) {
    for (j=7; j>=0; j--) {
      if (!checkBit(BLOCK_BITMAP_OFFSET+i, j)) {
        // Return block number
        setBit(BLOCK_BITMAP_OFFSET+i, j);
        // Convert the loop indices into a block bitmap index
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
 * @param[in]  numberOfBits specifies the number of bits to print
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
  int singleDirectBlock, doubleDirectBlock, memoryblock,memoryblockinner, i, j;


  indexNodeStart = RAM_memory+INDEX_NODE_ARRAY_OFFSET+nodeIndex*INDEX_NODE_SIZE;
  printk("-----Printing indexNode %d-----\n", nodeIndex);
  printk("NODE TYPE:%.4s\n", indexNodeStart+INODE_TYPE);
  printk("NODE SIZE:%d\n", (int) *( (int *) (indexNodeStart+INODE_SIZE) ) );
  printk("FILE COUNT:%hi\n", (short) *( (short *)(indexNodeStart+FILE_COUNT) ) );  
  printk("FILE NAME: %s\n", indexNodeStart+INODE_FILE_NAME);

  // Prints the Direct memory channels
  printk("MEM DIRECT: ");
  for (i=0; i<=7;i++) {
      printk("%d  ", (int)(*(indexNodeStart+DIRECT_1 + 4*i)));
  }
  printk("\n");

  // Prints the Single indirect channels 
   singleDirectBlock = (int) *(indexNodeStart+SINGLE_INDIR) ;
   singleIndirectStart = RAM_memory + DATA_BLOCKS_OFFSET + (singleDirectBlock*RAM_BLOCK_SIZE);

   printk("MEM SINGLE INDIR: \n");
   for (i=0; i<RAM_BLOCK_SIZE/4;i++) {
     memoryblock = (int)(*(singleIndirectStart+4*i));
     if (memoryblock!=0 || memoryblock!=-1)
        printk("%d  ", memoryblock);
   }
   printk("\n");

  // Prints the Double indirect channels 
   doubleDirectBlock = (int) *(indexNodeStart+DOUBLE_INDIR) ;
   doubleIndirectStart = RAM_memory + DATA_BLOCKS_OFFSET + (doubleDirectBlock*RAM_BLOCK_SIZE);

   printk("MEM DOUBLE INDIR: ");
   for (i=0; i<RAM_BLOCK_SIZE/4;i++) {
     singleDirectBlock = (int)(*(doubleIndirectStart+4*i));
     singleIndirectStart = RAM_memory + DATA_BLOCKS_OFFSET + (singleDirectBlock*RAM_BLOCK_SIZE);

     if (singleDirectBlock>0) {
  printk("Sector %d :\n ", i);
  for (j=0; j<RAM_BLOCK_SIZE/4;j++) {
  memoryblockinner =  (int)(*(singleIndirectStart+4*j));

        if (memoryblockinner>0)
           printk("%d ", memoryblockinner);
  }
  printk("\n");
     }

   }
   printk("\n");

   printk("-----End of Printing indexNode %d-----\n", nodeIndex);


}

/************************INIT AND EXIT ROUTINES*****************************/

/** 
* The main init routine for the kernel module.  Initializes proc entry 
*/
static int __init initialization_routine(void) {
  int indexNodeNum;

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

printk("MEM BEFORE\n");
  printBitmap(400);
  indexNodeNum = createIndexNode("reg\0", "myfile.txt\0",  64816);
  printIndexNode(indexNodeNum);

  clearIndexNode(indexNodeNum);
printk("MEM AFTER\n");
  printBitmap(400);

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