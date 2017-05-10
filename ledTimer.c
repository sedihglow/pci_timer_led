/*******************************************************************************
 * filename: ledTimer.c
 *
 * Handles the timer functionality for pciLED driver module
 *
 * Written by: James Ross
 ******************************************************************************/

#include "ledTimer.h"

#define T_OFF 0x0
#define T_ON  0x1
#define T_DEL 0x3

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

static void gbe38v_toggle_led(void __iomem *ledReg)
{
    unsigned long regVal;
    unsigned long toWrite;

    /* read the current value of the LED control register, set toWrite */
    regVal = ioread32(ledReg);
    
    /* write the appropriate bits to toggle LED */
    if(regVal == LED0_ON){
        toWrite = regVal & ~LED0_ON; /* clear LED_ON bits */
        toWrite |= LED0_OFF;
        iowrite32(toWrite, ledReg);
    }
    else{ /* LED_OFF */
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
int gbe38v_init_led_timer(void __iomem *ledCtlReg)
{
    gbe38v_update_timer(&(ledTimerData.blinkTime), &(ledTimerData.jiff));
    setup_timer(&openTimer, gbe38v_openBlink_cb, (unsigned long)&ledTimerData);

    /* set timerData and ensure LED register is set to LED_OFF */
    ledTimerData.ledCtlReg = ledCtlReg;
    iowrite32(LED0_OFF, ledCtlReg);

    /* if param gets set to 0, do not blink, do not start timer. */
    if(unlikely(blinkRate_g == 0)){
        printk(KERN_WARNING "pciLED: Blink rate has been set to 0, no blink\n");
        return SUCCESS;
    }
    else if(unlikely(blinkRate_g < 0)){
        printk(KERN_WARNING "pciLED: Blink rate paramater changed to negetive,"
               "Invalid parameter.\n");
        return -EINVAL;
    }

    mod_timer(&openTimer, (jiffies + ((ledTimerData.blinkTime) * HZ)));
    return SUCCESS;
}/* end gbe38v_init_timer */

int gbe38v_timer_blink_rate(void)
{
    return blinkRate_g;
}/* end get_blink_rate */

void gbe38v_set_timer_blink_rate(int blinkRate)
{
    blinkRate_g = blinkRate;
}/* end gbe38v_set_timer_blink_rate */

void gbe38v_remove_led_timer(void)
{
    iowrite32(LED0_OFF, ledTimerData.ledCtlReg);
    del_timer_sync(&openTimer);
}/* end gbe38v_remove_led_timer */
/************** EOF ***************/
