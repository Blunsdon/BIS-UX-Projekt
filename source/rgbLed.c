/*
 * rgbLed.c
 *
 *  Created on: 12 Sep 2018
 *      Author: au263437
 */
#include "pin_mux.h"
#include "clock_config.h"
#include "rgbLed.h"

#define MASK(x) 	(1 << (x))

void init_rgb(void) {
	/* Setup GPIO pins */
	/* enable clocks */
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;

	/* setup port multiplexing */
	PORTB->PCR[RED_LED_SHIFT] &= PORT_PCR_MUX_MASK;
	PORTB->PCR[RED_LED_SHIFT] |= PORT_PCR_MUX(1);
	PORTB->PCR[GREEN_LED_SHIFT] &= PORT_PCR_MUX_MASK;
	PORTB->PCR[GREEN_LED_SHIFT] |= PORT_PCR_MUX(1);
	PORTD->PCR[BLUE_LED_SHIFT] &= PORT_PCR_MUX_MASK;
	PORTD->PCR[BLUE_LED_SHIFT] |= PORT_PCR_MUX(1);

	/* set as outputs*/
	PTB->PDDR |= MASK(RED_LED_SHIFT) | MASK(GREEN_LED_SHIFT);
	PTD->PDDR |= MASK(BLUE_LED_SHIFT);
}

void set_rgd(rgb_color_t col) {
	switch (col) {
	case BLACK:
		RED_OFF;
		GREEN_OFF;
		BLUE_OFF;
		break;
	case WHITE:
		RED_ON;
		GREEN_ON;
		BLUE_ON;
		break;
	case RED:
		RED_ON;
		GREEN_OFF;
		BLUE_OFF;
		break;
	case GREEN:
		RED_OFF;
		GREEN_ON;
		BLUE_OFF;
		break;
	case BLUE:
		RED_OFF;
		GREEN_OFF;
		BLUE_ON;
		break;
	case YELLOW:
		RED_ON;
		GREEN_ON;
		BLUE_OFF;
		break;
	case CYAN:
		RED_OFF;
		GREEN_ON;
		BLUE_ON;
		break;
	case MAGENTA:
		RED_ON;
		GREEN_OFF;
		BLUE_ON;
		break;
	}

}

