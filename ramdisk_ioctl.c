/**************************************************************************/
/**** Ramdisk IOCTL module for HW3 in Operating systems                ****/
/**** The actual file system functions are stored in a separate file   ****/
/**** This file contains initialization and the entry point            ****/
/**************************************************************************/

#include "defines.h"

MODULE_LICENSE("GPL");

static int ramdisk_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

/************************INIT AND EXIT ROUTINES*****************************/

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

  return 0;
}

// Clean up routine
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

  case IOCTL_TEST:
    copy_from_user(&ioc, (struct ioctl_test_t *)arg, 
		   sizeof(struct ioctl_test_t));
    printk("<1> ioctl: call to IOCTL_TEST (%d,%c)!\n", 
	   ioc.field1, ioc.field2);

    my_printk ("Got msg in kernel\n");
    break;

  case KEYBOARD:
    key.letter = my_getchar();
    copy_to_user((struct keyboard_struct *)arg, &key, sizeof(struct keyboard_struct));
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