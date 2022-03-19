#ifndef LED_H
#define LED_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	#include <stdio.h>
	
	/* Defines -------------------------------------------------------------------*/
	#define LED_RED 							GPIO_PIN_13
	#define LED_GREEN 						GPIO_PIN_12
	#define LED_BLUE 							GPIO_PIN_11
	#define LED_RED_ON						0x01
	#define LED_GREEN_ON					0x02
	#define LED_BLUE_ON						0x04
	#define MSGQUEUE_LED_OBJECTS 	16              									// Number of Message Queue Objects

	/* Exported functions ------------------------------------------------------- */
	extern void Init_GPIO_LED(void);																// Init GPIO
	extern int  Init_LED(void);																			// Init LED module

	/* Exported variables --------------------------------------------------------*/
	extern osMessageQueueId_t mid_LED;                							// Message queue id

#endif
