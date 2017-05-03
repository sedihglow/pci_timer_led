/******************************************************************************
 * filename: pci_ledDrivers.c
 *
 * This driver will make the LED blink through the PCI bus on the ethernet
 * gbe 82583v
 *
 * Written by: James Ross
 ******************************************************************************/

#include "pci_led.h"

#define MINOR_CNT   1
#define MINOR_STRT  0
#define NUM_DEVICES 1

#define PCI_DEV_ID  0x150C
#define PCI_VEND_ID 0x8086

                /* global variables */
static struct{
    dev_t devNum;
    struct cdev *cdev;
    struct device *device;
    struct class *pciClass;
}myDev;

static struct pci_device_id gbe83v_ids[] = {
    { PCI_DEVICE(PCI_VEND_ID, PCI_DEV_ID) },
    {0,} /* end: all zeros */
};

static struct pci_driver gbe83v = {
    .name     = "pciLED",
    .id_table = gbe83v_ids,
    .probe    = gbe38v_probe, 
    .remove   = gbe38v_remove
};

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open  = gbe38v_open,
    .read  = gbe38v_read,
    .write = gbe38v_write
};

                    /* functions */
static int __init pci_led_init(void)
{
    long errRet;

    printk(KERN_ALERT DEV_NAME ": start of setup\n");
    /* get major and minor */
    errRet = alloc_chrdev_region(&(myDev.devNum), MINOR_STRT, MINOR_CNT, 
                                 DEV_NAME);
    if(unlikely(errRet != SUCCESS)){
        printk(KERN_WARNING DEV_NAME ": Unable to allocate major %d\n", 
               MAJOR(myDev.devNum));
        return errRet;
    }
    
    /* set cdev */
    myDev.cdev = cdev_alloc();
    if(IS_ERR(myDev.cdev)){
        printk(KERN_WARNING DEV_NAME ": Unable to allocate cdev, NULL ret\n");
        errRet = PTR_ERR(myDev.cdev);
        goto cdev_alloc_fail;
    }

    cdev_init(myDev.cdev, &fops);
    myDev.cdev -> owner = THIS_MODULE;
    myDev.cdev -> ops = &fops;
    
    errRet = cdev_add(myDev.cdev, myDev.devNum, NUM_DEVICES);
    if(unlikely(errRet != SUCCESS)){
        printk(KERN_WARNING DEV_NAME ": Unable to add cdev, err: %ld\n", errRet);
        goto cdev_add_fail;
    }

    /* place module in /dev, requires a GPL liscense */
    myDev.pciClass = class_create(THIS_MODULE, DEV_NAME);
    if(IS_ERR(myDev.pciClass)){
        errRet = PTR_ERR(myDev.pciClass);
        printk(KERN_WARNING DEV_NAME ": Unable to add cdev, err: %ld\n", errRet);
        goto class_create_fail;
    }
     
    /* No parent device, No additional data */
    myDev.device = device_create(myDev.pciClass, NULL, myDev.devNum, NULL, DEV_NAME);
    if(IS_ERR(myDev.device)){
        printk(KERN_WARNING DEV_NAME ": Failed to create device. %ld\n", errRet);
        errRet = PTR_ERR(myDev.device);
        goto device_create_fail;
    }

    /* register pci device */
    errRet = pci_register_driver(&gbe83v);
    if(unlikely(errRet != SUCCESS)){
        printk(KERN_WARNING DEV_NAME ": Unable to reg pci, err: %ld\n", errRet);
        goto pci_register_fail;
    }
    
    printk(KERN_ALERT DEV_NAME ": End of setup\n");
    
    return SUCCESS;

/* error handling */
pci_register_fail : 
    device_destroy(myDev.pciClass, myDev.devNum);
device_create_fail: 
    class_destroy(myDev.pciClass);
class_create_fail :
cdev_add_fail     : 
    cdev_del(myDev.cdev);
cdev_alloc_fail   : 
    unregister_chrdev_region(myDev.devNum, MINOR_CNT);
    return errRet;
}/* end pci_led_init */

static void __exit pci_led_exit(void)
{
    pci_unregister_driver(&gbe83v);
    device_destroy(myDev.pciClass, myDev.devNum);
    class_destroy(myDev.pciClass);
    unregister_chrdev_region(myDev.devNum, MINOR_CNT);
    cdev_del(myDev.cdev);
}/* end pci_led_exit */

module_init(pci_led_init);
module_exit(pci_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("James Ross");
MODULE_DESCRIPTION("Sets the led's on the ethernet port on an ATOM box"
                    "utilizing pci.");

MODULE_VERSION("0.1"); /* Look for convention in module.h */
/********************* EOF *******************/

