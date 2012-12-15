#define KERNELREADY 0

/*****************************IOCTL STRUCTURES*******************************/

struct RAM_path
{
    char *name;  /** Pathname for the file */
    int ret;          /** Return value, will be used for a variety of reasons */
};

struct RAM_file
{
	char *name;
    int indexNode;       /** Index Node */
    int offset;  /** Only used for seek, not for close.  Offset into data requested */
    int ret;      /** Return value */
    int fileSize;
};

struct RAM_accessFile
{
    int fd;               /** File descriptor */
    int numBytes;    /** Number of bytes to transfer into userspace (Used if regular file) */
    int ret;              /** Return value */
    int indexNode;
    int offset;
    int dirIndex;
    int numOfFiles;
    char *address;  /** User space address to which to send data */
};

struct FD_entry
{
	int fd;				/* File descriptor */
	int indexNode;		/* IndexNode ID */
	int offset;			/* Offset in the file */ 
	int fileSize;		/* Size of file */
	int dirIndex;
	int numOfFiles;
};

/***************************KERNEL FS FUNCTION PROTOTYPES********************/

/**
 * Kernel pair for the create function
 *
 * @param[in]   input   The RAM_path struct for creating the file
 */
void kr_creat(struct RAM_path *input);

/**
 * Kernel pair for making a new directory
 *
 * @param[in]   input   The RAM_path struct for creating the file
 */
void kr_mkdir(struct RAM_path *input);

/**
 * Kernel pair for opening a file
 *
 * @param[in]   input   The RAM_path struct for opening the file
 */
void kr_open(struct RAM_file *input);


/**
 * Kernel pair for reading a file
 *
 * @param[in]   input   The accessfile struct.  Output read is placed into this struct
 */
void kr_read(struct RAM_accessFile *input);

/**
 * Kernel pair for the write function
 *
 * @param[in]   input   The accessfile struct.  Input for writing is in this struct
 */
void kr_write(struct RAM_accessFile *input);

/**
 * Kernel pair for the seeking function
 *
 * @param[in]   input   input, use offset in here to index into file
 */
void kr_lseek(struct RAM_file *input);

/**
 * Kernel pair for unlinking a file from the filesystem
 *
 * @param[in]   input   Path struct.  Will delete file at the desired RAM_path
 */
void kr_unlink(struct RAM_path *input);

/**
 * Kernel pair for the readdir function
 *
 * @param[in]   input   Accessfile struct.  Used to read the relevant directory
 */
void kr_readdir(struct RAM_accessFile *input);


/********** Helper Function Declarations **********/
int fdFromIndexNode(int indexNode);
struct FD_entry* getEntryFromFd(int fd);
int checkIfFileExists(int fd);
int indexNodeFromfd(int fd);