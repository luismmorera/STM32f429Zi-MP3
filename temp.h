#ifndef TEMP_H
#define TEMP_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	#include "Driver_I2C.h"
	
	/* Defines -------------------------------------------------------------------*/
	#define TEMP_SCL 				GPIO_PIN_8
	#define TEMP_SDA 				GPIO_PIN_9
	
	/* Exported functions ------------------------------------------------------- */
	extern int Init_TEMP(void);																			// Init Clock module
	extern void Init_I2C_TEMP(void);																// Init I2C and GPIO
	
	/* Exported variables --------------------------------------------------------*/
	extern float temperatura;                												// Contains system temperature
	
#endif
