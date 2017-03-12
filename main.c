/*
	
	STM32F030 WiFi Forecast station
	by mkdxdx
	MIT license

*/


#include <stm32f0xx.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_tim.h>
#include <stdlib.h>
#include <delay.h>
#include <stdint.h>
#include <string.h>

// this file contains AP connection string as follows:
// const char esp_cmd_joinap[] = "AT+CWJAP=\"APNAME\",\"APPASSWORD\"";
#include <wlanconf.h>

#include <ds18b20/soft1w.h>
#include <PCD8544/pcd8544.h>
#include <bmp180/bmp180.h>


#define LED_GPIO_CLK   RCC_AHBPeriph_GPIOA
#define LED_PORT   	   GPIOA
#define LED_PIN        GPIO_Pin_4

#define ESP_PORT		GPIOA
#define ESP_RST			GPIO_Pin_6
#define	ESP_GPIO_CLK	RCC_AHBPeriph_GPIOA

#define rxarray_len 250

#define DM_TEMPONLY 0
#define DM_VERBOSE 1
#define DM_FORECAST 2
#define DM_STATUS 3

#define CM_IDLE 0
#define CM_CMDPROC 1
#define CM_ESPWAIT 2

#define CS_NOCONN 0
#define CS_CONNAP 1

#define DIGIT_BMP_WIDTH	16
#define DS18_CMD_CONVT			0x44	// Convert T
#define mmhgppa 0.007500617F

#define FCAST_T_LENGTH	4
#define CMD_STR_LEN = 11

#define ESP_RESULT_NONE 	0
#define ESP_RESULT_OK		1
#define ESP_RESULT_FAIL 	2
#define ESP_RESULT_ERROR	3
#define ESP_RESULT_TIMEOUT	4

#define ESP_MAX_ATTEMPTS	5

#define WLAN_CHECK_COUNT	9


//GLOBAL CONSTS INTO PROGMEM
const uint8_t icon8_wlan_conn[] = {
	0b00000000,
	0b11110000,
	0b00001000,
	0b11100100,
	0b00010010,
	0b11001010,
	0b00101010,
	0b10101010,
};

const uint8_t icon8_wlan_noconn[] = {
	0b00010001,
	0b00001010,
	0b00000100,
	0b00001010,
	0b00010001,
	0b11000000,
	0b00100000,
	0b10100000,
};

const uint8_t icon5_hi[] = {
	0b00001000,
	0b00001100,
	0b00111110,
	0b00001100,
	0b00001000,
};

const uint8_t icon5_lo[] = {
	0b00010000,
	0b00110000,
	0b01111100,
	0b00110000,
	0b00010000,
};

const uint8_t icon12_cloud[] = {
	0b00110000,
	0b01111000,
	0b01111100,
	0b01111100,
	0b01111110,
	0b01111101,
	0b01111101,
	0b01111101,
	0b01111001,
	0b00110010,
	0b00100100,
	0b00011000
};




const char s_start[] = "/// Init ///";

const char cmd_setfc[] = "FCS;";
const char cmd_setwind[] = "SWS;";
const char cmd_savefc[] = "SFC;";

const char esp_cmd_at[] = "AT";
const char esp_cmd_softreset[] = "AT+RST";
const char esp_cmd_setmode_sta[] = "AT+CWMODE=1";
const char esp_cmd_setmux_multi[] = "AT+CIPMUX=1";

const char esp_cmd_queryap[] = "AT+CWJAP?";
const char esp_cmd_listen[] = "AT+CIPSERVER=1,5577";
const char esp_cmd_queryip[] = "AT+CIPAP?";

const char esp_rep_ok[] = "OK";
const char esp_rep_fail[]="FAIL";
const char esp_rep_error[]="ERROR";
//GLOBAL VARIABLES:


// forecast table
typedef struct FCAST_C{
  uint8_t date;
  int8_t temp_day;
  int8_t temp_night;
  uint8_t humidity;
  uint8_t sky;
};

volatile struct FCAST_C fcastt[FCAST_T_LENGTH];

