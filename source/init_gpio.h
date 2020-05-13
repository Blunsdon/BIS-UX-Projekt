/*
 * init_gpio.h
 *
 *  Created on: 19. feb. 2020
 *      Author: Bluns
 */

#ifndef INIT_GPIO_H_
#define INIT_GPIO_H_

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "timers.h"

#define MASK(x) 	(1 << (x)) //mask
#define PTB0 (0) // PTB0 Output(LED)
#define PTB1 (1) // PTB1 Input (button)
#define PTD7 (7) // PTD7 Input interrup (button)


void init_gipo(void);


#endif /* INIT_GPIO_H_ */
