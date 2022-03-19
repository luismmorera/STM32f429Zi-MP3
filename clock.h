#ifndef CLOCK_H
#define CLOCK_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	
	/* Exported functions ------------------------------------------------------- */
	extern int Init_CLK(void);																			// Init Clock module
	
	/* Exported variables --------------------------------------------------------*/
	extern uint8_t segundos;                												// Contains system seconds
	extern uint8_t minutos;                													// Contains system minutes
	extern uint8_t horas;                														// Contains system hours
	
#endif