volatile char rxbuf[rxarray_len];
volatile uint16_t rxbuf_index;
volatile char vals[5];
volatile uint8_t wlan_check = 0;

// sensor data:
volatile int8_t         c_interior_temp = 22;
volatile int8_t         c_exterior_temp = 20;
volatile int8_t         c_interior_humidity = 73;
volatile uint16_t       c_pressure_pa = 4;
volatile uint16_t       c_pressure_torr = 755;
volatile int8_t         c_interior_temp_case = 225;
volatile char			c_wind_dir[3] = "XX\0";
volatile uint8_t		c_wind_spd = 0;

// various operation modes
volatile uint8_t DISPLAY_MODE = DM_TEMPONLY;
volatile uint8_t CMD_MODE = CM_IDLE;
volatile uint8_t CONNSTATUS = CS_NOCONN;

/*
interrupt handlers:
  USART_RXCOMPLETE
    start/reset receive timeout
    put byte into rxarray / DMA it there
    if previous byte == 'Z' and current byte == 'Z', reset rxbuffer index to 0

  TIMER_ONCOMPARE
    switch to command validation loop
    stop timer
*/

void Status_LED_ON();
void Status_LED_OFF();
void LED_Init();
void PeriphInit();
void ESPReset();
void SensorInit();
void PollSensors();
void PollCmd();
void DisplayStats();
void Graph_Fcast();
int8_t DS18_Measure();
uint8_t ESPSendCmd(const char *cmd);
void ESPConnect(void);
void ClrBuffer(void);
inline uint8_t ESPCheckConnection(void);
void Delay_s(uint8_t s);
void WLAN_Status(void);
void COMMInit(void);
void TIM_ResetStart(void);
void TIM_Stop(void);
void ITInit(void);

void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET) {

		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		rxbuf[rxbuf_index] = USART_ReceiveData(USART1);
		rxbuf_index++;

		if (rxbuf_index>=rxarray_len) rxbuf_index = 0;

		if (CMD_MODE != CM_ESPWAIT)
			TIM_ResetStart();
	}

	if (USART_GetITStatus(USART1,USART_IT_TC)!=RESET) {
		USART_ClearITPendingBit(USART1,USART_IT_TC);
	}
}

void TIM14_IRQHandler(void) {
	if (TIM_GetITStatus(TIM14,TIM_IT_Update)!= RESET) {
		TIM_ClearITPendingBit(TIM14,TIM_IT_Update);
		TIM_Stop();
		CMD_MODE = CM_CMDPROC;
	}

}


int main(void)
{
	// core init
	SystemInit();

	// some chinese delay init library
	Delay_Init(48);

	// start usart, gpio, i2c and spi
	PeriphInit();

	// set up sensors
	SensorInit();

	// set up display data
	Display_Init();
	Display_Clear();
	Display_SetXY(0,2);
	Display_PutS(s_start);

	uint16_t i;
	for (i = 0; i<FCAST_T_LENGTH; i++) {
		fcastt[i].date = i+1;
		fcastt[i].temp_day = 0;
		fcastt[i].temp_night = 0;
		fcastt[i].sky = 0;
	}

	ClrBuffer();
	ITInit();
	COMMInit();
	ESPReset();

    while(1)
    {

    	// gather sensor data
    	PollSensors();

    	//DISPLAY_MODE = DM_FORECAST;
		DISPLAY_MODE++;
		if (DISPLAY_MODE>DM_FORECAST) DISPLAY_MODE = DM_VERBOSE;

    	// see if anything in rxbuffer
    	PollCmd();

    	DisplayStats();

    	wlan_check++;
    	if (wlan_check>WLAN_CHECK_COUNT) {
    		wlan_check = 0;
    		if (CONNSTATUS == CS_NOCONN)
				ESPConnect();
    		CONNSTATUS = ESPCheckConnection();
    	}

    	// draw stuff onto screen
    	if (DISPLAY_MODE == DM_FORECAST)
    		Delay_s(7);
    	else
    		Delay_s(2);
    }
}

