#include "vol.h"

/* Defines -------------------------------------------------------------------*/
#define RESOLUTION_12B 	4096U
#define VREF 						3.3f
#define VOL_RANGE 			31.0f

/* Public variables ----------------------------------------------------------*/
osMessageQueueId_t mid_VOL;                																// VOL message queue id

/* Private variables ---------------------------------------------------------*/
static ADC_HandleTypeDef adchandle;

/* Private function prototypes -----------------------------------------------*/
static void Th_VOL(void *argument);
static void ADC1_pins_F429ZI_config(void);
static int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef *ADC_Instance);
static float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel);

/* Public functions ----------------------------------------------------------*/
//Module initialization function
int Init_VOL(void){
  
	osThreadId_t tid_Th_VOL;
	
  tid_Th_VOL = osThreadNew(Th_VOL, NULL, NULL);
  if (tid_Th_VOL == NULL){
    return(-1);
	}
  
  mid_VOL = osMessageQueueNew(MSGQUEUE_VOL_OBJECTS, sizeof(uint8_t), NULL);
	if (mid_VOL == NULL){
    return(-1);
	}
  
  return(0);
}

//GPIO and ADC initialization function
void Init_GPIO_ADC_VOL(void){
	
	ADC1_pins_F429ZI_config();
	ADC_Init_Single_Conversion(&adchandle , ADC1);
}


/* Private functions ---------------------------------------------------------*/
//Thread function
static void Th_VOL(void *argument){
	
	uint8_t value = 40;
	uint8_t msg = 0;
	
	while (1) {
    
	  value = ADC_getVoltage(&adchandle, 10) * VOL_RANGE / VREF; //get values from channel 10->ADC123_IN10
		
		if(value != msg){
			msg = value;
			osMessageQueuePut(mid_VOL, &msg, NULL, 0U);
		}
		osDelay(READING_VOL_TIME);
  }
}


/**
  * @brief config the use of analog inputs ADC123_IN10 and ADC123_IN13 and enable ADC1 clock
  * @param None
  * @retval None
  */
static void ADC1_pins_F429ZI_config(){
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = VOL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}



/**
  * @brief Initialize the ADC to work with single conversions. 12 bits resolution, software start, 1 conversion
  * @param ADC handle
	* @param ADC instance
  * @retval HAL_StatusTypeDef HAL_ADC_Init
  */
static int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef *ADC_Instance){
	
   /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc->Instance = ADC_Instance;
  hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc->Init.Resolution = ADC_RESOLUTION_12B;
  hadc->Init.ScanConvMode = DISABLE;
  hadc->Init.ContinuousConvMode = DISABLE;
  hadc->Init.DiscontinuousConvMode = DISABLE;
  hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc->Init.NbrOfConversion = 1;
  hadc->Init.DMAContinuousRequests = DISABLE;
	hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	
  if (HAL_ADC_Init(hadc) != HAL_OK)
  {
    return -1;
  }
	
	return 0;
}



/**
  * @brief Configure a specific channels ang gets the voltage in float type. This funtion calls to  HAL_ADC_PollForConversion that needs HAL_GetTick()
  * @param ADC_HandleTypeDef
	* @param channel number
	* @retval voltage in float (resolution 12 bits and VRFE 3.3
  */
static float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel){
	
	ADC_ChannelConfTypeDef sConfig = {0};
	HAL_StatusTypeDef status;

	uint32_t raw = 0;
	float voltage = 0;
	
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = Channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	
  if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
    return -1;
		
	HAL_ADC_Start(hadc);
		
	do
		status = HAL_ADC_PollForConversion(hadc, 0); //This funtions uses the HAL_GetTick(), then it only can be executed wehn the OS is running
	while(status != HAL_OK);
		
	raw = HAL_ADC_GetValue(hadc);
		
	voltage = raw*VREF/RESOLUTION_12B; 
		
	return voltage;
}
