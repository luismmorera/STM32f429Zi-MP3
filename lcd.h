#ifndef LCD_H
#define LCD_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	#include <stdio.h>
	#include <string.h>
	
	/* Defines -------------------------------------------------------------------*/
	#define LCD_A0 								GPIO_PIN_13
	#define LCD_RST 							GPIO_PIN_6
	#define LCD_CS 								GPIO_PIN_14
	#define MSGQUEUE_LCD_OBJECTS 	16              									// Number of Message Queue Objects
	#define MAX_CHARACTERS_LCD		64																// Max number of characters to write

	/* Typedef -------------------------------------------------------------------*/
	typedef struct {
		
		char cadena[MAX_CHARACTERS_LCD];															// String to write
		uint8_t linea;																								// Line to write bit: 1 -> line 1
																																	//										2 -> line 2
	} MSGQUEUE_LCD_OBJ_t;																						// Object to be sent through the message queue

	/* Exported functions ------------------------------------------------------- */
	extern void Init_GPIO_SPI_LCD(void);														// Init SPI1 and GPIO
	extern int  Init_LCD(void);																			// Init LCD module

	/* Exported variables --------------------------------------------------------*/
	extern osMessageQueueId_t mid_LCD;                							// Message queue id

#endif