void SensorInit() {
	Bmp180InitI2C();
	Bmp180Calibrate();
	OW_SetBus();
}


void PollSensors() {
	c_interior_temp = Bmp180GetT()/10;
	c_pressure_torr = (uint16_t)(Bmp180GetP(BMP180_OSV8)*mmhgppa);
	c_exterior_temp = DS18_Measure();
}


void PollCmd() {
	switch (CMD_MODE) {
	case CM_IDLE: break;
	case CM_CMDPROC: {
			uint16_t i;
			for (i=rxbuf_index; i<rxarray_len; i++)
				rxbuf[i] = 0;

			char *p = strstr(rxbuf,cmd_setfc);

			if (p != NULL) {
				p++;
				p++;
				p++;
				p++;
				uint8_t index = *p++;
				if (index<FCAST_T_LENGTH) {
					fcastt[index].date = *p++;

					if (*p++ == '-')
						*p = 0-*p;
					fcastt[index].temp_day = *p++;

					if (*p++ == '-')
						*p = 0-*p;
					fcastt[index].temp_night = *p++;

					fcastt[index].humidity = *p++;
					fcastt[index].sky = *p;
				}
			}

			p = strstr(rxbuf,cmd_setwind);
			if (p != NULL) {
				p++;
				p++;
				p++;
				p++;
				c_wind_spd = *p++;
				c_wind_dir[0] = *p++;
				c_wind_dir[1] = *p;
			}
			ClrBuffer();
			CMD_MODE = CM_IDLE;
			break;
		}
	default: break;
	}
}




void ITInit() {
	TIM_ITConfig(TIM14,TIM_IT_Update,ENABLE);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_EnableIRQ(TIM14_IRQn);
	__enable_irq();

}

void TIM_ResetStart(void) {
	TIM_SetCounter(TIM14,0);
	TIM_Cmd(TIM14,ENABLE);
}

void TIM_Stop() {
	TIM_Cmd(TIM14,DISABLE);
}

void USend(uint8_t data) {
	USART_SendData(USART1,data);
	while(!USART_GetFlagStatus(USART1,USART_FLAG_TC));
}

void DisplayStats() {
	char vals[5];
	Display_Clear();

	switch (DISPLAY_MODE) {
	case DM_TEMPONLY: {
		itoa(c_exterior_temp,vals,10);

		Display_DigitVal(vals,16,1);
		//display wifi status icon

		Display_SetXY(68,2);
		WLAN_Status();
		break;
	}
	case DM_VERBOSE: {
		itoa(c_exterior_temp,vals,10);

		Display_SetXY(0,1);
		if (c_exterior_temp>0)
			Display_PutChar('+');
		else
			Display_PutChar('-');

		c_exterior_temp = abs(c_exterior_temp);
		itoa(c_exterior_temp,vals,10);
		Display_DigitVal(vals,6,0);

		Display_SetXY(40,0);
		Display_PutChar('C');

		itoa(c_pressure_torr,vals,10);
		Display_DigitVal(vals,0,3);

		// draw vertical line
		uint8_t y;
		for (y = 0; y<6; y++)
		{
			Display_SetXY(48,y);
			Display_Put(0xFF);
		}

		itoa(c_interior_temp,vals,10);
		Display_DigitVal(vals,52,0);

		Display_SetXY(52,3);
		itoa(c_wind_spd,vals,10);
		Display_PutS(vals);
		Display_PutS("m/s");
		Display_SetXY(52,4);
		Display_PutS(c_wind_dir);


		Display_SetXY(75,5);
		WLAN_Status();

		break;
	}
	case DM_FORECAST: {
		Graph_Fcast();
		break;
	}
	case DM_STATUS: {
		//Display_SetXY(0,1);
		//p = s_status;
		//Display_PutS(p);
		//display status
		  //last command received
		  //ip address
		  //wifi status

		Display_SetXY(0,0);
		WLAN_Status();
		break;
		}
	default: break;
	}

}

