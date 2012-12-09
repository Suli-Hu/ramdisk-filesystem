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

    if (my_tty != NULL)
    {
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
void setBit(int index, int bit)
{
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
void clearBit(int index, int bit)
{
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
int checkBit(int index, int bit)
{
    int mask;
    mask = 1;
    mask <<= bit;
    return (mask & RAM_memory[index]);
}

/**
 * Changes the block count in the superblock
 *
 * @param[in]  delta  1 if incrementing (block freed), -1 if decrementing (block added)
 * @remark    delta can be greater than 1 in magnitude, but only if you know what you are doing
 */
void changeBlockCount(int delta)
{
    int blockCount;
    memcpy(&blockCount, RAM_memory, sizeof(int));
    blockCount += delta
    memcpy(RAM_memory, &blockCount, sizeof(int));
}

/**
 * Changes the index node count in the superblock
 *
 * @param[in]  delta  1 if incrementing (index node deleted), -1 if decrementing (index node added)
 * @remark    delta can be greater in magnitude, but only if you know what you are doing
 */
void changeIndexNodeCount(int delta)
{
    int blockCount;
    memcpy(&blockCount, RAM_memory + 4, sizeof(int));
    blockCount += delta;
    memcpy(RAM_memory + 4, &blockCount, sizeof(int));
}

/**
 * RAMDISK initialization
 * Initializes the ramdisk superblock, indexnodes, and bitmap to include the root directory and nothing else
 *
 */
void init_ramdisk(void)
{
    // First, we must clear all of the bits of RAM_memory to ensure they are all 0
    int ii, data;
    short shortData;
    for (ii = 0 ; ii < FS_SIZE ; ii++)
        RAM_memory[ii] = '\0';  // Null terminator is 0

    /****** Set up the superblock *******/
    // Consists of two values, a 4 byte value containing the free block count
    // and a 4 byte value containing the number of free index nodes.  Initialized with
    // everything free
    data = TOT_AVAILABLE_BLOCKS;
    memcpy(RAM_memory, &data, sizeof(int));
    data = INDEX_NODE_COUNT;
    memcpy(RAM_memory + 4, &data, sizeof(int));
    // For now, thats all that our superblock contains, may expand more in the future

    /****************Create the root directory******************/
    createIndexNode("dir\0", "/\0",  256);

    /****** At start, root directory has no files, so its block is empty (but claimed) at the moment ******/
    printk("RAMDISK has been initialized with memory\n");
}

/************************ INTERNAL HELPER FUNCTIONS **************************/
/**
 * Get the next free IndexNodeNumber. Also clears up the block pointers.
 *
 * @return  int return index node number of a free index node
 */

int getNewIndexNodeNumber(void)
{

    int ii;
    int indexNodeCount;
    char *indexNodeType;

    for (ii = 0; ii < INDEX_NODE_COUNT; ii++)
    {

        indexNodeType = RAM_memory + INDEX_NODE_ARRAY_OFFSET + ii * INDEX_NODE_SIZE + INODE_TYPE;
        if (!(strlen(indexNodeType) > 1))
        {
            /* Clear up this index node before giving it back */
            negateIndexNodePointers(ii);
            /* Subtract 1 from the index node count */
            changeIndexNodeCount(-1);
            /* Return the index node number */
            return ii;
        }
    }

    return -1; /* No index node was found, so return this as an error */
}

/**
 * Helper method to clear an index node when a file/dir is removed
 *
 * @return  void
 * @param[indexNodeNumber]  int index node number to clear
 */
void clearIndexNode(int IndexNodeNumber)
{

    int i, j, blocknumber, blocknumberInner;
    char *indexNodeStart;
    char *singleIndirectBlockStart;
    char *doubleIndirectBlockStart;

    indexNodeStart = RAM_memory + INDEX_NODE_ARRAY_OFFSET + IndexNodeNumber * INDEX_NODE_SIZE;

    /****** Free memory used by index node *****/
    // Direct memory freeing
    for (i = 0; i < 8; i++)
    {
        blocknumber = (int) * (indexNodeStart + DIRECT_1 + i * 4);

        // If we received an unallocated block, we are done freeing memory
        if (blocknumber <= 0)
            break;

        freeBlock(blocknumber);
    }

    // Single indirect memory freeing
    blocknumber = (int) * (int *)(indexNodeStart + SINGLE_INDIR);
    singleIndirectBlockStart =  RAM_memory + DATA_BLOCKS_OFFSET + (blocknumber * RAM_BLOCK_SIZE);
    freeBlock(blocknumber);

    for (i = 0; i < 64; i++)
    {
        blocknumber = (int) * (int *)(singleIndirectBlockStart + i * 4);

        // If we received an unallocated block, we are done freeing memory
        if (blocknumber == -1)
            break;

        freeBlock(blocknumber);
    }

    // Double indirect memory freeing
    blocknumber = (int) * (int *)(indexNodeStart + DOUBLE_INDIR);
    singleIndirectBlockStart =  RAM_memory + DATA_BLOCKS_OFFSET + (blocknumber * RAM_BLOCK_SIZE);
    freeBlock(blocknumber);

    for (i = 0; i < 64; i++)
    {
        blocknumber = (int) * (int *)(singleIndirectBlockStart + i * 4);

        // If we received an unallocated block, we are done freeing memory
        if (blocknumber <= 0)
            break;

        doubleIndirectBlockStart = RAM_memory + DATA_BLOCKS_OFFSET + (blocknumber * RAM_BLOCK_SIZE);

        for (j = 0; j < 64; j++)
        {
            blocknumberInner = (int) * (int *)(doubleIndirectBlockStart + j * 4);

            if (blocknumberInner <= 0)
                break;

            freeBlock(blocknumberInner);
        }
        freeBlock(blocknumber);
    }

    /****** End of Free memory used by index node *****/

    // Clear index node bits
    for (i = 0; i < INDEX_NODE_SIZE; i++)
        indexNodeStart[i] = '\0';

    /* Update the superblock index node count */
    changeIndexNodeCount(1);
}

/**
 * Helper function for setting memory region to -1 for easier tracking
 * Set Direct 1-8, single indirect 9 and double indirect 10 byte to -1
 *
 * @param[in-out]  indexNodeNumber - id of the index number
 */
void negateIndexNodePointers(int indexNodeNumber)
{
    int ii, negate;
    char *indexNodeMemoryRegion;
    negate = -1;
    indexNodeMemoryRegion = RAM_memory + INDEX_NODE_ARRAY_OFFSET + indexNodeNumber * INDEX_NODE_SIZE + DIRECT_1;
    for (ii = 0 ; ii < 10 ; ii++)
    {
        memcpy(indexNodeMemoryRegion + ii * 4, &negate, sizeof(int));
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

int createIndexNode(char *type, char *pathname, int memorysize)
{
    int indexNodeNumber;
    int data;
    int numberOfBlocksRequired, numBlocksPlusPointers, numBlocksDoubleIndir;
    int blocksAvailable;
    short shortData;
    char *indexNodeStart;

    // Calculate the actual number of blocks needed for the file
    numberOfBlocksRequired = (memorysize / RAM_BLOCK_SIZE) + 1;
    numBlocksPlusPointers = numberOfBlocksRequired;
    if (numberOfBlocksRequired > 8)
    {
        /* Add an extra block for the singly indirect block */
        numBlocksPlusPointers += 1;
        if (numberOfBlocksRequired > 72)
        {
            /* Need extra blocks for the double indirect pointer blocks */
            numBlocksPlusPointers += 1; /* Add one for the pointer block */
            numBlocksDoubleIndir = numberOfBlocksRequired - 72;
            /* Add the number of blocks necessary for all the double indir necessary */
            numBlocksPlusPointers += (numBlocksDoubleIndir / 64) + 1
        }
    }

    memcpy(&blocksAvailable, RAM_memory, sizeof(int));
    if (numBlocksPlusPointers > blocksAvailable)
    {
        printk("Not enough blocks available!\n");
        return -1;
    }

    if (memorysize > MAX_FILE_SIZE)
    {
        printk("File too large!\n");
        return -1;
    }

    // String parsing to get the file name and directory node
    char delims[] = "/";
    result = strsep( pathname, delims );
    while ( result != NULL )
    {
        filename = strsep( NULL, delims );
    }
    indexNodeNumber = getNewIndexNodeNumber();
    allocMemoryForIndexNode(indexNodeNumber, numberOfBlocksRequired);
    indexNodeStart = RAM_memory + INDEX_NODE_ARRAY_OFFSET + indexNodeNumber * INDEX_NODE_SIZE;

    // Insert the file into the right directory node
    directoryNodeNum = getIndexNodeNumberFromPathname(pathname);
    insertFileIntoDirectoryNode(directoryNodeNum, indexNodeNumber, filename);

    /* Set the index node values */
    // Set the type
    strcpy(indexNodeStart + INODE_TYPE, type);
    // Set indexNode size
    data = memorysize;
    memcpy(indexNodeStart + INODE_SIZE, &data, sizeof(int));
    // Set the file count, default to 0
    shortData = 0;
    memcpy(indexNodeStart + FILE_COUNT, &shortData , sizeof(short));
    strcpy(indexNodeStart + INODE_FILE_NAME, filename);

    printk("New index node: %d created\n", indexNodeNumber);

    return indexNodeNumber;
}

void insertFileIntoDirectoryNode(int directoryNodeNum, int fileNodeNum, char *filename)
{

    char *indexNodeStart, *dirlistingstart, *singleIndirectStart;
    int i, blocknumber, nextblocknumber, freeblock;

    indexNodeStart = RAM_memory + INDEX_NODE_ARRAY_OFFSET + directoryNodeNum * INDEX_NODE_SIZE;

    // Look for the last memory block that is free
    for (i = 0; i < 8; i++)
    {
        blocknumber = (int) * (indexNodeStart + DIRECT_1 + i * 4);
        if (blocknumber > -1)
        {
            nextblocknumber = (int) * (indexNodeStart + DIRECT_1 + (i + 1) * 4);
            if (nextblocknumber == -1)
            {
                freeblock = blocknumber;
                break;
            }
        }
    }

    dirlistingstart = RAM_memory + DATA_BLOCKS_OFFSET + (freeblock * RAM_BLOCK_SIZE);

    // Find the next unused directry file index
    for (i = 0; i < RAM_BLOCK_SIZE / FILE_INFO_SIZE; i++)
    {
        blocknumber = (int) * (dirlistingstart + i * FILE_INFO_SIZE + INODE_NUM_OFFSET);

        // We have find a directory block with no blocknumber, so its unused
        if (blocknumber <= 0)
        {
            strcpy(dirlistingstart + i * FILE_INFO_SIZE, filename);
            memcpy(dirlistingstart + i * FILE_INFO_SIZE + INODE_NUM_OFFSET, (short *)&fileNodeNum , sizeof(short));
            return;
        }
    }

    // If we here, we did not find any free direct memory blocks for our new file, look in single indirect memory blocks
    //blocknumber

}

/**
 * Allocate memory for indexNode
 *
 * @param[in]  indexNodeNumber - index Node number to allocate memory for
 * @param[in]  numberOfBlocks - number of blocks to allocate
 */

void allocMemoryForIndexNode(int indexNodeNumber, int numberOfBlocks)
{

    char *indexNodeStart;
    char *singleIndirectBlockStart;
    char *doubleIndirectBlockStart;
    int i, j, blockNumber, singleIndirectMemBlock, doubleIndirectMemBlock;
    indexNodeStart = RAM_memory + INDEX_NODE_ARRAY_OFFSET + indexNodeNumber * INDEX_NODE_SIZE;

    // Allocate memory for direct blocks first
    for (i = 0; i < 8; i++)
    {

        blockNumber = getFreeBlock();
        // @todo Weird situation, pointers are 8 bytes, so storing them doesn't make sense, store number instead
        memcpy(indexNodeStart + DIRECT_1 + 4 * i, &blockNumber, sizeof(int));

        numberOfBlocks--;

        // If number of blocks have reached 0, we are done allocating memory (works for now....)
        if (numberOfBlocks == 0)
            return;
    }

    // Allocate memory for single indirect block second
    singleIndirectMemBlock = getFreeBlock();
    memcpy(indexNodeStart + SINGLE_INDIR, &singleIndirectMemBlock, sizeof(int));
    singleIndirectBlockStart =  RAM_memory + DATA_BLOCKS_OFFSET + (singleIndirectMemBlock * RAM_BLOCK_SIZE);

    // Each data block hold 64 pointers to further memory blocks
    for (i = 0; i < 64; i++)
    {

        blockNumber = getFreeBlock();
        memcpy(singleIndirectBlockStart + 4 * i, &blockNumber, sizeof(int));

        numberOfBlocks--;

        if (numberOfBlocks == 0)
            return;
    }

    // Allocate memory for double indirect block third
    doubleIndirectMemBlock = getFreeBlock();
    memcpy(indexNodeStart + DOUBLE_INDIR, &doubleIndirectMemBlock, sizeof(int));
    doubleIndirectBlockStart = RAM_memory + DATA_BLOCKS_OFFSET + (doubleIndirectMemBlock * RAM_BLOCK_SIZE);

    // For each data block, we will allocate another data block of 64 pointers
    for (i = 0; i < 64; i++)
    {

        singleIndirectMemBlock = getFreeBlock();
        singleIndirectBlockStart =  RAM_memory + DATA_BLOCKS_OFFSET + (singleIndirectMemBlock * RAM_BLOCK_SIZE);
        memcpy(doubleIndirectBlockStart + 4 * i, &singleIndirectMemBlock, sizeof(int));

        // Each data block hold 64 pointers to further memory blocks
        for (j = 0; j < 64; j++)
        {

            blockNumber = getFreeBlock();
            memcpy(singleIndirectBlockStart + 4 * j, &blockNumber, sizeof(int));

            numberOfBlocks--;

            if (numberOfBlocks == 0)
                return;
        }
    }
}

/************************ MEMORY MANAGEMENT *****************************/
int getFreeBlock(void)
{

    int i, j;
    int blockCount;

    for (i = 0; i < BLOCK_BITMAP_SIZE; i++)
    {
        for (j = 7; j >= 0; j--)
        {
            if (!checkBit(BLOCK_BITMAP_OFFSET + i, j))
            {
                // Return block number
                setBit(BLOCK_BITMAP_OFFSET + i, j);
                /* Decrement the block count in the superblock */
                changeBlockCount(-1);
                // Convert the loop indices into a block bitmap index
                return i * 8 + (7 - j);
            }

        }

    }
    // No free blocks
    return -1;
}

void freeBlock(int blockindex)
{
    int major, minor, blockCount;
    major = blockindex / 8;
    minor = blockindex % 8;
    minor = 7 - minor;
    clearBit(BLOCK_BITMAP_OFFSET + major, minor);

    /* Increment block count in the superblock */
    changeBlockCount(1);
}

/************************ DEBUGGING FUNCTIONS *****************************/

/**
 * This debugging function prints the specified number of bitmap numberOfBits
 *
 * @return  void prints the bitmap bits in 25 bit chunks
 * @param[in]  numberOfBits specifies the number of bits to print
 */
void printBitmap(int numberOfBits)
{
    int i, j, bitCount;
    bitCount = 0;
    for (i = 0; i < BLOCK_BITMAP_SIZE; i++)
    {
        for (j = 7; j >= 0; j--)
        {
            // We have printed the right number of bits
            if (bitCount >= numberOfBits)
                return;
            if (bitCount % 25 == 0)
                printk("Printing %d - %d bitmaps\n", bitCount, bitCount + 24);
            if (!checkBit(BLOCK_BITMAP_OFFSET + i, j))
                printk("0 ");
            else
                printk("1 ");
            bitCount++;
            if (bitCount % 25 == 0)
                printk("\n");
        }
    }
}

void printIndexNode(int nodeIndex)
{

    char *indexNodeStart;
    char *singleIndirectStart;
    char *doubleIndirectStart;
    int singleDirectBlock, doubleDirectBlock, memoryblock, memoryblockinner, i, j;


    indexNodeStart = RAM_memory + INDEX_NODE_ARRAY_OFFSET + nodeIndex * INDEX_NODE_SIZE;
    printk("-----Printing indexNode %d-----\n", nodeIndex);
    printk("NODE TYPE:%.4s\n", indexNodeStart + INODE_TYPE);
    printk("NODE SIZE:%d\n", (int) * ( (int *) (indexNodeStart + INODE_SIZE) ) );
    printk("FILE COUNT:%hi\n", (short) * ( (short *)(indexNodeStart + FILE_COUNT) ) );
    printk("FILE NAME: %s\n", indexNodeStart + INODE_FILE_NAME);

    // Prints the Direct memory channels
    printk("MEM DIRECT: ");
    for (i = 0; i <= 7; i++)
    {
        printk("%d  ", (int)(*(indexNodeStart + DIRECT_1 + 4 * i)));
    }
    printk("\n");

    // Prints the Single indirect channels
    singleDirectBlock = (int) * (indexNodeStart + SINGLE_INDIR) ;
    singleIndirectStart = RAM_memory + DATA_BLOCKS_OFFSET + (singleDirectBlock * RAM_BLOCK_SIZE);

    printk("MEM SINGLE INDIR: \n");
    for (i = 0; i < RAM_BLOCK_SIZE / 4; i++)
    {
        memoryblock = (int)(*(singleIndirectStart + 4 * i));
        if (memoryblock != 0 || memoryblock != -1)
            printk("%d  ", memoryblock);
    }
    printk("\n");

    // Prints the Double indirect channels
    doubleDirectBlock = (int) * (indexNodeStart + DOUBLE_INDIR) ;
    doubleIndirectStart = RAM_memory + DATA_BLOCKS_OFFSET + (doubleDirectBlock * RAM_BLOCK_SIZE);

    printk("MEM DOUBLE INDIR: ");
    for (i = 0; i < RAM_BLOCK_SIZE / 4; i++)
    {
        singleDirectBlock = (int)(*(doubleIndirectStart + 4 * i));
        singleIndirectStart = RAM_memory + DATA_BLOCKS_OFFSET + (singleDirectBlock * RAM_BLOCK_SIZE);

        if (singleDirectBlock > 0)
        {
            printk("Sector %d :\n ", i);
            for (j = 0; j < RAM_BLOCK_SIZE / 4; j++)
            {
                memoryblockinner =  (int)(*(singleIndirectStart + 4 * j));

                if (memoryblockinner > 0)
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
static int __init initialization_routine(void)
{
    int indexNodeNum;

    printk("<1> Loading RAMDISK filesystem\n");

    pseudo_dev_proc_operations.ioctl = &ramdisk_ioctl;

    /* Start create proc entry */
    proc_entry = create_proc_entry("ramdisk", 0444, NULL);
    if (!proc_entry)
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
    indexNodeNum = createIndexNode("reg\0", "/myfile.txt\0",  64816);
    printIndexNode(indexNodeNum);

    clearIndexNode(indexNodeNum);
    printk("MEM AFTER\n");
    printBitmap(400);

    // Verify that memory is correctly set up initially

    return 0;
}

/**
 * Get the index Node number from pathname
 *
 * @returns the index node number of the directory that holds the specified file or di
 * @param[in-out]  name  description
 */
int getIndexNodeNumberFromPathname(char *pathname)
{

}

/**
* Clean up routine
*/
static void __exit cleanup_routine(void)
{

    printk("<1> Dumping RAMDISK module\n");
    remove_proc_entry("ramdisk", NULL);

    return;
}

/****************************IOCTL ENTRY POINT********************************/

static int ramdisk_ioctl(struct inode *inode, struct file *file,
                         unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {

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