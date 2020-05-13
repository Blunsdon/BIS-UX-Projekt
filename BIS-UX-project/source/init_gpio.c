/*
 * init_gpio.c
 *
 *  Created on: 19. feb. 2020
 *      Author: Bluns
 */


#include "init_gpio.h"


void init_gipo(void) {
	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();

	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; // clock on port B
	PORTB->PCR[PTB0] &= ~PORT_PCR_MUX_MASK; // Ini port PTB0
	PORTB->PCR[PTB0] |= PORT_PCR_MUX(1); // GPIO
	PTB->PDDR |= MASK(PTB0); // 1 for output

	PORTB->PCR[PTB1] &= ~PORT_PCR_MUX_MASK; // Ini port PTB1
	PORTB->PCR[PTB1] |= PORT_PCR_MUX(1); // GPIO
	PTB->PDDR &= ~MASK(PTB1); // 1 invert for output
	// input GIPO med PS(0) pulldown og PE(1) (185)

	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK; // clock on port D
	PORTD->PCR[PTD7] &= ~PORT_PCR_MUX_MASK; // Ini port PTD7
	PORTD->PCR[PTD7] |= PORT_PCR_MUX(1); // GPIO
	PTB->PDDR &= ~MASK(PTD7); // 1 invert for output
	// input GIPO med PS(0) pulldown og PE(1) (185)
	/* Select GPIO and enable pull-up resistors and
	 interrupts on falling edges for pin connected to switch */
	PORTD->PCR[PTD7] |= PORT_PCR_MUX(
			1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0a);

	/* Enable Interrupts */
	NVIC_SetPriority(PORTD_IRQn, 4);
	NVIC_ClearPendingIRQ(PORTD_IRQn);
	NVIC_EnableIRQ(PORTD_IRQn);

}
