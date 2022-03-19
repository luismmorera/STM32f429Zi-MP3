/* Includes ------------------------------------------------------------------*/
#include "principal.h"

/* Defines -------------------------------------------------------------------*/
#define MP3_RESET								0																		// MP3 code for RESET command
#define MP3_SELECT_DEVICE				1																		// MP3 code for SELECT DEVICE command
#define MP3_PLAY								2																		// MP3 code for PLAY command
#define MP3_PAUSE								3																		// MP3 code for PAUSE command
#define MP3_SLEEP								4																		// MP3 code for SLEEP command
#define MP3_WAKE_UP							5																		// MP3 code for WAKE UP command
#define MP3_VOLUME							6																		// MP3 code for VOLUME command
#define MP3_PLAY_WITH_FF				7																		// MP3 code for PLAY WITH FOLDER AND FILE command
#define MP3_CMD_WAITING_TIME		200U																// Time to wait to ensure MP3 functioning
#define LED_RED_FREQ						100U																// Red led frequency 
#define LED_GREEN_FREQ					125U																// Green led frequency 
#define LED_BLUE_FREQ						500U																// Blue led frequency 
#define COM_TIMER								5000U																// LCD screen to COM timeout 
#define QUEUE_TIMEOUT						200U																// Message queues timeout

/* Typedef -------------------------------------------------------------------*/
typedef enum{
	
	MAIN_REPOSO,
	MAIN_REPRODUCCION,
	MAIN_HORA
	
}MAIN_STATE;																												// States of the main automaton

typedef enum{
	
	CHANGE_HOURS,
	CHANGE_MINUTES,
	CHANGE_SECONDS
	
}CHANGE_TIME;																												// States of the changing time mode

/* Private variables ---------------------------------------------------------*/
static MAIN_STATE 		estado;
static CHANGE_TIME		h_m_s;
static osTimerId_t 		timMAIN_id;                            				// 1 second MAIN timer id
static osTimerId_t 		timLED_id;                            				// LED multiplexing timer id
static osTimerId_t 		timCOM_id;                            				// 5 second COM timer id
static uint16_t 			tiempo_reproduccion;													// Currently song time
static uint8_t				folder;																				// Variable where current folder is saved
static uint8_t				file;																					// Variable where current file is saved
static uint8_t				msg_VOL;																			// Variable where volume is saved
static uint8_t 				play_pause;																		// 0 -> currently paused; 1-> currently playing
static uint8_t				card_inserted;																// 0 -> no SD card ; 1 -> SD card inserted
static uint8_t				horas_aux;																		// Variable to show hours while changing time
static uint8_t				minutos_aux;																	// Variable to show minutes while changing time
static uint8_t				segundos_aux;																	// Variable to show seconds while changing time

/* Private function prototypes -----------------------------------------------*/
static void Th_MAIN(void *argument);																// Main thread funciton
static void MAIN_Timer_Callback(void const *arg);										// LED multiplexing timer funcion
static void LED_Timer_Callback(void const *arg);										// Main 1 second timer funcion
static void COM_Timer_Callback(void const *arg);										// COM 5 second timer funcion
static void Send_MP3_commands(uint8_t command, uint16_t param);			// Send commands to MP3
static void Send_LCD_data(char cadena[], uint8_t linea);						// Send data to LCD
static void Send_COM_data(char cadena[]);														// Send data to COM
static void JOY_pulsacion_reproduccion(uint8_t msg_JOY);						// Do what each joystick pulsation should do while reproduction
static void JOY_pulsacion_change_time(uint8_t msg_JOY);							// Do what each joystick pulsation should do while changing time
static void play_with_ff(void);																			// Play whith folder and file, print in LCD and restart reproduction time

