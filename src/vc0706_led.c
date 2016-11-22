/*******************************************************************************
** File: vc0706_led.c
**
** Purpose:
**   This file contains functions to enable, disable, and toggle the LED for the VC0706 application.
**
**
*******************************************************************************/

// Requires WiringPi
// #include <wiringPi.h>

#include "vc0706_led.h"

/**
 * Initializes the GPIO interface for the LED
 * \param[in,out] led - A pointer to the led struct to store this connection in
 * \param[in] pin - The GPIO pin the LED is attached to
 */
int led_init(led_t *led, int pin)
{
    if (wiringPiSetup() == -1)
    {
		// TODO: change to a CFE message
		OS_printf("LED: wiringPiSetup() Failed!\n");
		return -1;
    }
    led->led_pin = pin;
    pinMode(pin, OUTPUT);
    led_off(led);
    return 0;
}

/**
 * Turns on the specified LED.
 * \param[in,out] - A pointer to the LED to enable
 */
void led_on(led_t *led)
{
    digitalWrite(led->led_pin, HIGH);
    led->led_status = 1;
}

/**
 * Turns off the specified LED.
 * \param[in, out] - A pointer to the LED to disable
 */
void led_off(led_t *led)
{
    digitalWrite(led->led_pin, LOW);
    led->led_status = 0;
}
