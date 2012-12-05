/**
*  RAM Filesystem Library for the ramdisk kernel module
*
* @author Raphael Landaverde
*
*  This is for use in userspace, not kernel space
*/


/**
* Creates a new full file at pathname
*
* @return	int	0 on success, -1 on error
* @param[in]	pathname	the absolute path of the file to create
* @remark	Fails if directory for new file does not exist
*/
int rd_creat(char *pathname);

/**
 * Creates a new directory at pathname
 *
 * @return	int	0 on success, -1 on fail
 * @param[in]	pathname	the absolute path of the directory
 * @remark	Fails if directory for new directory does not exist
 */
int rd_mkdir(char *pathname);

/**
 * Opens a file for reading
 *
 * @return	int	the file descriptor handle for the file or -1 on fail
 * @param[in]	pathname	the absolute path of the file
 * @remark	Fails if file does not exist
 */
int rd_open(char *pathname);

/**
 * Close a currently opened file
 *
 * @return	int	0 on success, -1 on error
 * @param[in]	fd	the file descriptor of the file to close
 * @remark	Fails if file descriptor is not valid
 */
int rd_close(int fd);

/**
 * Read a currently opened file
 *
 * @return	int	Number of bytes actually read, or -1 on fail
 * @param[in]	fd	the file descriptor of the file to read
 * @param[out]	address	pointer to the data in userspace
 * @param[in]	num_bytes	number of bytes to read from file 
 * @remark	Fails if file descriptor is invalid or a directory
 */
int rd_read(int fd, char *address, int num_bytes);

/**
 * Write to a currently opened file
 *
 * @return	int	Number of bytes return, or -1 on error
 * @param[in]	fd	the file descriptor of the file to read
 * @param[in]	address	data to write to memory
 * @param[in]	num_bytes	number of bytes to write
 * @remark	Fails if file descriptor is invalid or a directory
 */
int rd_write(int fd, char *address, int num_bytes);

/**
 * Seeks into a currently opened file
 *
 * @return	int	0 on success, -1 on failure
 * @param[in]	fd	file descriptor of the file to seek into
 * @param[in]	offset	offset into file desired
 * @remark	sets file position to EOF if offset larger than remaining size
 */
int rd_lseek(int fd, int offset);

/**
 * Unlinks the file from ramdisk memory
 *
 * @return	int	0 on success, -1 on failure
 * @param[in]	pathname	absolute path of the file or directory to unlink
 * @remark	Error occurs if file does not exist, file is open, file is a directory, or file is root
 */
int rd_unlink(char *pathname); 

/**
 * Read one entry from the directory file
 *
 * @return	int	1 on success, -1 on failure
 * @param[in]	fd	file descriptor of directory to read
 * @param[out]	address	16 byte output, 14 bytes filename, 2 bytesindex_node num
 * @remark	Fails if fd is a regular file.  On each call, file position of fd incremented by 1
 */
int rd_readdir(int fd, char *address);