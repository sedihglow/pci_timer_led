/******************************************************************************
 * filename: varHandle.h
 *
 * Driver functions to enable read and write on a variable
 *
 * written by: James Ross
 ******************************************************************************/

#ifndef _PCI_LED_
#define _PCI_LED_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <asm/io.h>
#include <linux/vmalloc.h>

#define SUCCESS 0
#define FAILURE -1
#define DEV_NAME "pciLED"

#define LED_OFFSET 0xE00
#define DEVCTL_OFFSET 0x0
#define RST_BIT 0x4000000

                /* Function prototypes */
int gbe38v_open(struct inode *inode, struct file *file);

ssize_t gbe38v_read(struct file *filep, char __user *buff, size_t count, 
                  loff_t *offp);

ssize_t gbe38v_write(struct file *filep, const char __user *buff, size_t count,
                   loff_t *offp);

int gbe38v_probe(struct pci_dev *pdev, const struct pci_device_id *ent);

void gbe38v_remove(struct pci_dev *pdev);

#endif
/****************** EOF ***************/
