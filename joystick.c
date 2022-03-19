/* Includes ------------------------------------------------------------------*/
#include "joystick.h"

/* Defines -------------------------------------------------------------------*/
#define F_TIMER_JOY_DEBOUNCE 	0x00000002U								// Flag debounce timer joystick
#define F_TIMER_JOY_1SECOND 	0x00000004U								// Flag 1 second timer joystick

/* Typedef -------------------------------------------------------------------*/
	typedef enum{
		off,
		on_short,
		on_long
	}state;																								// States of the automaton

/* Public variables ----------------------------------------------------------*/
osThreadId_t tid_Th_JOY_IRQ;                     				// IRQ thread id
osMessageQueueId_t mid_JOY;                							// Message queue id

/* Private variables ---------------------------------------------------------*/
static osThreadId_t tid_Th_JOY;                     		// Joystick thread id
static osTimerId_t  timJOY_short_id;                    // Debounce timer id
static osTimerId_t  timJOY_long_id;                    	// 1 second timer id
static uint8_t bits_aux;																// Auxiliar variable for the automaton
	
/* Private function prototypes -----------------------------------------------*/
static void Th_JOY(void *argument);                			// Joystick thread funciton
static void Th_JOY_IRQ(void *argument);                	// IRQ thread funciton
static void JOY_short_Timer_Callback(void const *arg);	// Short timer callback
static void JOY_long_Timer_Callback(void const *arg);		// Short timer callback
static uint8_t readJoystick(void);											// Reading joystick function

/* Public functions ----------------------------------------------------------*/
//Module initialization function
int Init_JOY(void){
	
	uint32_t execJOY = 1U;
	
	tid_Th_JOY = osThreadNew(Th_JOY, NULL, NULL);
  if (tid_Th_JOY == NULL){
    return(-1);
  }
	
	tid_Th_JOY_IRQ = osThreadNew(Th_JOY_IRQ, NULL, NULL);
  if (tid_Th_JOY_IRQ == NULL){
    return(-1);
  }
	
	mid_JOY = osMessageQueueNew(MSGQUEUE_JOY_OBJECTS, sizeof(uint8_t), NULL);
  if (mid_JOY == NULL){
    return(-1);
  }
	
  timJOY_short_id = osTimerNew((osTimerFunc_t)&JOY_short_Timer_Callback, osTimerOnce, &execJOY, NULL);
  if (timJOY_short_id == NULL){
    return -1;
  }
	
	timJOY_long_id = osTimerNew((osTimerFunc_t)&JOY_long_Timer_Callback, osTimerOnce, &execJOY, NULL);
  if (timJOY_long_id == NULL){
    return -1;
  }
	
	return 0;
}

//GPIO initialization function
 void Init_GPIO_JOY(void){
	
	GPIO_InitTypeDef GPIO_InitStruct;
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();	
	
	GPIO_InitStruct.Pin 	= JOY_UP | JOY_RIGHT;
	GPIO_InitStruct.Mode 	= GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull 	= GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin	 	= JOY_DOWN | JOY_LEFT | JOY_CENTRE;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}


/* Private functions ---------------------------------------------------------*/
//Threads functions
static void Th_JOY(void *argument){
	
	state estado = off;
	int flags;
	uint8_t msg;																							// Joystick action bits: 			0x01 -> up
																														// 														0x02 -> right
																														//														0x04 -> down
																														// 														0x08 -> left
																														// 														0x10 -> mid
																														// Pulsation type bit 0x80: 	0 -> short
																														// 														1 -> long
	
	while(1){
		
		flags = osThreadFlagsWait(F_TIMER_JOY_DEBOUNCE | F_TIMER_JOY_1SECOND, osFlagsWaitAny, osWaitForever);
		
		switch(estado){
			
			case on_short:
				if(flags == F_TIMER_JOY_DEBOUNCE && bits_aux == 0x00U){
          osTimerStop(timJOY_long_id);
          msg = msg & 0x7F;
          osMessageQueuePut(mid_JOY, &msg, NULL, 0U);
					estado = off;
					
				}else if(flags == F_TIMER_JOY_1SECOND){
          estado = on_long;
          msg = msg | 0x80;
          osMessageQueuePut(mid_JOY, &msg, NULL, 0U);
        }
			break;
			
			case on_long:
				estado = off;
			break;
			
			case off:
				if(flags == F_TIMER_JOY_DEBOUNCE && bits_aux != 0x00U){
					msg = bits_aux;
					estado = on_short;
				}
			break;
		}
	}
}

static void Th_JOY_IRQ(void *argument){
	
	while(1){
		osThreadFlagsWait(F_IRQ_JOY, osFlagsWaitAll, osWaitForever);
		osTimerStart(timJOY_short_id, DEBOUNCE_TIMER_TICKS);
	}
}

//Timer function
static void JOY_short_Timer_Callback(void const *arg){
	bits_aux = readJoystick();
	osThreadFlagsSet(tid_Th_JOY, F_TIMER_JOY_DEBOUNCE);
	
	if(bits_aux != 0x00U)
    osTimerStart(timJOY_long_id, LONG_PULSATION_TICKS);
}

static void JOY_long_Timer_Callback(void const *arg){
	osThreadFlagsSet(tid_Th_JOY, F_TIMER_JOY_1SECOND);
}

//Reading joystick function
static uint8_t readJoystick(void){
  
  if(HAL_GPIO_ReadPin(GPIOB, JOY_UP) == 1)
    return 0x01U;
  if(HAL_GPIO_ReadPin(GPIOB, JOY_RIGHT) == 1)
    return 0x02U;
  if(HAL_GPIO_ReadPin(GPIOE, JOY_DOWN) == 1)
    return 0x04U;
  if(HAL_GPIO_ReadPin(GPIOE, JOY_LEFT) == 1)
    return 0x08U;
  if(HAL_GPIO_ReadPin(GPIOE, JOY_CENTRE) == 1)
    return 0x10U;
  
  return 0x00U;
}

//IRQ functions
void EXTI15_10_IRQHandler(void){
	
	HAL_GPIO_EXTI_IRQHandler(JOY_UP | JOY_RIGHT | JOY_DOWN | JOY_LEFT | JOY_CENTRE);
}
	
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	
	osThreadFlagsSet(tid_Th_JOY_IRQ, F_IRQ_JOY);
}
