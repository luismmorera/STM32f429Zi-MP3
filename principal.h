#ifndef PRINCIPAL_H
#define PRINCIPAL_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	#include <stdio.h>
	#include <string.h>
	#include "lcd.h"
	#include "joystick.h"
	#include "clock.h"
	#include "com.h"
	#include "vol.h"
	#include "mp3.h"
	#include "led.h"
	#include "temp.h"
	#include "pwm.h"

	/* Exported functions ------------------------------------------------------- */
	extern int Init_MAIN(void);																		// Init main module

#endif
