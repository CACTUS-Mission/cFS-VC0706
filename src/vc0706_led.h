/*******************************************************************************
** File: vc0706_led.h
**
** Purpose:
**   This file contains functions to enable, disable, and toggle the LED for the VC0706 application.
**
**
*******************************************************************************/

// Requires WiringPi
// #include <wiringPi.h>

#ifndef _vc0706_led_h_
#define _vc0706_led_h_

#include "vc0706.h"

/**
 * An LED connected to the Raspberry Pi via GPIO. Used as a camera flash
 */
typedef struct led_t {
	int led_pin; 	/**< The GPIO pin this LED is attached to */
	bool led_status;	/**< Whether the LED is on or not */
} led_t;


int led_init(led_t *led, int pin);

void led_on(led_t *led);

void led_off(led_t *led);


#endif /* _vc0706_flash_h_ */
