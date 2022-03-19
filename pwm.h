#ifndef PWM_H
#define PWM_H

	/* Includes ------------------------------------------------------------------*/
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"
	
	/* Defines -------------------------------------------------------------------*/
	#define F_PWM_SOUND		0x00000001U
	#define PWM_PIN				GPIO_PIN_3
	#define PWM_TIM				TIM2
	#define PWM_CHANNEL		TIM_CHANNEL_4
	
	/* Exported functions ------------------------------------------------------- */
	extern void Init_GPIO_PWM(void);																// Init GPIO
	extern int  Init_PWM(void);																			// Init PWM module

	/* Exported variables --------------------------------------------------------*/
	extern osThreadId_t tid_Th_PWM;                									// Thread id

#endif
