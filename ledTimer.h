/*******************************************************************************
 * filename: ledTimer.h
 *
 * Handles the timer functionality for pciLED driver module
 *
 * Written by: James Ross
 ******************************************************************************/
#ifndef _LED_TIMER_H
#define _LED_TIMER_H

#define DFT_BLRATE 2  /* default blink rate in seconds */
#define LED0_ON  0x4E /* assert LED, set invery bit */
#define LED0_OFF 0xF 


            /* function prototypes */
void gbe38v_init_led_timer(void *ledCtlReg);
void gbe38v_remove_led_timer(void);
int gbe38v_timer_blink_rate(void);
#endif
/************** EOF ***************/
