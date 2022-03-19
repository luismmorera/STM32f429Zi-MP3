 /* Includes ------------------------------------------------------------------*/
#include "lcd.h"
#include "Driver_SPI.h"
#include "Arial12x12.h"

extern ARM_DRIVER_SPI Driver_SPI1;

/* Public variables ----------------------------------------------------------*/
osMessageQueueId_t mid_LCD;                																								// LCD message queue id

/* Private variables ---------------------------------------------------------*/
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
static TIM_HandleTypeDef tim7;
static uint8_t buffer_LCD[512];
static uint16_t positionL1;
static uint16_t positionL2;

/* Private function prototypes -----------------------------------------------*/
static void GPIO_init(void);																															// Init GPIO needed for LCD
static void LCD_reset(void);																															//	
static void LCD_init(void);																																//
static void delay(uint32_t n_microsegundos);																							//
static void LCD_update(void);																															//
static void LCD_wr_data(unsigned char data);																							//
static void LCD_wr_cmd(unsigned char cmd);																								//
static void LCD_symbolToLocalBuffer_L1(uint8_t symbol);																		//
static void LCD_symbolToLocalBuffer_L2(uint8_t symbol);																		//
static void LCD_symbolToLocalBuffer_L1_underlined(uint8_t symbol);												//
static void LCD_symbolToLocalBuffer_L2_underlined(uint8_t symbol);												//
static void LCD_symbolToLocalBuffer(uint8_t line, uint8_t symbol, uint8_t underlined);		//
static void LCD_clean_line(uint8_t line);																									// Cleaning line function	1 -> line 1 
																																													//												2 -> line 2
																																													//												3 -> both lines
static void Th_LCD(void *argument);																												// LCD thread function

/* Public functions ----------------------------------------------------------*/
//Module initialization function
int Init_LCD(void){
	
	osThreadId_t tid_Th_LCD;
	
  tid_Th_LCD = osThreadNew(Th_LCD, NULL, NULL);
  if (tid_Th_LCD == NULL){
    return(-1);
  }
	
	mid_LCD = osMessageQueueNew(MSGQUEUE_LCD_OBJECTS, sizeof(MSGQUEUE_LCD_OBJ_t), NULL);
  if (mid_LCD == NULL){
    return(-1);
  }
	
	return 0;
}

//GPIO and SPI initialization function
void Init_GPIO_SPI_LCD(){
	
	LCD_clean_line(3);
	GPIO_init();
	LCD_reset();
	LCD_init();
	LCD_update();
}

/* Private functions ---------------------------------------------------------*/
//Thread function
static void Th_LCD(void *argument){
	
	int i;
	MSGQUEUE_LCD_OBJ_t msg;
	char slash = '/';
	
	while(1){
		osMessageQueueGet(mid_LCD, &msg, NULL, osWaitForever);
		
		if(msg.linea == 1){
			
			LCD_clean_line(1);
			positionL1 = 0;			
		}
		
		if(msg.linea == 2){
			
			LCD_clean_line(2);
			positionL2 = 0;			
		}
		
		for(i = 0; i < strlen(msg.cadena); i++){
			
			if(msg.cadena[i] == slash){
				do{
					i++;
					if(msg.cadena[i] != slash)
						LCD_symbolToLocalBuffer(msg.linea, msg.cadena[i], 1);
					
				}while(msg.cadena[i] != slash && i < strlen(msg.cadena));
			}
			
			else{
				LCD_symbolToLocalBuffer(msg.linea, msg.cadena[i], 0);
			}
		}
		
		LCD_update();
	}
}


