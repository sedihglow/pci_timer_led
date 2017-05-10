/*******************************************************************************
 * filename: ledTimer.h
 *
 * Handles the timer functionality for pciLED driver module
 *
 * Written by: James Ross
 ******************************************************************************/
#ifndef _LED_TIMER_H
#define _LED_TIMER_H

#define SUCCESS 0

#define DFT_BLRATE 2  /* default blink rate in seconds */
#define LED0_ON  0x4E /* assert LED, set invery bit */
#define LED0_OFF 0xF 

#include <linux/module.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/types.h>
#include <asm/io.h>
#include <linux/jiffies.h>

            /* function prototypes */
int gbe38v_init_led_timer(void __iomem *ledCtlReg);
void gbe38v_remove_led_timer(void);
int gbe38v_timer_blink_rate(void);
void gbe38v_set_timer_blink_rate(int blinkRate);
#endif
/************** EOF ***************/
