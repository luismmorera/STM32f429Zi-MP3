#ifndef MP3_H
#define MP3_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	#include "Driver_USART.h"
	#include <stdio.h>
	#include <string.h>
	
	/* Defines -------------------------------------------------------------------*/
	#define MSGQUEUE_MP3_OBJECTS 				16         									// Number of Message Queue Objects
	#define MAX_CHARACTERS_TX_MSG_MP3		8														// Number of writing message max characters
	#define MAX_CHARACTERS_RX_MSG_MP3		10													// Number of reading message max characters

	/* Exported functions ------------------------------------------------------- */
	extern void Init_USART_MP3(void);																// Init USART
	extern int  Init_MP3(void);																			// Init MP3 module

	/* Exported variables --------------------------------------------------------*/
	extern osMessageQueueId_t mid_MP3_write;                				// Writing message queue id
	extern osMessageQueueId_t mid_MP3_read;	                				// Reading message queue id

#endif
