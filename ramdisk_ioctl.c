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

/************************INIT AND EXIT ROUTINES*****************************/

/** 
* The main init routine for the kernel module.  Initializes proc entry 
*/
static int __init initialization_routine(void) {
  printk("<1> Loading RAMDISK filesystem\n");

  pseudo_dev_proc_operations.ioctl = ramdisk_ioctl;

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
  struct ioctl_test_t ioc;
  struct keyboard_struct key;
  
  switch (cmd){

    case CREATE:
      my_printk ("Creating file...\n");
      break;

    case MKDIR:
      my_printk ("Making directory...\n");
      break;

    case OPEN:
      my_printk ("Opening file...\n");
      break;
    
    case CLOSE:
      my_printk ("Closing file...\n");
      break;
  
    case READ:
      my_printk ("Reading file...\n");
      break;

    case WRITE:
      my_printk ("Writing file...\n");
      break;
    
    case LSEEK:
      my_printk ("Seeking into file...\n");
      break;

    case UNLINK:
      my_printk ("Unlinking file...\n");
      break;

    case READDIR:
      my_printk ("Reading file from directory...\n");
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