/* Public functions ----------------------------------------------------------*/
//Module initialization function
int Init_MAIN(void){
	
	osThreadId_t tid_Th_MAIN;
	uint32_t execMAIN = 1U;
	uint32_t execLED = 2U;
	
	tid_Th_MAIN = osThreadNew(Th_MAIN, NULL, NULL);
  if (tid_Th_MAIN == NULL){
    return(-1);
  }
	
  timMAIN_id = osTimerNew((osTimerFunc_t)&MAIN_Timer_Callback, osTimerPeriodic, &execMAIN, NULL);
  if (timMAIN_id == NULL){
    return -1;
  }
	
	timLED_id = osTimerNew((osTimerFunc_t)&LED_Timer_Callback, osTimerPeriodic, &execLED, NULL);
  if (timLED_id == NULL){
    return -1;
  }
	
	timCOM_id = osTimerNew((osTimerFunc_t)&COM_Timer_Callback, osTimerPeriodic, &execLED, NULL);
  if (timCOM_id == NULL){
    return -1;
  }
	
	return 0;
}


/* Private functions ---------------------------------------------------------*/
//Threads functions
static void Th_MAIN(void *argument){
	
	uint8_t 			msg_JOY;
	uint8_t 			msg_MP3[MAX_CHARACTERS_RX_MSG_MP3];
	uint8_t 			msg_COM[MAX_CHARACTERS_RX_MSG_COM];
	osStatus_t		status;																						// Message queues status
	char 					cadena[MAX_CHARACTERS_LCD];												// String to send to the LCD
	
	estado = MAIN_REPOSO;
	tiempo_reproduccion = 0;
	folder = 1;
	file = 1;
	play_pause = 0;
	card_inserted = 1;
	
	osTimerStart(timMAIN_id, 1000U);
	osTimerStart(timCOM_id, COM_TIMER);
	
	Send_MP3_commands(MP3_RESET, 0x0000);
	osDelay(300U);
	Send_MP3_commands(MP3_SELECT_DEVICE, 0x0002);
	Send_MP3_commands(MP3_SLEEP, 0x0000);
	
	while(1){
		
		switch(estado){
			
			case MAIN_REPOSO:
								
				status = osMessageQueueGet(mid_MP3_read, &msg_MP3, NULL, QUEUE_TIMEOUT);
				if(status == osOK){
					
					if(msg_MP3[3] == 0x3A){
						card_inserted = 1;
						osTimerStop(timLED_id);
						osMessageQueuePut(mid_LED, 0x00, 0U, 0U);
					}
					
					if(msg_MP3[3] == 0x3B){
						card_inserted = 0;
						osTimerStart(timLED_id, LED_RED_FREQ);
						osThreadFlagsSet(tid_Th_PWM, F_PWM_SOUND);
					}
				}
				
				status = osMessageQueueGet(mid_COM_read, &msg_COM, NULL, QUEUE_TIMEOUT);
				if(status == osOK){
					horas = (msg_COM[5] & 0x0F) * 10 + (msg_COM[6] & 0x0F);
					minutos = (msg_COM[8] & 0x0F) * 10 + (msg_COM[9] & 0x0F);
					segundos = (msg_COM[11] & 0x0F) * 10 + (msg_COM[12] & 0x0F);
				}
				
				status = osMessageQueueGet(mid_JOY, &msg_JOY, NULL, QUEUE_TIMEOUT);				
				if(status == osOK){
					
					osThreadFlagsSet(tid_Th_PWM, F_PWM_SOUND);
					
					if(msg_JOY == 0x90 && card_inserted){
						estado = MAIN_REPRODUCCION;
						tiempo_reproduccion = 0;
						osTimerStop(timMAIN_id);
						sprintf(cadena, "F: %.2d  C: %.2d   Vol: %.2d", folder, file, msg_VOL);
						Send_LCD_data(cadena, 1);
						Send_LCD_data("   Trepr: 00 : 00", 2);
						Send_MP3_commands(MP3_WAKE_UP, 0x0000);
						osTimerStart(timLED_id, LED_BLUE_FREQ);
					}
					
					if(msg_JOY == 0x84){
						estado = MAIN_HORA;
						h_m_s = CHANGE_HOURS;
						horas_aux = horas;
						minutos_aux = minutos;
						segundos_aux = segundos;
					}
				}
				
				break;
			
				
			case MAIN_REPRODUCCION:
			
				status = osMessageQueueGet(mid_JOY, &msg_JOY, NULL, QUEUE_TIMEOUT);
				if(status == osOK){
					osThreadFlagsSet(tid_Th_PWM, F_PWM_SOUND);
					JOY_pulsacion_reproduccion(msg_JOY);
				}
				
				status = osMessageQueueGet(mid_VOL, &msg_VOL, NULL, QUEUE_TIMEOUT);
				if(status == osOK){
					sprintf(cadena, "F: %.2d  C: %.2d   Vol: %.2d", folder, file, msg_VOL);
					Send_LCD_data(cadena, 1);
					
					Send_MP3_commands(MP3_VOLUME, (uint16_t)msg_VOL);
				}
				
				status = osMessageQueueGet(mid_COM_read, &msg_COM, NULL, QUEUE_TIMEOUT);
				if(status == osOK){
					horas = (msg_COM[5] & 0x0F) * 10 + (msg_COM[6] & 0x0F);
					minutos = (msg_COM[8] & 0x0F) * 10 + (msg_COM[9] & 0x0F);
					segundos = (msg_COM[11] & 0x0F) * 10 + (msg_COM[12] & 0x0F);
				}
				
				status = osMessageQueueGet(mid_MP3_read, &msg_MP3, NULL, QUEUE_TIMEOUT);
				if(status == osOK){
					
					if(msg_MP3[3] == 0x3B){
						card_inserted = 0;
						estado = MAIN_REPOSO;
						play_pause = 0;
						Send_MP3_commands(MP3_PAUSE, 0x0000);
						Send_MP3_commands(MP3_SLEEP, 0x0000);
						osTimerStart(timMAIN_id, 1000U);
						osTimerStop(timLED_id);
						osTimerStart(timLED_id, LED_RED_FREQ);
						osThreadFlagsSet(tid_Th_PWM, F_PWM_SOUND);
					}
					
					if(msg_MP3[3] == 0x3D){
						Send_MP3_commands(MP3_PAUSE, 0x0000);
						play_pause = 0;
						tiempo_reproduccion = 0;
						osTimerStop(timMAIN_id);
						osTimerStop(timLED_id);
						osTimerStart(timLED_id, LED_BLUE_FREQ);
					}
				}
				
				break;
			
				
			case MAIN_HORA:
				
				if(h_m_s == CHANGE_HOURS){
					sprintf(cadena, "    /%.2d/ : %.2d : %.2d", horas_aux, minutos_aux, segundos_aux);
					Send_LCD_data(cadena, 2);
				}
				
				if(h_m_s == CHANGE_MINUTES){
					sprintf(cadena, "    %.2d : /%.2d/ : %.2d", horas_aux, minutos_aux, segundos_aux);
					Send_LCD_data(cadena, 2);
				}
				
				if(h_m_s == CHANGE_SECONDS){
					sprintf(cadena, "    %.2d : %.2d : /%.2d/", horas_aux, minutos_aux, segundos_aux);
					Send_LCD_data(cadena, 2);
				}
				
				status = osMessageQueueGet(mid_MP3_read, &msg_MP3, NULL, QUEUE_TIMEOUT);
				if(status == osOK){
					
					if(msg_MP3[3] == 0x3A){
						card_inserted = 1;
						osTimerStop(timLED_id);
						osMessageQueuePut(mid_LED, 0x00, 0U, 0U);
					}
					
					if(msg_MP3[3] == 0x3B){
						card_inserted = 0;
						osTimerStart(timLED_id, LED_RED_FREQ);
						osThreadFlagsSet(tid_Th_PWM, F_PWM_SOUND);
					}
				}
				
				status = osMessageQueueGet(mid_COM_read, &msg_COM, NULL, QUEUE_TIMEOUT);
				if(status == osOK){
					horas = (msg_COM[5] & 0x0F) * 10 + (msg_COM[6] & 0x0F);
					minutos = (msg_COM[8] & 0x0F) * 10 + (msg_COM[9] & 0x0F);
					segundos = (msg_COM[11] & 0x0F) * 10 + (msg_COM[12] & 0x0F);
				}
				
				status = osMessageQueueGet(mid_JOY, &msg_JOY, NULL, QUEUE_TIMEOUT);
				if(status == osOK){
					osThreadFlagsSet(tid_Th_PWM, F_PWM_SOUND);
					JOY_pulsacion_change_time(msg_JOY);
				}
				
				break;
		}
	}	
}

