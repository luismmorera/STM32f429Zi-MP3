/* Includes ------------------------------------------------------------------*/
#include "pwm.h"

/* Public variables ----------------------------------------------------------*/
osThreadId_t tid_Th_PWM;                																// PWM thread id

/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static void Th_PWM(void *argument);																			// PWM thread funciton
static void Init_TIM(TIM_HandleTypeDef *tim);														// Timer initialization function

/* Public functions ----------------------------------------------------------*/
//Module initialization function
int Init_PWM(void){
	
	tid_Th_PWM = osThreadNew(Th_PWM, NULL, NULL);
  if (tid_Th_PWM == NULL){
    return(-1);
	}
  
  return(0);
}

//GPIO initialization function
 void Init_GPIO_PWM(void){
	
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitStruct.Pin 				= PWM_PIN;
	GPIO_InitStruct.Mode 				= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate 	= GPIO_AF1_TIM2;
	GPIO_InitStruct.Speed			  = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Pull 				= GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
 

/* Private functions ---------------------------------------------------------*/
//Thread function
static void Th_PWM(void *argument){
	
	TIM_HandleTypeDef tim2;
	Init_TIM(&tim2);
	
	while(1){
		
		osThreadFlagsWait(F_PWM_SOUND, osFlagsWaitAll, osWaitForever);
		
		HAL_TIM_PWM_Start(&tim2, PWM_CHANNEL);
		osDelay(150);
		HAL_TIM_PWM_Stop(&tim2, PWM_CHANNEL);
	}
}

static void Init_TIM(TIM_HandleTypeDef *tim){
	
	TIM_OC_InitTypeDef oc;
	__HAL_RCC_TIM2_CLK_ENABLE();
	
	tim->Instance 			= PWM_TIM;
	tim->Init.Prescaler = 839;
	tim->Init.Period 		= 199;
	
	oc.OCMode 		= TIM_OCMODE_PWM1;
	oc.OCPolarity = TIM_OCPOLARITY_HIGH;
	oc.OCFastMode	= TIM_OCFAST_DISABLE;
	oc.Pulse 			= 100;
	
	HAL_TIM_PWM_Init(tim);
	HAL_TIM_PWM_ConfigChannel(tim, &oc, PWM_CHANNEL);
	
}
