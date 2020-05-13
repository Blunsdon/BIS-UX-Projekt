/**
 * @file pwm.h
 * @brief E3ISD1 Week 46: Set up PWM
 * @date E3ISD1 2019
 * @author Janus Bo Andersen
 *
 */


#ifndef PWM_H_
#define PWM_H_

#include "MKL25Z4.h"  //also includes the types we need...

#define BLUE_LED_POS 1 // PTD1 and blue onboard
#define PTD3 3 // PTD3 for sound
#define PTD2 2 // TPM0 (not Blue)
#define PTB3 3 // TPM2



// init TPM0
int pwmInit(uint16_t period);


// Init TPM1
int pwm2Init(uint16_t period2);

#endif /* PWM_H_ */
