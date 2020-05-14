/**
 * @file    BIS-UX-project.c
 * @brief   Application entry point.
 */

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

#include "pwm.h"
#include "init_gpio.h"
#include "rgbLed.h"

#define PWM_PERIOD 24000
#define PWM1_PERIOD 60000 // 48*10^6/(60000/16) = 50Hz
// PTE23 for motor
// PTD2 for sound
// PTD7 for button (interrupt)
// PTB0 for vibrator

#define PTB19 (19) // Green
#define PTD1 (1) // Blue
#define PTB18 (18) // Red

//counter for game
int buzz = 0;
int vib = 0;
int led = 0;
int game_score = 0;


// definerer 5 state tilstande
typedef enum {
	on, off
} state_t;
state_t State_sound = off, state_LED = off, state_starting = off, state_Buzz =
		off, state_standby = on, State_result=off, game = off, end = off;

// use for task create
TaskHandle_t LED = NULL; // LED game
TaskHandle_t Sound = NULL; // Sound game
TaskHandle_t Buzz = NULL; // Buzz game (vibrator)
TaskHandle_t Starting = NULL; // starting up seq
TaskHandle_t Standby = NULL; // Standby
TaskHandle_t Starting_sound = NULL; // Pre-starting game
TaskHandle_t Result_sound = NULL; // Task for result
TaskHandle_t state = NULL; // state for games, seq and result

// LED game wait 3000 ticks then turn on the BLUE LED
void LED_handler(void *p) {
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
		if (state_LED == on) {
			led++;
			vTaskDelay(pdMS_TO_TICKS(1000));
			set_rgd(BLUE);

		}
	}
}

//Sound game wait 3000 ticks then make sound
void Sound_handler(void *p) {
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
		if (State_sound == on) {
			buzz++;
			TPM0->CNT = 1;
			TPM0->CONTROLS[2].CnV = 1000;
			//wait a while
			vTaskDelay(pdMS_TO_TICKS(1145));
			TPM0->CONTROLS[2].CnV = PWM_PERIOD;
			vTaskDelay(pdMS_TO_TICKS(1000));
		}

	}
}

