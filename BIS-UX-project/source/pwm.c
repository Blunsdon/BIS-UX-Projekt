
#include "pwm.h"


int pwmInit(uint16_t period) {

	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;	// Clock PTD2 (Not LED)
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;	// Clock TPM0

	//Let's use FAST GPIO
	PORTD->PCR[PTD2] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[PTD2] |= PORT_PCR_MUX(4); 	//Alt. 4 FGPIO

	//Let's use FAST GPIO
	PORTD->PCR[PTD3] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[PTD3] |= PORT_PCR_MUX(4); 	//Alt. 4 FGPIO

	//Config TPM0
	SIM->SOPT2 |= (SIM_SOPT2_TPMSRC(1) | SIM_SOPT2_PLLFLLSEL_MASK); //48 MHz
	TPM0->MOD = period - 1; // MOD counter mod
	TPM0->SC = TPM_SC_PS(2); //count up, div by 2 prescaler
	TPM0->CONF |= TPM_CONF_DBGMODE(3); //Let it work in debug mode
	TPM0->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSA_MASK; //Pwm edge align
	TPM0->CONTROLS[1].CnV = 0; //initial duty cycle
	TPM0->CONTROLS[2].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSA_MASK; //Pwm edge align Active low
	TPM0->CONTROLS[2].CnV = 0; //initial duty cycle
	TPM0->SC |= TPM_SC_CMOD(1);	//START :)
	return 0;
}



int pwm2Init(uint16_t period2) {

	/*
	 * Start TPM2 on channel 1
	 */
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;	// Clock PTB3
	SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK;	// Clock TPM2

	//Let's use FAST GPIO
	PORTB->PCR[PTB3] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[PTB3] |= PORT_PCR_MUX(3); 	//Alt. 3 TMP2 ch [1]

	//Config TPM2
	//SIM->SOPT2 |= (SIM_SOPT2_TPMSRC(1) | SIM_SOPT2_PLLFLLSEL_MASK); //48 MHz
	TPM2->MOD = period2 - 1; // MOD counter mod
	TPM2->SC = TPM_SC_PS(4); //count up, div by 16 prescaler
	TPM2->CONF |= TPM_CONF_DBGMODE(3); //Let it work in debug mode
	TPM2->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK; //Pwm edge align Active high
	TPM2->CONTROLS[1].CnV = 0; //initial duty cycle
	TPM2->SC |= TPM_SC_CMOD(1);	//START :)
	return 0;
}


