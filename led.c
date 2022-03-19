/* Includes ------------------------------------------------------------------*/
#include "led.h"

/* Public variables ----------------------------------------------------------*/
osMessageQueueId_t mid_LED;                													// LED message queue id

/* Private function prototypes -----------------------------------------------*/
static void Th_LED(void *argument);                									// LED thread funciton

/* Public functions ----------------------------------------------------------*/
//Module initialization function
int Init_LED(void){
	
	osThreadId_t tid_Th_LED;
	
  tid_Th_LED = osThreadNew(Th_LED, NULL, NULL);
  if (tid_Th_LED == NULL){
    return(-1);
  }
	
	mid_LED = osMessageQueueNew(MSGQUEUE_LED_OBJECTS, sizeof(uint8_t), NULL);
  if (mid_LED == NULL){
    return(-1);
  }
	
	return 0;
}

//GPIO initialization function
 void Init_GPIO_LED(void){
	
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	GPIO_InitStruct.Pin 	= LED_RED | LED_GREEN | LED_BLUE;
	GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull 	= GPIO_PULLUP;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOD, LED_RED, 	GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, LED_GREEN,	GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, LED_BLUE, 	GPIO_PIN_SET);
}
 
/* Private functions ---------------------------------------------------------*/
//Thread function
static void Th_LED(void *argument){
	
	uint8_t msg;
	
  while (1) {
    osMessageQueueGet(mid_LED, &msg, NULL, osWaitForever);
    
		msg & 0x01 ? HAL_GPIO_WritePin(GPIOD, LED_RED, 		GPIO_PIN_RESET) : HAL_GPIO_WritePin(GPIOD, LED_RED, 	GPIO_PIN_SET);
		msg & 0x02 ? HAL_GPIO_WritePin(GPIOD, LED_GREEN, 	GPIO_PIN_RESET) : HAL_GPIO_WritePin(GPIOD, LED_GREEN, GPIO_PIN_SET);
		msg & 0x04 ? HAL_GPIO_WritePin(GPIOD, LED_BLUE, 	GPIO_PIN_RESET) : HAL_GPIO_WritePin(GPIOD, LED_BLUE, 	GPIO_PIN_SET);	
	}
}
