/*******************************************************************************
 * filename: pic_led.c
 *
 * Implements the functions defined in pci_led.h
 *
 * Written by: James Ross
 ******************************************************************************/
#include "pci_led.h"
#include "ledTimer.h"

#define REG_SIZE   sizeof(int)
#define ONE_BYTE   1
#define MM_BAR0    0 /* memory map IO on 82583 is bar0 */
#define TWO_BYTE   2
#define FOUR_BYTE  4

                /* global variables */
static struct pci_data{
    void __iomem *hwAddr;
    int bar;
}pciData;

                    /* functions */
int gbe38v_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    int errRet;
    resource_size_t mmioStart, mmioLen;

    errRet = pci_enable_device_mem(pdev);
    if(unlikely(errRet != SUCCESS)){
        printk(KERN_WARNING DEV_NAME ": Failed to enable device mem region, "
                                        "errRet: %d", errRet);
        return errRet;
    }
    
    /* set bar registers */
    pciData.bar = pci_select_bars(pdev, IORESOURCE_MEM);
    errRet = pci_request_selected_regions(pdev, pciData.bar, DEV_NAME);
    if(unlikely(errRet != SUCCESS)){
        printk(KERN_WARNING DEV_NAME ": Failed to PCI request bar mem region, "
                                     "errRet: %d", errRet);
        goto pci_request_region_fail;
    }
    
    /* get master on bus for communication */
    pci_set_master(pdev);

    /* set memory map io start/finish for physical address */ 
    mmioStart = pci_resource_start(pdev, MM_BAR0);
    mmioLen   = pci_resource_len(pdev, MM_BAR0);

    /* get physical space addr */
    pciData.hwAddr = ioremap(mmioStart, mmioLen);
    if(unlikely(!pciData.hwAddr)){
        printk(KERN_WARNING DEV_NAME ": Failed to remap io, errRet: %d", errRet);
        errRet = -EIO;
        goto ioremap_fail;
    }
    
    /* reset MAC function, register values reset and wait till complete */
    iowrite32(RST_BIT, (pciData.hwAddr)+DEVCTL_OFFSET);
    while((ioread32((pciData.hwAddr) + DEVCTL_OFFSET) & RST_BIT) != 0);

    return SUCCESS;

ioremap_fail: 
    pci_release_selected_regions(pdev, pciData.bar);
pci_request_region_fail:
    pci_disable_device(pdev);
    return errRet;
}/* end gbe38v_probe */

void gbe38v_remove(struct pci_dev *pdev)
{
    iounmap(pciData.hwAddr);
    pci_release_selected_regions(pdev, pciData.bar);
    pci_disable_device(pdev);
}/* end gbe38v_remove */

int gbe38v_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO DEV_NAME ": Device opened\n"); 
    /* have hardware address, can initialize the timer for pci device LED */
    gbe38v_init_led_timer((pciData.hwAddr)+LED_OFFSET);

    return SUCCESS;
}/* end gbe38v_open */

int gbe38v_release(struct inode *inode, struct file *file)
{
    gbe38v_remove_led_timer();
    printk(KERN_INFO DEV_NAME ": Device closed\n");
    return SUCCESS;
}/* end gbe38v_release */

ssize_t gbe38v_read(struct file *filep, char __user *buff, size_t count, 
                  loff_t *offp)
{
    unsigned long retVal;

    /* read 0 bytes */
    if(unlikely(count == 0))
        return 0;
    
    switch(count){
        case  ONE_BYTE: 
            retVal = ioread8((pciData.hwAddr)+LED_OFFSET);
            break;
        case TWO_BYTE: 
            retVal = ioread16((pciData.hwAddr)+LED_OFFSET);
            break;
        case FOUR_BYTE: 
            retVal = ioread32((pciData.hwAddr)+LED_OFFSET);
            break;
        default: /* error */
            printk(KERN_WARNING DEV_NAME ": invalid count, 1,2 or 4 bytes\n");
            return -EINVAL;
    }

    if(unlikely(buff  == NULL)){
        printk(KERN_WARNING DEV_NAME ": NULL __user buffer\n");
        return -EINVAL;
    } /* NULL buffer */

    retVal = copy_to_user((int*)buff, &retVal , count);
    if(unlikely(retVal != SUCCESS)){
        if(retVal > SUCCESS)
            printk(KERN_WARNING DEV_NAME ": read: copy_to_user(), partial copy\n");
        else /* error */
            return retVal;
    }

    return count - retVal;
}/* end gbe_read */

ssize_t gbe38v_write(struct file *filep, const char __user *buff, size_t count,
                   loff_t *offp)
{
    unsigned long retVal;
    unsigned long toWrite;

    if(unlikely(count == 0))/* write 0 bytes */
        return SUCCESS;

    if(unlikely(buff  == NULL)){
        printk(KERN_WARNING DEV_NAME ": NULL __user buffer\n");
        return -EINVAL;
    } 

    retVal = copy_from_user(&toWrite, (int*)buff, count);
    if(unlikely(retVal != SUCCESS)){
        if(retVal > SUCCESS){
            printk(KERN_WARNING DEV_NAME ": write: copy_from_user(), "
                   "partial copy, no write to register.");
            return -EIO;
        }
        else /* error */
            return retVal;
    }

    switch(count){
        case  ONE_BYTE: 
            iowrite8(toWrite,(pciData.hwAddr)+LED_OFFSET);
            break;
        case TWO_BYTE: 
            iowrite16(toWrite, (pciData.hwAddr)+LED_OFFSET);
            break;
        case FOUR_BYTE: 
            iowrite32(toWrite, (pciData.hwAddr)+LED_OFFSET);
            break;
        default: /* error */
            printk(KERN_WARNING DEV_NAME ": invalid count, 1,2 or 4 bytes\n");
            return -EINVAL;
    }

    return SUCCESS;
}/*end gbe_write */
/****************** EOF **************/
