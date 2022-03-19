#ifndef VOL_H
#define VOL_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	#include <stdio.h>
	#include <string.h>
	
	/* Defines -------------------------------------------------------------------*/
	#define VOL_PIN								GPIO_PIN_0
	#define MSGQUEUE_VOL_OBJECTS 	16              									// Number of Message Queue Objects
	#define READING_VOL_TIME			1000															// Number of ms between readings

	/* Typedef -------------------------------------------------------------------*/

	/* Exported functions ------------------------------------------------------- */
	extern void Init_GPIO_ADC_VOL(void);														// Init ADC and GPIO
	extern int  Init_VOL(void);																			// Init VOL module

	/* Exported variables --------------------------------------------------------*/
	extern osMessageQueueId_t mid_VOL;                							// Message queue id

#endif