void Graph_Fcast() {
	Display_SetXY(0,0);
	uint8_t col_index;
	char *p;
	char conv[5];

	Display_SetXY(4,0);
	Display_PutChar('#');

	Display_SetXY(4,2);
	for (col_index = 0; col_index<5; col_index++)
		Display_Put(icon5_hi[col_index]);

	Display_SetXY(4,3);
	for (col_index = 0; col_index<5; col_index++)
		Display_Put(icon5_lo[col_index]);

	Display_SetXY(0,4);
	for (col_index = 0; col_index<12; col_index++)
		Display_Put(icon12_cloud[col_index]);

	Display_SetXY(4,5);
	Display_PutChar('R');
	Display_PutChar('H');

	for (col_index = 0; col_index<FCAST_T_LENGTH; col_index++) {
		uint8_t cx = col_index*16+20;

		// display day
		Display_SetXY(cx,0);
		itoa(fcastt[col_index].date,conv,10);
		p = conv;
		Display_PutS(p);

		// display day temp
		Display_SetXY(cx,2);
		if (fcastt[col_index].temp_day<0) {
			Display_SetXY(cx-3,2);
			Display_Put(0b00001000);
			Display_Put(0b00001000);
			Display_Put(0x00);
		}
		itoa(abs(fcastt[col_index].temp_day),conv,10);
		p = conv;
		Display_PutS(p);

		// display night temp
		Display_SetXY(cx,3);
		if (fcastt[col_index].temp_night<0) {
			Display_SetXY(cx-3,3);
			Display_Put(0b00001000);
			Display_Put(0b00001000);
			Display_Put(0x00);
		}
		itoa(abs(fcastt[col_index].temp_night),conv,10);
		p = conv;
		Display_PutS(p);

		// display cloudness
		Display_SetXY(cx,4);
		itoa(fcastt[col_index].sky,conv,10);
		p = conv;
		Display_PutS(p);

		Display_SetXY(cx,5);
		itoa(fcastt[col_index].humidity,conv,10);
		p = conv;
		Display_PutS(p);
	}

	uint8_t y,x;
	Display_SetXY(0,1);
	for (x = 0; x<84; x++)
		Display_Put(0b00011000);

	for (x = 0; x < FCAST_T_LENGTH; x++) {
		for (y=0;y<7;y++) {
			Display_SetXY((x+1)*16,y);
			Display_Put(0xFF);
		}

	}



}

void Status_LED_ON(){
	GPIO_ResetBits(LED_PORT,LED_PIN);
}

void Status_LED_OFF(){
	GPIO_SetBits(LED_PORT,LED_PIN);
}


void PeriphInit() {
	RCC_AHBPeriphClockCmd(LED_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14,ENABLE);

	TIM_TimeBaseInitTypeDef tbi;
	tbi.TIM_Prescaler = 48000-1;
	tbi.TIM_CounterMode = TIM_CounterMode_Up;
	tbi.TIM_ClockDivision = 0;
	tbi.TIM_Period = 99;
	TIM_TimeBaseInit(TIM14,&tbi);
}

void COMMInit(void) {
	GPIO_InitTypeDef gis;
	// if I2C get conflicts with USART - remap GPIOA9 to GPIOA2 for TX and GPIOA10 to GPIOA3 for RX
		GPIO_PinAFConfig(LED_PORT,GPIO_PinSource2, GPIO_AF_1);
		GPIO_PinAFConfig(LED_PORT,GPIO_PinSource3, GPIO_AF_1);

		GPIO_StructInit(&gis);
		gis.GPIO_Pin = GPIO_Pin_2;
		gis.GPIO_Speed = GPIO_Speed_50MHz;
		gis.GPIO_Mode = GPIO_Mode_AF;
		gis.GPIO_OType = GPIO_OType_PP;
		gis.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &gis);

		gis.GPIO_Pin = GPIO_Pin_3;
		gis.GPIO_Mode = GPIO_Mode_AF;
		gis.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &gis);

		GPIO_StructInit(&gis);
		gis.GPIO_Pin = ESP_RST;
		gis.GPIO_Speed = GPIO_Speed_50MHz;
		gis.GPIO_OType = GPIO_OType_PP;
		gis.GPIO_Mode = GPIO_Mode_OUT;
		gis.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(ESP_PORT,&gis);

		USART_InitTypeDef usart_struct;
		usart_struct.USART_BaudRate            = 115200;
		usart_struct.USART_WordLength          = USART_WordLength_8b;
		usart_struct.USART_StopBits            = USART_StopBits_1;
		usart_struct.USART_Parity              = USART_Parity_No ;
		usart_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		usart_struct.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
		USART_Init(USART1,&usart_struct);

		USART_Cmd(USART1,ENABLE);
}

