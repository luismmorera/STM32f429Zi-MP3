/* Includes ------------------------------------------------------------------*/
#include "com.h"

/* Defines -------------------------------------------------------------------*/
#define F_TX_USART_COM 0x00000001U																				// Tx flag COM USART
#define F_RX_USART_COM 0x00000002U																				// Rx flag COM USART

extern ARM_DRIVER_USART Driver_USART3;

/* Public variables ----------------------------------------------------------*/
osMessageQueueId_t mid_COM_write;                													// COM write message queue id
osMessageQueueId_t mid_COM_read;                													// COM read message queue id

/* Private variables ---------------------------------------------------------*/
static ARM_DRIVER_USART* USARTdrv = &Driver_USART3;												// USART driver
static osThreadId_t tid_Th_COM_write;																			// COM write thread id
static osThreadId_t tid_Th_COM_read;																			// COM read thread id

/* Private function prototypes -----------------------------------------------*/
static void Th_COM_write(void *argument);																	// COM write thread funciton
static void Th_COM_read(void *argument);																	// COM read thread funciton
static void COM_USART_Callback(uint32_t event);														// COM USART call back function


/* Public functions ----------------------------------------------------------*/
//Module initialization function
int Init_COM(void) {
  
  tid_Th_COM_write = osThreadNew(Th_COM_write, NULL, NULL);
  if (tid_Th_COM_write == NULL){
    return(-1);
	}
	
	tid_Th_COM_read = osThreadNew(Th_COM_read, NULL, NULL);
  if (tid_Th_COM_read == NULL){
    return(-1);
	}
  
  mid_COM_write = osMessageQueueNew(MSGQUEUE_COM_OBJECTS, sizeof(uint8_t[MAX_CHARACTERS_TX_MSG_COM]), NULL);
  if (mid_COM_write == NULL){
    return(-1);
  }
	
	mid_COM_read = osMessageQueueNew(MSGQUEUE_COM_OBJECTS, sizeof(uint8_t[MAX_CHARACTERS_RX_MSG_COM]), NULL);
  if (mid_COM_read == NULL){
    return(-1);
  }
	
  return(0);
}

//USART initialization function
void Init_USART_COM(void){
	
  USARTdrv->Initialize(COM_USART_Callback);
  USARTdrv->PowerControl(ARM_POWER_FULL);
  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                    ARM_USART_DATA_BITS_8 |
                    ARM_USART_PARITY_NONE |
                    ARM_USART_STOP_BITS_1 |
                    ARM_USART_FLOW_CONTROL_NONE, 9600);
   
  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
  USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
}


/* Private functions ---------------------------------------------------------*/
//Writing thread function
static void Th_COM_write(void *argument){
  
	uint8_t msg[MAX_CHARACTERS_TX_MSG_COM];
	
  while (1) {
    osMessageQueueGet(mid_COM_write, &msg, NULL, osWaitForever);
    
		USARTdrv->Send(msg, sizeof(msg));
		osThreadFlagsWait(F_TX_USART_COM, osFlagsWaitAll, osWaitForever);
  }
}

//Reading thread function
static void Th_COM_read(void *argument){
  
	uint8_t msg[MAX_CHARACTERS_RX_MSG_COM];
	
  while (1) {
		USARTdrv->Receive(msg, sizeof(msg));
		osThreadFlagsWait(F_RX_USART_COM, osFlagsWaitAll, osWaitForever);
		
		osMessageQueuePut(mid_COM_read, &msg, 0U, 0U);
  }
}


//USART functions
static void COM_USART_Callback(uint32_t event){

	if(event == ARM_USART_EVENT_SEND_COMPLETE)
		osThreadFlagsSet(tid_Th_COM_write, F_TX_USART_COM);
	
	if(event == ARM_USART_EVENT_RECEIVE_COMPLETE)
		osThreadFlagsSet(tid_Th_COM_read, F_RX_USART_COM);
}