//1 second MAIN timer function
static void MAIN_Timer_Callback(void const *arg){
	
	char cadena[MAX_CHARACTERS_LCD] = {0};												// String to send to the LCD
	
	if(estado == MAIN_REPOSO){
		
		sprintf(cadena, " SBM 2021  T:%2.1f C", temperatura);
		Send_LCD_data(cadena, 1);
		
		sprintf(cadena, "    %.2d : %.2d : %.2d", horas, minutos, segundos);
		Send_LCD_data(cadena, 2);
	}
	
	if(estado == MAIN_REPRODUCCION){
		
		tiempo_reproduccion++;
		
		sprintf(cadena, "   Trepr: %.2d : %.2d", tiempo_reproduccion/60, tiempo_reproduccion%60);
		Send_LCD_data(cadena, 2);
	}
	
	if(estado == MAIN_HORA){
		
		sprintf(cadena, "Prog HORA  T:%2.1f C", temperatura);
		Send_LCD_data(cadena, 1);
	}
}

//LED multiplexing timer funcion
static void LED_Timer_Callback(void const *arg){
	
	static uint8_t multi = 0;
	uint8_t msg_LED = 0;
	
	if(!card_inserted){
		msg_LED = (multi % 2);
		osMessageQueuePut(mid_LED, &msg_LED, 0U, 0U);
	}
	
	if(play_pause){
		msg_LED = (multi % 2) * 2;
		osMessageQueuePut(mid_LED, &msg_LED, 0U, 0U);
	}
	
	if(!play_pause && card_inserted){
		msg_LED = (multi % 2) * 4;
		osMessageQueuePut(mid_LED, &msg_LED, 0U, 0U);
	}
	
	multi++;
}

