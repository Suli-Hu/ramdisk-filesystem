#define KERNELREADY 0

/*****************************IOCTL STRUCTURES*******************************/

struct RAM_path
{
    char *name;  /** Pathname for the file */
    int ret;          /** Return value, will be used for a variety of reasons */
};

struct RAM_file
{
    int fd;       /** File descriptor */
    int offset;  /** Only used for seek, not for close.  Offset into data requested */
    int ret;      /** Return value */
};

struct RAM_accessFile
{
    int fd;               /** File descriptor */
    int numBytes;    /** Number of bytes to transfer into userspace (Used if regular file) */
    int ret;              /** Return value */
    char *address;  /** User space address to which to send data */
};

struct FD_entry
{
	int fd;				/* File descriptor */
	int indexNode;		/* IndexNode ID */
	int offset;			/* Offset in the file */ 
	int fileSize;		/* Size of file */
};

/***************************KERNEL FS FUNCTION PROTOTYPES********************/

/**
 * Kernel pair for the create function
 *
 * @param[in]   input   The RAM_path struct for creating the file
 */
int kr_creat(struct RAM_path input);

/**
 * Kernel pair for making a new directory
 *
 * @param[in]   input   The RAM_path struct for creating the file
 */
int kr_mkdir(struct RAM_path input);

/**
 * Kernel pair for opening a file
 *
 * @param[in]   input   The RAM_path struct for opening the file
 */
int kr_open(struct RAM_path input);

/**
 * Kernel pair for closing a file
 * @todo may not be necessary, not sure yet
 *
 * @param[in]   input   The file struct for closing the file
 */
int kr_close(int fd);

/**
 * Kernel pair for reading a file
 *
 * @param[in]   input   The accessfile struct.  Output read is placed into this struct
 */
int kr_read(struct RAM_accessFile input);

/**
 * Kernel pair for the write function
 *
 * @param[in]   input   The accessfile struct.  Input for writing is in this struct
 */
int kr_write(struct RAM_accessFile input);

/**
 * Kernel pair for the seeking function
 *
 * @param[in]   input   input, use offset in here to index into file
 */
int kr_lseek(struct RAM_file input);

/**
 * Kernel pair for unlinking a file from the filesystem
 *
 * @param[in]   input   Path struct.  Will delete file at the desired RAM_path
 */
int kr_unlink(struct RAM_path input);

/**
 * Kernel pair for the readdir function
 *
 * @param[in]   input   Accessfile struct.  Used to read the relevant directory
 */
int kr_readdir(struct RAM_accessFile input);