//Initialization functions
static void GPIO_init(){
	
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  GPIO_InitStruct.Pin = LCD_RST;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = LCD_CS;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = LCD_A0;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

static void LCD_reset(){
	
  __SPI1_CLK_ENABLE();
  SPIdrv->Initialize(NULL);
  SPIdrv->PowerControl(ARM_POWER_FULL);
  SPIdrv->Control(ARM_SPI_MODE_MASTER | 
                  ARM_SPI_CPOL1_CPHA1 | 
                  ARM_SPI_MSB_LSB     | 
                  ARM_SPI_DATA_BITS(8), 20000000);
  
  //Activar timer 7
  __HAL_RCC_TIM7_CLK_ENABLE();
  tim7.Instance = TIM7;
  tim7.Init.Prescaler = 41; //Frec resultante: 2MHz
  
  GPIO_init();
  
  HAL_GPIO_WritePin(GPIOA, LCD_RST, GPIO_PIN_RESET);
  delay(1);
  HAL_GPIO_WritePin(GPIOA, LCD_RST, GPIO_PIN_SET);
  delay(1000);
}

static void LCD_init(){
	
  LCD_wr_cmd(0xAE);  // Display off
  LCD_wr_cmd(0xA2);  // Fija el valor de la relación de la tensión de polarización del LCD a 1/9 
  LCD_wr_cmd(0xA0);  // El direccionamiento de la RAM de datos del display es la normal
  LCD_wr_cmd(0xC8);  // El scan en las salidas COM es el normal
  LCD_wr_cmd(0x22);  // Fija la relación de resistencias interna a 2
  LCD_wr_cmd(0x2F);  // Power on
  LCD_wr_cmd(0x40);  // Display empieza en la línea 0
  LCD_wr_cmd(0xAF);  // Display ON
  LCD_wr_cmd(0x81);  // Contraste
  LCD_wr_cmd(0x17);  // Valor Contraste
  LCD_wr_cmd(0xA4);  // Display all points normal
  LCD_wr_cmd(0xA6);  // LCD Display normal 
}
//End of initialization functions


//Delay function
static void delay(uint32_t n_microsegundos){
	
  tim7.Init.Period = 2*n_microsegundos - 1;
  
  HAL_TIM_Base_Init(&tim7); // configure timer
  HAL_TIM_Base_Start(&tim7); // start timer
    
  while(!__HAL_TIM_GET_FLAG(&tim7, TIM_FLAG_UPDATE)){}
  
  __HAL_TIM_CLEAR_FLAG(&tim7, TIM_FLAG_UPDATE);
  HAL_TIM_Base_Stop(&tim7);
  __HAL_TIM_SET_COUNTER(&tim7,0);
}


//Writing LCD functions
static void LCD_update(){
	
 int i;
 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB0); // Página 0

 for(i = 0; i < positionL1; i++){
  LCD_wr_data(buffer_LCD[i]);
 }

 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB1); // Página 1

 for(i = 128; i < (positionL1 + 128); i++){
  LCD_wr_data(buffer_LCD[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB2); //Página 2
 
 for(i = 256; i < (positionL2 + 256); i++){
  LCD_wr_data(buffer_LCD[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB3); // Pagina 3


 for(i = 384; i < (positionL2 + 384); i++){
  LCD_wr_data(buffer_LCD[i]);
 }
}

static void LCD_wr_data(unsigned char data){
	
  ARM_SPI_STATUS status;
  
  HAL_GPIO_WritePin(GPIOD, LCD_CS, GPIO_PIN_RESET);// Seleccionar CS = 0;
  HAL_GPIO_WritePin(GPIOF, LCD_A0, GPIO_PIN_SET);// Seleccionar A0 = 1;

  // Escribir un dato (data) usando la función SPIDrv->Send(…);
  SPIdrv->Send(&data, sizeof(data));
  // Esperar a que se libere el bus SPI;
  do{
    status = SPIdrv->GetStatus();
		
  }while (status.busy);
	
  HAL_GPIO_WritePin(GPIOD, LCD_CS, GPIO_PIN_SET);// Seleccionar CS = 1;
}

static void LCD_wr_cmd(unsigned char cmd){
	
  ARM_SPI_STATUS status;
 
  HAL_GPIO_WritePin(GPIOD,LCD_CS,GPIO_PIN_RESET);// Seleccionar CS = 0;
  HAL_GPIO_WritePin(GPIOF,LCD_A0,GPIO_PIN_RESET);// Seleccionar A0 = 0;

  // Escribir un comando (cmd) usando la función SPIDrv->Send(…);  
  SPIdrv->Send(&cmd, sizeof(cmd));
  // Esperar a que se libere el bus SPI;
  do{
    status = SPIdrv->GetStatus();
  }
  while (status.busy);

  HAL_GPIO_WritePin(GPIOD,LCD_CS,GPIO_PIN_SET);// Seleccionar CS = 1;
}

static void LCD_symbolToLocalBuffer_L1(uint8_t symbol){
	
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25 * (symbol - ' ');
	
	for(i = 0; i < Arial12x12[offset]; i++){
		
		value1 = Arial12x12[offset + i*2 + 1];
		value2 = Arial12x12[offset + i*2 + 2];

		buffer_LCD[i + positionL1] = value1;
		buffer_LCD[i + 128 + positionL1] = value2;
	}
	positionL1 = positionL1 + Arial12x12[offset];
}

static void LCD_symbolToLocalBuffer_L2(uint8_t symbol){
	
  uint8_t i, value1, value2;
  uint16_t offset = 0;
  
  offset = 25 * (symbol - ' ');
  
  for(i = 0; i < Arial12x12[offset]; i++){
		
    value1 = Arial12x12[offset + i*2 + 1];
    value2 = Arial12x12[offset + i*2 + 2];
    
    buffer_LCD[i + 256 + positionL2] = value1;
    buffer_LCD[i + 384 + positionL2] = value2;
  }
  positionL2 = positionL2 + Arial12x12[offset];
}

static void LCD_symbolToLocalBuffer_L1_underlined(uint8_t symbol){
	
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25 * (symbol - ' ');
	
	for(i = 0; i < Arial12x12[offset]; i++){
		
		value1 = Arial12x12[offset + i*2 + 1];
		value2 = Arial12x12[offset + i*2 + 2];

		buffer_LCD[i + positionL1] = value1;
		buffer_LCD[i + 128 + positionL1] = value2 | 0x10;
	}
	positionL1 = positionL1 + Arial12x12[offset];
}

static void LCD_symbolToLocalBuffer_L2_underlined(uint8_t symbol){
	
  uint8_t i, value1, value2;
  uint16_t offset = 0;
  
  offset = 25 * (symbol - ' ');
  
  for(i = 0; i < Arial12x12[offset]; i++){
		
    value1 = Arial12x12[offset + i*2 + 1];
    value2 = Arial12x12[offset + i*2 + 2];
    
    buffer_LCD[i + 256 + positionL2] = value1;
    buffer_LCD[i + 384 + positionL2] = value2 | 0x10;
  }
  positionL2 = positionL2 + Arial12x12[offset];
}

static void LCD_symbolToLocalBuffer(uint8_t line, uint8_t symbol, uint8_t underlined){	
	
	if(underlined){
		
		if(line == 1)
			LCD_symbolToLocalBuffer_L1_underlined(symbol);
	
		if(line == 2)
			LCD_symbolToLocalBuffer_L2_underlined(symbol);
	}
	
	if(!underlined){
		if(line == 1)
			LCD_symbolToLocalBuffer_L1(symbol);
		
		if(line == 2)
			LCD_symbolToLocalBuffer_L2(symbol);
	}
}

static void LCD_clean_line(uint8_t line){
	
	int i;
	
	if(line == 1){
		
		positionL1 = 128;
		positionL2 = 0;
		
		for(i = 0; i < 256; i++)
			buffer_LCD[i] = 0;
	}
	
	if(line == 2){
		
		positionL1 = 0;
		positionL2 = 128;
		
		for(i = 256; i < 512; i++)
			buffer_LCD[i] = 0;
	}
	
	if(line == 3){
		
		positionL1 = 128;
		positionL2 = 128;
		
		for(i = 0; i < 512; i++)
			buffer_LCD[i] = 0;
	}
	
	LCD_update();
}
//End od writing LCD functions