//5 second COM timer function
static void COM_Timer_Callback(void const *arg){
	
	char cadena[MAX_CHARACTERS_TX_MSG_COM] = {0};												// String to send to the LCD
	
	if(estado == MAIN_REPOSO){
		
		sprintf(cadena, "SBM 2021  T:%2.1f C", temperatura);
		Send_COM_data(cadena);
		
		sprintf(cadena, "   %.2d : %.2d : %.2d", horas, minutos, segundos);
		Send_COM_data(cadena);
	}
	
	if(estado == MAIN_REPRODUCCION){
		
		sprintf(cadena, "F: %.2d  C: %.2d   Vol: %.2d", folder, file, msg_VOL);
		Send_COM_data(cadena);
		
		sprintf(cadena, "   Trepr: %.2d : %.2d", tiempo_reproduccion/60, tiempo_reproduccion%60);
		Send_COM_data(cadena);
	}
	
	if(estado == MAIN_HORA){
		
		sprintf(cadena, "Prog HORA  T:%2.1f C", temperatura);
		Send_COM_data(cadena);
		
		sprintf(cadena, "    %.2d : %.2d : %.2d", horas_aux, minutos_aux, segundos_aux);
		Send_COM_data(cadena);
		
		if(h_m_s == CHANGE_HOURS){
			sprintf(cadena, "    ^^");
			Send_COM_data(cadena);
		}
		
		if(h_m_s == CHANGE_MINUTES){
			sprintf(cadena, "         ^^");
			Send_COM_data(cadena);
		}
		
		if(h_m_s == CHANGE_SECONDS){
			sprintf(cadena, "              ^^");
			Send_COM_data(cadena);
		}
	}
	
	Send_COM_data("\n");
}

