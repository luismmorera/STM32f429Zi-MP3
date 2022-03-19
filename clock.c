/* Includes ------------------------------------------------------------------*/
#include "clock.h"

/* Public variables ----------------------------------------------------------*/
uint8_t segundos;                													// Contains system seconds
uint8_t minutos;                													// Contains system minutes
uint8_t horas;                														// Contains system hours

/* Private variables ---------------------------------------------------------*/
static osTimerId_t timCLK_id;                            	// 1 second timer id

/* Private function prototypes -----------------------------------------------*/
static void Th_CLK(void *argument);												// Clock thread function
static void CLK_Timer_Callback(void const *arg);					// 1 second timer callback

/* Public functions ----------------------------------------------------------*/
// Module initialization function
int Init_CLK(void){
 
	osThreadId_t tid_Th_CLK;
	uint32_t execCLK = 2U;
	
  tid_Th_CLK = osThreadNew(Th_CLK, NULL, NULL);
  if (tid_Th_CLK == NULL) {
    return(-1);
  }
	
  timCLK_id = osTimerNew((osTimerFunc_t)&CLK_Timer_Callback, osTimerPeriodic, &execCLK, NULL);
  if (timCLK_id == NULL) {
      return -1;
  }
	
  return 0;
}


/* Private functions ---------------------------------------------------------*/
// Thread function
static void Th_CLK(void *argument){
	
	osTimerStart(timCLK_id, 1000);
	
	while(1){
		osThreadYield();
	}
}

// Timer callback
static void CLK_Timer_Callback(void const *arg){
  
	segundos++;
	
	if(segundos > 59){
		segundos = 0;
		minutos++;
	}
	
	if(minutos > 59){
		minutos = 0;
		horas++;
	}
	
	if(horas > 23){
		horas = 0;
	}
}