// Buzz game wait 3000 ticks the vibrate
void Buzz_handler(void *p) {
	vTaskDelay(pdMS_TO_TICKS(1000));
	while (1) {
		if (state_Buzz == on) {
			vib ++;
			PTB->PSOR = MASK(PTB0); // Set buzz (vibrator)
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

// Pre-starting state runs for 2500 ticks then suspends
void Starting_handler(void *p) {
	while (1) {
		if (state_starting == on) {
			game = on;
			PTB->PTOR = MASK(PTB0); // toggle buzz
			vTaskDelay(pdMS_TO_TICKS(500)); //0,3 Hz
			set_rgd(RED); // set RED LED
			PTB->PTOR = MASK(PTB0); // toggle buzz
			vTaskDelay(pdMS_TO_TICKS(500)); //0,3 Hz
			set_rgd(BLUE); // Set Blue
			PTB->PTOR = MASK(PTB0); // toggle buzz
			vTaskDelay(pdMS_TO_TICKS(500)); //0,3 Hz
			set_rgd(YELLOW); // Set Yellow
			PTB->PTOR = MASK(PTB0); // toggle buzz
			vTaskDelay(pdMS_TO_TICKS(500)); //0,3 Hz
			set_rgd(MAGENTA); // Set Magenta
			vTaskDelay(pdMS_TO_TICKS(500)); //0,3 Hz
			set_rgd(BLACK);

			state_starting = off;
			vTaskSuspend(Starting_sound); // suspend sound
			PTB->PCOR = MASK(PTB0); // Clear buzz
			state_LED = on;

			vTaskSuspend(NULL); // suspend until IRQ
		}

	}
}

// Standby handler runs with Green LED on infinite
void Standby_handler(void *p) {
	while (1) {
		if (state_standby == on) {
			set_rgd(GREEN); // Set standby LED (GREEN)
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

// Sound for starting state
void Starting_sound_handler(void *p) {
	while (1) {
		if (state_starting == on) {
			//TPM0->MOD = PWM_PERIOD - 1;
			TPM0->CNT = 1;
			TPM0->CONTROLS[2].CnV = 1;
			//wait a while
			vTaskDelay(pdMS_TO_TICKS(45)); //1,5 Hz
			TPM0->CONTROLS[2].CnV = 1000;
			//wait a while
			vTaskDelay(pdMS_TO_TICKS(5)); //1,5 Hz
			TPM0->CONTROLS[2].CnV = 1;
			//wait a while
		}
		vTaskDelay(pdMS_TO_TICKS(30)); //1,5 Hz
	}
}

//
void Result_handler(void *p) {
	while (1) {
		if (game ==on){
			TPM2->CONTROLS[1].CnV = 1500; // -90 deg
		}
		if ((game == off) && (end == off)){
			TPM2->CONTROLS[1].CnV = 7000; // -90 deg
		}
		if (State_result == on) {
			// find en bedre måde
			int s = game_score;
			switch (s) {
				case 3 :
					TPM2->CONTROLS[1].CnV = 6000;
					break;
				case 4 :
					TPM2->CONTROLS[1].CnV = 5000;
					break;
				case 5 :
					TPM2->CONTROLS[1].CnV = 4000;
					break;
				case 6:
				case 7 :
					TPM2->CONTROLS[1].CnV = 3000;
					break;
				default :
					TPM2->CONTROLS[1].CnV = 2200;
					break;
			}

		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

// Handler for all the states
void state_handler(void *p) {
	while (1) {

		if (State_sound == on) {
			set_rgd(BLACK);
			State_sound = off;
			state_Buzz = on;
		} else if (state_Buzz == on) {
			set_rgd(BLACK);
			state_Buzz = off;
			PTB->PCOR = MASK(PTB0); // Clear buzz
			end = on;
			State_result = on;
			game = off;
			game_score = vib+buzz+led;
			vib = 0;
			buzz = 0;
			led = 0;
			// start Task state_handler
		} else if (state_LED == on) {
			state_LED = off;
			set_rgd(BLACK);
			State_sound = on;
		} else if (state_standby == on) {
			//set_rgd(BLACK);
			state_standby = off;
			state_starting = on;
			BaseType_t checkIfYieldRequired0;
			// start Starting_sound
			checkIfYieldRequired0 = xTaskResumeFromISR(Starting);
			portYIELD_FROM_ISR(checkIfYieldRequired0);

			BaseType_t checkIfYieldRequired;
			// start Starting_sound
			checkIfYieldRequired = xTaskResumeFromISR(Starting_sound);
			portYIELD_FROM_ISR(checkIfYieldRequired);
			//vTaskResume(Starting_handler);
		} else if (State_result == on){
			State_result = off;
			game = off;
			end = off;
			state_standby = on;
			game_score = 0;
		}
		vTaskSuspend(NULL); // suspend until IRQ
	}
}

// IRQ handler (button interupt)
void PORTD_IRQHandler(void) {
	int i;
	int z = 0;
	if (PORTD->ISFR && MASK(PTD7)) {
		//debounce
		for (i = 0; i < 3000000; i++) {
			z++;
		}
		// clear pending interrupts
		NVIC_ClearPendingIRQ(PORTD_IRQn);
		PTB->PSOR = MASK(PTB18); // Set Red
		// Basetype
		BaseType_t checkIfYieldRequired;
		// start Task state_handler
		checkIfYieldRequired = xTaskResumeFromISR(state);
		portYIELD_FROM_ISR(checkIfYieldRequired);
		// clear status flags
		PORTD->ISFR = 0xffffffff;
	}
}

int main(void) {

	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();
	init_gipo();
	init_rgb();

	//Initialize PWM using our driver
	pwmInit(PWM_PERIOD);
	pwm2Init(PWM1_PERIOD);

	/* allow interrupts ellers kan man ikke anvende de interrupts man har lavet*/
	__enable_irq();

	// Task function, name of function, stack size (90), arguments, priority (0), Which handler to use
	xTaskCreate(LED_handler, "LED", configMINIMAL_STACK_SIZE + 120, (void*) 0,
	configMAX_PRIORITIES - 4, &LED);

	xTaskCreate(Sound_handler, "Sound", configMINIMAL_STACK_SIZE + 120,
			(void*) 0,
			configMAX_PRIORITIES - 4, &Sound);

	xTaskCreate(Buzz_handler, "Buzz", configMINIMAL_STACK_SIZE + 100, (void*) 0,
	configMAX_PRIORITIES - 4, &Buzz);

	xTaskCreate(Starting_handler, "Starting", configMINIMAL_STACK_SIZE + 120,
			(void*) 0,
			tskIDLE_PRIORITY, &Starting);

	xTaskCreate(state_handler, "state", configMINIMAL_STACK_SIZE + 120,
			(void*) 0,
			configMAX_PRIORITIES, &state);

	xTaskCreate(Standby_handler, "Standby", configMINIMAL_STACK_SIZE + 100,
			(void*) 0,
			configMAX_PRIORITIES - 3, &Standby);

	xTaskCreate(Result_handler, "Result_sound", configMINIMAL_STACK_SIZE + 100,
			(void*) 0, configMAX_PRIORITIES - 3, &Result_sound);

	xTaskCreate(Starting_sound_handler, "Starting_sound",
			configMINIMAL_STACK_SIZE + 100, (void*) 0, configMAX_PRIORITIES - 3,
			&Starting_sound);

	// Start Scheduler
	vTaskStartScheduler();

	return 0;
}