//Send commands to MP3
static void Send_MP3_commands(uint8_t command, uint16_t param){
	
	const uint8_t MP3_commands[] = {
		0x7E, 0xFF, 0x06, 0x0C, 0x00, 0x00, 0x00, 0xEF, // RESET
		0x7E, 0xFF, 0x06, 0x09, 0x00, 0x00, 0x00, 0xEF, // SELECT DEVICE
		0x7E, 0xFF, 0x06, 0x0D, 0x00, 0x00, 0x00, 0xEF, // PLAY
		0x7E, 0xFF, 0x06, 0x0E, 0x00, 0x00, 0x00, 0xEF, // PAUSE
		0x7E, 0xFF, 0x06, 0x0A, 0x00, 0x00, 0x00, 0xEF, // SLEEP
		0x7E, 0xFF, 0x06, 0x0B, 0x00, 0x00, 0x00, 0xEF, // WAKE UP
		0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, 0x00, 0xEF,	// SET VOLUME
		0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x00, 0x00, 0xEF	// PLAY WITH FOLDER AND FILE
	};
	
	uint8_t msg_MP3[MAX_CHARACTERS_TX_MSG_MP3];				// Message to send to the MP3
	char cadena[MAX_CHARACTERS_TX_MSG_COM];							// Message to send to the Teraterm console
	uint8_t i;
	
	for(i = 0; i < MAX_CHARACTERS_TX_MSG_MP3; i++)
		msg_MP3[i] = MP3_commands[command * 8 + i];
	
	msg_MP3[5] = param / 256;
	msg_MP3[6] = param;
	
	osMessageQueuePut(mid_MP3_write, &msg_MP3, 0U, 0U);
	
	sprintf(cadena, "%.2d:%.2d:%.2d ---> %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",	horas, minutos, segundos,
																																										msg_MP3[0],
																																										msg_MP3[1],
																																										msg_MP3[2],
																																										msg_MP3[3],
																																										msg_MP3[4],
																																										msg_MP3[5],
																																										msg_MP3[6],
																																										msg_MP3[7]);
	Send_COM_data(cadena);
	osDelay(MP3_CMD_WAITING_TIME);
}

//Send data to LCD
static void Send_LCD_data(char cadena[], uint8_t linea){
	
	MSGQUEUE_LCD_OBJ_t 	msg_LCD;																	// Message to send to the LCD
	
	sprintf(msg_LCD.cadena, cadena);
	msg_LCD.linea = linea;
	osMessageQueuePut(mid_LCD, &msg_LCD, 0U, 0U);
}

//Send data to COM
static void Send_COM_data(char cadena[]){
	
	char msg_COM[MAX_CHARACTERS_TX_MSG_COM] = {0};										// Message to send to the Teraterm
	
	sprintf(msg_COM, cadena);
	osMessageQueuePut(mid_COM_write, &msg_COM, 0U, 0U);
	
	memset(&msg_COM, 0x00, MAX_CHARACTERS_TX_MSG_COM);
	sprintf(msg_COM, "\n");
	osMessageQueuePut(mid_COM_write, &msg_COM, 0U, 0U);
}

//Do what each joystick pulsation should do while reproduction
static void JOY_pulsacion_reproduccion(uint8_t msg_JOY){
	
	//Right short pulsation
	if(msg_JOY & 0x02){
		if(folder == 1)
			file = (file > 1) ? 1 : file + 1;
		
		if(folder == 2)
			file = (file > 2) ? 1 : file + 1;
		
		if(folder == 3)
			file = 1;
		
		play_with_ff();
	}
	
	//Left short pulsation
	if(msg_JOY & 0x08 && !(msg_JOY & 0x80)){
		if(folder == 1)
			file = (file < 2) ? 2 : file - 1;
		
		if(folder == 2)
			file = (file < 2) ? 3 : file - 1;
		
		if(folder == 3)
			file = 1;
		
		play_with_ff();
	}
	
	//Left long pulsation
	if(msg_JOY & 0x08 && msg_JOY & 0x80){
		
		Send_MP3_commands(MP3_PLAY_WITH_FF, (uint16_t)folder * 256 | file);
		play_pause = 1;
		
		tiempo_reproduccion = 0;
		osTimerStart(timMAIN_id, 1000U);
		osTimerStop(timLED_id);
		osTimerStart(timLED_id, LED_GREEN_FREQ);
		Send_LCD_data("   Trepr: 00 : 00", 2);
	}
	
	//Down short pulsation
	if(msg_JOY & 0x04){
		file = 1;
		folder = (folder > 2) ? 1 : folder + 1;
		
		play_with_ff();
	}
	
	//Up short pulsation
	if(msg_JOY & 0x01){
		file = 1;
		folder = (folder < 2) ? 3 : folder - 1;
		
		play_with_ff();
	}
	
	//Middle short pulsation
	if(msg_JOY & 0x10 && !(msg_JOY & 0x80)){
		if(!play_pause){
			Send_MP3_commands(MP3_PLAY, 0x0000);
			play_pause = 1;
			osTimerStart(timMAIN_id, 1000U);
			osTimerStop(timLED_id);
			osTimerStart(timLED_id, LED_GREEN_FREQ);
		}
		
		else if(play_pause){
			Send_MP3_commands(MP3_PAUSE, 0x0000);
			play_pause = 0;
			osTimerStop(timMAIN_id);
			osTimerStop(timLED_id);
			osTimerStart(timLED_id, LED_BLUE_FREQ);
		}
	}
	
	//Middle long pulsation
	if(msg_JOY & 0x10 && msg_JOY & 0x80){
		estado = MAIN_REPOSO;
		play_pause = 0;
		Send_MP3_commands(MP3_PAUSE, 0x0000);
		Send_MP3_commands(MP3_SLEEP, 0x0000);
		osTimerStart(timMAIN_id, 1000U);
		osTimerStop(timLED_id);
		osMessageQueuePut(mid_LED, 0x00, 0U, 0U);
	}
}

