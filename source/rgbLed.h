/*
 * rgbLed.h
 *
 *  Created on: 12 Sep 2018
 *      Author: au263437
 */
#include <MKL25Z4.H>

#ifndef RGBLED_H_
#define RGBLED_H_

#define RED_LED_SHIFT 	18	//Port B18 red led
#define GREEN_LED_SHIFT 19	//Port B19 green led
#define BLUE_LED_SHIFT 	1	//Port D1 blue led

#define RED_ON		GPIOB->PCOR = (1 << RED_LED_SHIFT)
#define RED_OFF		GPIOB->PSOR = (1 << RED_LED_SHIFT)
#define GREEN_ON	GPIOB->PCOR = (1 << GREEN_LED_SHIFT)
#define GREEN_OFF	GPIOB->PSOR = (1 << GREEN_LED_SHIFT)
#define BLUE_ON		GPIOD->PCOR = (1 << BLUE_LED_SHIFT)
#define BLUE_OFF	GPIOD->PSOR = (1 << BLUE_LED_SHIFT)


typedef enum
{
	BLACK,
	WHITE,
	RED,
	GREEN,
	BLUE,
	YELLOW,
	CYAN,
	MAGENTA
}rgb_color_t;

void init_rgb(void);

void set_rgd(rgb_color_t col);

#endif /* RGBLED_H_ */