int8_t DS18_Measure(void) {
	if (OW_SkipROM())
		{
			OW_WriteByte(DS18_CMD_CONVT);
			delay_ms(200);
			OW_SkipROM();
			OW_WriteByte(OW_CMD_RDSCR);
			int8_t temp = (OW_ReadByte() | (OW_ReadByte()<<8))>>4;
			return temp;
		} else
			return -1;
}

void ClrBuffer(void) {
	uint16_t i;
	for (i=0;i<rxarray_len;i++)
		rxbuf[i] = 0;
	rxbuf_index = 0;
}

uint8_t ESPSendCmd(const char *cmd) {
	ClrBuffer();
	CMD_MODE = CM_ESPWAIT;
	char *cp = cmd;
	while (*cp!='\0') {
		USend(*cp);
		cp++;
	}
	USend(13);
	USend(13);
	USend(10);
	delay_ms(200);
	uint8_t att = 0;
	uint8_t result = ESP_RESULT_NONE;
	while (result == ESP_RESULT_NONE) {
		if ( strstr(rxbuf,esp_rep_ok) != NULL) {
			result = ESP_RESULT_OK;
			break;
		} else if (strstr(rxbuf,esp_rep_error) != NULL) {
			result = ESP_RESULT_ERROR;
			break;
		} else if (strstr(rxbuf,esp_rep_fail) != NULL) {
			result = ESP_RESULT_FAIL;
			break;
		}
		Delay_s(1);
		att++;
		if (att>ESP_MAX_ATTEMPTS)
			result = ESP_RESULT_TIMEOUT;
	}

	CMD_MODE = CM_IDLE;

	return result;
}


void ESPReset(void) {
	GPIO_ResetBits(ESP_PORT,ESP_RST);
	Delay_s(1);
	GPIO_SetBits(ESP_PORT,ESP_RST);
	Delay_s(2);
	ESPSendCmd(esp_cmd_softreset);
	Delay_s(1);
}

inline uint8_t ESPCheckConnection(void) {
	if (ESPSendCmd(esp_cmd_queryap) == ESP_RESULT_OK)
		return CS_CONNAP;
	else
		return CS_NOCONN;
}

void ESPConnect(void) {
	if (ESPSendCmd(esp_cmd_at) == ESP_RESULT_OK)
		if (ESPSendCmd(esp_cmd_setmode_sta) == ESP_RESULT_OK)
			if (ESPSendCmd(esp_cmd_setmux_multi) == ESP_RESULT_OK)
				if (ESPSendCmd(esp_cmd_joinap) == ESP_RESULT_OK)
					if (ESPSendCmd(esp_cmd_listen) == ESP_RESULT_OK) {
						CONNSTATUS = CS_CONNAP;
						return;
					}
	CONNSTATUS = CS_NOCONN;
}

void WLAN_Status(void) {
	uint8_t ind;
	switch (CONNSTATUS) {
	case CS_NOCONN:
		for (ind = 0; ind<8; ind++)
			Display_Put(icon8_wlan_noconn[ind]);
		break;
	case CS_CONNAP:
		for (ind = 0; ind<8; ind++)
			Display_Put(icon8_wlan_conn[ind]);
	default: break;
	}
}

void Delay_s(uint8_t s) {
	while (s) {
		delay_ms(1000);
		s--;
	}
}