//Do what each joystick pulsation should do while changing time
static void JOY_pulsacion_change_time(uint8_t msg_JOY){
	
	//Right short pulsation
	if(msg_JOY & 0x02){
		if(h_m_s == CHANGE_HOURS){
			h_m_s = CHANGE_MINUTES;
		}
		
		else if(h_m_s == CHANGE_MINUTES){
			h_m_s = CHANGE_SECONDS;
		}
	}
	
	//Left short pulsation
	if(msg_JOY & 0x08 && !(msg_JOY & 0x80)){
		if(h_m_s == CHANGE_MINUTES){
			h_m_s = CHANGE_HOURS;
		}
		
		else if(h_m_s == CHANGE_SECONDS){
			h_m_s = CHANGE_MINUTES;
		}
	}
	
	//Down short pulsation
	if(msg_JOY & 0x04){
		if(h_m_s == CHANGE_HOURS)
			horas_aux = (horas_aux < 1) ? 23 : horas_aux - 1;
		
		if(h_m_s == CHANGE_MINUTES)
			minutos_aux = (minutos_aux < 1) ? 59 : minutos_aux - 1;
		
		if(h_m_s == CHANGE_SECONDS)
			segundos_aux = (segundos_aux < 1) ? 59 : segundos_aux - 1;
	}
	
	//Up short pulsation
	if(msg_JOY & 0x01){
		if(h_m_s == CHANGE_HOURS)
			horas_aux = (horas_aux > 22) ? 0 : horas_aux + 1;
		
		if(h_m_s == CHANGE_MINUTES)
			minutos_aux = (minutos_aux > 58) ? 0 : minutos_aux + 1;
		
		if(h_m_s == CHANGE_SECONDS)
			segundos_aux = (segundos_aux > 58) ? 0 : segundos_aux + 1;
	}
	
	//Middle short pulsation
	if(msg_JOY & 0x10 && !(msg_JOY & 0x80)){
		horas = horas_aux;
		minutos = minutos_aux;
		segundos = segundos_aux;
	}
	
	//Middle long pulsation
	if(msg_JOY & 0x10 && msg_JOY & 0x80){
		estado = MAIN_REPOSO;
	}
}

//Play whith folder and file, print in LCD and restart reproduction time
static void play_with_ff(void){
	
	char cadena[MAX_CHARACTERS_LCD];												// String to send to the LCD
	
	sprintf(cadena, "F: %.2d  C: %.2d   Vol: %.2d", folder, file, msg_VOL);
	Send_LCD_data(cadena, 1);
	
	Send_MP3_commands(MP3_PLAY_WITH_FF, (uint16_t)folder * 256 | file);
	play_pause = 1;
	
	tiempo_reproduccion = 0;
	osTimerStart(timMAIN_id, 1000U);
	osTimerStop(timLED_id);
	osTimerStart(timLED_id, LED_GREEN_FREQ);
}
