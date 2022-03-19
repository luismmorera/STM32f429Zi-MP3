#ifndef _JOYSTICK_H
#define _JOYSTICK_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	
	/* Defines -------------------------------------------------------------------*/
	#define F_IRQ_JOY 	0x00000001U														// Flag IRQ joystick
	#define MSGQUEUE_JOY_OBJECTS 			16                     	// Number of Message Queue Objects
	#define DEBOUNCE_TIMER_TICKS			50
	#define LONG_PULSATION_TICKS			1000
	#define JOY_UP										GPIO_PIN_10
	#define JOY_RIGHT									GPIO_PIN_11
	#define JOY_DOWN									GPIO_PIN_12
	#define JOY_LEFT									GPIO_PIN_14
	#define JOY_CENTRE								GPIO_PIN_15
	
	/* Exported functions ------------------------------------------------------- */
	extern int Init_JOY(void);																// Init joystick module
	extern void Init_GPIO_JOY(void);													// Init GPIO needed for joystick
	extern void EXTI15_10_IRQHandler(void);
	extern void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
	
	/* Exported variables --------------------------------------------------------*/
	extern osThreadId_t tid_Th_JOY_IRQ;                     	// IRQ thread id
	extern osMessageQueueId_t mid_JOY;                				// Message queue id
	
#endif
