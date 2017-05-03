/*******************************************************************************
 * filename: ledTimer.c
 *
 * Handles the timer functionality for pciLED driver module
 *
 * Written by: James Ross
 ******************************************************************************/

#include "ledTimer.h"
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
#include <linux/jiffies.h>

/* blinks-per-second */ 
static int blinkRate_g = DFT_BLRATE; 
module_param(blinkRate_g, int, S_IRUSR | S_IWUSR); 

static struct timer_list openTimer;

/* data for timer init when driver opens */
struct gbe38v_timer_data{
    int blinkTime;
    unsigned long jiff;
    void __iomem *ledCtlReg;
}ledTimerData;

                /* static function declarations */
static inline void gbe38v_update_timer(int *blinkTime, unsigned long *jiff)
{
    *blinkTime = blinkRate_g; 
    *jiff      = jiffies;
}/* end gbe38v_update_timer */

static void gbe38v_toggle_led(void *ledReg)
{
    unsigned long regVal;
    unsigned long toWrite;

    /* read the current value of the LED control register, set toWrite */
    regVal = ioread32(ledReg);
    
    printk(KERN_ALERT "pciLED : in toggle_led, regVal: %ld\n", regVal);
    /* write the appropriate bits to toggle LED */
    if(regVal & LED0_ON){
        printk(KERN_ALERT "pciLED : in LED_ON\n");
        toWrite = regVal & ~LED0_ON; /* clear LED_ON bits */
        toWrite |= LED0_OFF;
        iowrite32(toWrite, ledReg);
    }
    else{ /* LED_OFF */
        printk(KERN_ALERT "pciLED : LED_OFF\n");
        toWrite = regVal & ~LED0_OFF; /* clear LED_OFF bits */
        toWrite |= LED0_ON;
        iowrite32(toWrite, ledReg);
    }
}/* end gbe38v_toggle_led */

static void gbe38v_openBlink_cb(unsigned long data)
{
    struct gbe38v_timer_data *val = (struct gbe38v_timer_data*)data;
    
    /* toggle the led */
    gbe38v_toggle_led(val->ledCtlReg);

    gbe38v_update_timer(&(val->blinkTime), &(val->jiff));
    mod_timer(&openTimer, (jiffies + (val->blinkTime * HZ)));
}/* end gbe38v_timer_ledToggle */

                /* header function declarations */
void gbe38v_init_led_timer(void *ledCtlReg)
{
    gbe38v_update_timer(&(ledTimerData.blinkTime), &(ledTimerData.jiff));
    setup_timer(&openTimer, gbe38v_openBlink_cb, (unsigned long)&ledTimerData);
    /* set timerData and ensure LED register is set to LED_OFF */
    ledTimerData.ledCtlReg = ledCtlReg;
    iowrite32(LED0_OFF, ledCtlReg);
    mod_timer(&openTimer, (jiffies + ((ledTimerData.blinkTime) * HZ)));
}/* end gbe38v_init_timer */

int get_blink_rate(void)
{
    return ledTimerData.blinkTime;
}/* end get_blink_rate */

void gbe38v_remove_led_timer(void)
{
    del_timer_sync(&openTimer);
}/* end gbe38v_remove_led_timer */
/************** EOF ***************/
