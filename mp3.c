/* Includes ------------------------------------------------------------------*/
#include "mp3.h"

/* Defines -------------------------------------------------------------------*/
#define F_TX_USART_MP3 0x00000001U																				// Tx flag MP3 USART
#define F_RX_USART_MP3 0x00000002U																				// Rx flag MP3 USART

extern ARM_DRIVER_USART Driver_USART6;

/* Public variables ----------------------------------------------------------*/
osMessageQueueId_t mid_MP3_write;                													// MP3 write message queue id
osMessageQueueId_t mid_MP3_read;                													// MP3 read message queue id

/* Private variables ---------------------------------------------------------*/
static ARM_DRIVER_USART * USARTdrv = &Driver_USART6;											// USART driver
static osThreadId_t tid_Th_MP3_write;																			// MP3 write thread id
static osThreadId_t tid_Th_MP3_read;																			// MP3 read thread id

/* Private function prototypes -----------------------------------------------*/
static void Th_MP3_write(void *argument);																	// MP3 write thread funciton
static void Th_MP3_read(void *argument);																	// MP3 read thread funciton
static void MP3_USART_Callback(uint32_t event);														// MP3 USART call back function


/* Public functions ----------------------------------------------------------*/
//Module initialization function
int Init_MP3(void){
  
  tid_Th_MP3_write = osThreadNew(Th_MP3_write, NULL, NULL);
  if (tid_Th_MP3_write == NULL){
    return(-1);
	}
	
	tid_Th_MP3_read = osThreadNew(Th_MP3_read, NULL, NULL);
  if (tid_Th_MP3_read == NULL){
    return(-1);
	}
  
  mid_MP3_write = osMessageQueueNew(MSGQUEUE_MP3_OBJECTS, sizeof(uint8_t[MAX_CHARACTERS_TX_MSG_MP3]), NULL);
	if (mid_MP3_write == NULL){
    return(-1);
	}
  
	mid_MP3_read = osMessageQueueNew(MSGQUEUE_MP3_OBJECTS, sizeof(uint8_t[MAX_CHARACTERS_RX_MSG_MP3]), NULL);
	if (mid_MP3_read == NULL){
    return(-1);
	}
	
  return(0);
}

//USART initialization function
void Init_USART_MP3(void){
	
  USARTdrv->Initialize(MP3_USART_Callback);
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
static void Th_MP3_write(void *argument){
  
	uint8_t msg[MAX_CHARACTERS_TX_MSG_MP3];
	
  while (1) {
    osMessageQueueGet(mid_MP3_write, &msg, NULL, osWaitForever);
    
		USARTdrv->Send(msg, sizeof(msg));
		osThreadFlagsWait(F_TX_USART_MP3, osFlagsWaitAll, osWaitForever);
  }
}

//Reading thread function
static void Th_MP3_read(void *argument){
  
	uint8_t msg[MAX_CHARACTERS_RX_MSG_MP3];
	
  while (1) {
		USARTdrv->Receive(msg, sizeof(msg));
		osThreadFlagsWait(F_RX_USART_MP3, osFlagsWaitAll, osWaitForever);
		
		osMessageQueuePut(mid_MP3_read, &msg, 0U, 0U);
  }
}

//USART functions
//Call back function
static void MP3_USART_Callback(uint32_t event){
	
	if(event == ARM_USART_EVENT_SEND_COMPLETE)
		osThreadFlagsSet(tid_Th_MP3_write, F_TX_USART_MP3);
	
	if(event == ARM_USART_EVENT_RECEIVE_COMPLETE)
		osThreadFlagsSet(tid_Th_MP3_read, F_RX_USART_MP3);
}
