#ifndef COM_H
#define COM_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	#include "Driver_USART.h"
	#include <stdio.h>
	#include <string.h>
	
	/* Defines -------------------------------------------------------------------*/
	#define MSGQUEUE_COM_OBJECTS 				16             							// Number of Message Queue Objects
	#define MAX_CHARACTERS_TX_MSG_COM		64													// Number of writing message max characters
	#define MAX_CHARACTERS_RX_MSG_COM		13													// Number of reading message max characters
	
	/* Exported functions ------------------------------------------------------- */
	extern void Init_USART_COM(void);																// Init USART
	extern int Init_COM(void);																			// Init COM module
	
	/* Exported variables --------------------------------------------------------*/
	extern osMessageQueueId_t mid_COM_write;                				// Writing message queue id
	extern osMessageQueueId_t mid_COM_read;	                				// Reading message queue id
	
#endif
