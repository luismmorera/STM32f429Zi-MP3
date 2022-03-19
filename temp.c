/* Includes ------------------------------------------------------------------*/
#include "temp.h"

/* Defines -------------------------------------------------------------------*/
#define EEPROM_I2C_ADDR       0x48      									// EEPROM I2C address
#define POINTER_BYTE					0x00												// Reading temperature
#define F_TRANSFER_DONE			 	0x00000001U									// I2C transfer completed

extern ARM_DRIVER_I2C Driver_I2C1;

/* Public variables ----------------------------------------------------------*/
float temperatura;                												// Contains system temperature

/* Private variables ---------------------------------------------------------*/
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1; 						// I2C driver
static osThreadId_t tid_Th_TEMP;													// Temperature thread ID

/* Private function prototypes -----------------------------------------------*/
static void Th_TEMP(void *argument);											// TEMP thread funciton
static void TEMP_I2C_Callback(uint32_t event);						// TEMP I2C call back function


/* Public functions ----------------------------------------------------------*/
// Module initialization function
int Init_TEMP(void){	
	
  tid_Th_TEMP = osThreadNew(Th_TEMP, NULL, NULL);
  if (tid_Th_TEMP == NULL){
    return(-1);
	}
	
  return(0);
}

//I2C initialization function
void Init_I2C_TEMP(void){

  I2Cdrv->Initialize(TEMP_I2C_Callback);
  I2Cdrv->PowerControl (ARM_POWER_FULL);
  I2Cdrv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);
}


/* Private functions ---------------------------------------------------------*/
// Thread function
static void Th_TEMP(void *argument){
	
	uint8_t buffer_measure[2];
	uint8_t pointer_register = POINTER_BYTE;
	
	while(1){
		
		osDelay(1000);
		
		I2Cdrv->MasterTransmit(EEPROM_I2C_ADDR, &pointer_register, 1, true);
		osThreadFlagsWait(F_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
		
		I2Cdrv->MasterReceive(EEPROM_I2C_ADDR, buffer_measure, 2, false);
		osThreadFlagsWait(F_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
		
		temperatura = 0.125 * (((int16_t)(buffer_measure[0] * 256) | (int16_t)(buffer_measure[1])) / 32);
	}
}

//I2C call back function
static void TEMP_I2C_Callback(uint32_t event){
	
	if(event == ARM_I2C_EVENT_TRANSFER_DONE)
		osThreadFlagsSet(tid_Th_TEMP, F_TRANSFER_DONE);
}
