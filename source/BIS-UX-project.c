
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

int i;

// definerer 5 state tilstande
typedef enum {
	on, off
} state_t;
state_t State_sound = off, state_LED = off, state_starting = off, state_Buzz = off, state_standby = on;

// use for task create
TaskHandle_t LED = NULL; // LED game
TaskHandle_t Sound = NULL; // Sound game
TaskHandle_t Buzz = NULL; // Buzz game (vibrator)
TaskHandle_t Starting = NULL; // starting up seq
TaskHandle_t Standby = NULL; // Standby

TaskHandle_t state = NULL; // state for games, seq and result

TaskHandle_t OK_sound = NULL; // 3 Hz time counter for LED
TaskHandle_t Warring_sound = NULL; // 1 Hz time counter for Sound
TaskHandle_t Starting_sound = NULL; // 0,5 Hz time counter for Feel
TaskHandle_t Error_sound = NULL; // 0,1 HZ

// LED game wait 3000 ticks then turn on the BLUE LED
void LED_handler(void *p) {
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(3000)); //1,5 Hz
		if (state_LED == on) {

			set_rgd(BLUE);
			//PTB->PCOR = MASK(PTB19); // Set BLUE for LED
		}
	}
}

//Sound game wait 3000 ticks then make sound
void Sound_handler(void *p) {
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(3000)); //1,5 Hz
		if (State_sound == on) {
			//TPM0->MOD = PWM_PERIOD - 1;
			TPM0->CNT = 1;
			TPM0->CONTROLS[2].CnV = 1000;
			//wait a while
			vTaskDelay(pdMS_TO_TICKS(45)); 
			TPM0->CONTROLS[2].CnV = PWM_PERIOD;
			//wait a while
		}
	}
}


// Buzz game wait 3000 ticks the vibrate
void Buzz_handler(void *p) {
	vTaskDelay(pdMS_TO_TICKS(3000)); //0,3 Hz
	while (1) {
		if (state_Buzz == on) {
			PTB->PSOR = MASK(PTB0); // Set buzz (vibrator)
		}
		vTaskDelay(pdMS_TO_TICKS(10000)); //0,3 Hz
	}
}

// Pre-starting state runs for 2500 ticks then suspends
void Starting_handler(void *p) {
	while (1) {
		if (state_starting == on) {
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
		}
		//vTaskSuspend(NULL); // suspend until IRQ
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



// state handler
void state_handler(void *p) {
	while (1) {

		if (state_starting == on) {
			state_starting = off;
			vTaskSuspend( Starting_sound ); // suspend sound
			vTaskSuspend( Starting ); // suspend starting_handler
			PTB->PCOR = MASK(PTB0); // Clear buzz
			state_LED = on;
		} else if (State_sound == on) {
			set_rgd(BLACK);
			State_sound = off;
			state_Buzz = on;
		} else if (state_Buzz == on) {
			set_rgd(BLACK);
			state_Buzz = off;
			PTB->PCOR = MASK(PTB0); // Clear buzz
			state_standby = on;
			// start Task state_handler
		} else if (state_LED == on) {
			state_LED = off;
			set_rgd(BLACK);
			State_sound = on;
		} else if (state_standby == on){
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
		}
		vTaskSuspend(NULL); // suspend until IRQ
	}
}

void PORTD_IRQHandler(void) {
	int i;
	int z = 0;
	if (PORTD->ISFR && MASK(PTD7)) {
		//debounce
		for (i = 0; i < 1480000; i++) {
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




/*
void Warring_sound_handler(void *p) {
	while (1) {
		if (State_sound == on) {
			//TPM0->MOD = PWM_PERIOD - 1;
			TPM0->CNT = 1;
			TPM0->CONTROLS[2].CnV = 1000;
			//wait a while
			vTaskDelay(pdMS_TO_TICKS(45)); //1,5 Hz
			TPM0->CONTROLS[2].CnV = PWM_PERIOD;
			//wait a while
		}
		vTaskDelay(pdMS_TO_TICKS(1355)); //1,5 Hz
	}
}
*/

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

/*
void Error_sound_handler(void *p) {
	while (1) {
		if (state_starting == on) {
			//TPM0->MOD = PWM_PERIOD - 1;
			TPM0->CNT = 1;
			TPM0->CONTROLS[2].CnV = 2000;
			//wait a while
			//vTaskDelay(pdMS_TO_TICKS(300)); //1,5 Hz
			TPM0->CONTROLS[2].CnV = 1000;
			//wait a while
		}
		vTaskDelay(pdMS_TO_TICKS(600)); //1,5 Hz
	}
}
*/

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
	xTaskCreate(LED_handler, "LED", configMINIMAL_STACK_SIZE + 90, (void*) 0,
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
/*
	xTaskCreate(Warring_sound_handler, "Warring_sound",
	configMINIMAL_STACK_SIZE + 100, (void*) 0,
	configMAX_PRIORITIES - 3, &Warring_sound);
*/
	xTaskCreate(Starting_sound_handler, "Starting_sound",
	configMINIMAL_STACK_SIZE + 100, (void*) 0,
	configMAX_PRIORITIES - 3, &Starting_sound);
/*
	xTaskCreate(Error_sound_handler, "Error_sound",
	configMINIMAL_STACK_SIZE + 100, (void*) 0,
	configMAX_PRIORITIES - 3, &Error_sound);
*/

	// Start Scheduler
	vTaskStartScheduler();

	return 0;
}
