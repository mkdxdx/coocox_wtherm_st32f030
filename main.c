#define USE_API_OPENWEATHERMAP 1
#include "main.h"

//GLOBAL VARIABLES:


// since icon can contain empty data, i'm not going to define it as array of char and depend on \0 character
// fist byte represents icon width
const uint8_t icon8_wlan_conn[] = {8, 0b00000000, 0b11110000, 0b00001000, 0b11100100, 0b00010010, 0b11001010, 0b00101010, 0b10101010};
const uint8_t icon8_wlan_noconn[] = {8, 0b00010001, 0b00001010, 0b00000100, 0b00001010, 0b00010001, 0b11000000, 0b00100000, 0b10100000};
const uint8_t icon8_wlan_noesp[] = {3, 0b11000000, 0b00100000, 0b10100000};
const uint8_t icon5_hi[] = {5, 0b00001000,	0b00001100,	0b00111110,	0b00001100,	0b00001000};
const uint8_t icon5_lo[] = {5, 0b00010000,	0b00110000,	0b01111100,	0b00110000,	0b00010000};
const uint8_t icon12_cloud[] = {12, 0b00110000,0b01111000,	0b01111100,	0b01111100,	0b01111110,	0b01111101,	0b01111101,	0b01111101,	0b01111001,	0b00110010,	0b00100100,	0b00011000};



const char s_noap[] = "No AP";

const char esp_cmd_at[] = "AT";
const char esp_cmd_softreset[] = "AT+RST";
const char esp_cmd_setmode_sta[] = "AT+CWMODE=1";
const char esp_cmd_setmux_multi[] = "AT+CIPMUX=1";
const char esp_cmd_settxmode_unvarn[] = "AT+CIPMODE=1";
const char esp_cmd_cipclose[] = "AT+CIPCLOSE";
#ifdef USE_API_OPENWEATHERMAP
const char esp_cmd_fcastserver[] = "AT+CIPSTART=\"TCP\",\"api.openweathermap.org\",80";
const char owm_getreq_get[] = "GET http://api.openweathermap.org/data/2.5/forecast?";
const char owm_getreq_body[] = " HTTP/1.1\r\nHost: api.openweathermap.org\r\nUser-Agent: curl/7.52.1\r\nAccept: */*\r\nConnection: Close\r\n\r\n";
#endif

const char esp_cmd_send[] = "AT+CIPSEND";
const char esp_cmd_queryap[] = "AT+CWJAP?";
const char esp_cmd_listen[] = "AT+CIPSERVER=1,5577";
const char esp_cmd_queryip[] = "AT+CIPAP?";
const char esp_cmd_joinap[] = "AT+CWJAP=";

const char esp_rep_ok[] = "OK";
const char esp_rep_fail[]="FAIL";
const char esp_rep_error[]="ERROR";

const char esp_str_cmdret[] = "+++";

#ifndef USE_API_NONE
volatile struct FCAST_C fcastt[FCAST_T_LENGTH];
volatile struct PREPARSE_STRUCT_T preparse[PREPARSE_COUNT];
volatile uint8_t preparse_index = 0;
volatile enum E_FCSTATUS FCAST_STATUS = FCS_UNCHANGED;

// wind lookup table
const struct WINDDIR_T wdir[8] = {
		{22,"NE\0"},
		{68," E\0"},
		{113,"SE\0"},
		{158," S\0"},
		{203,"SW\0"},
		{248," W\0"},
		{293,"NW\0"},
		{338," N\0"},
};
#endif

volatile char rxbuf[RXBUF_LEN];
volatile char rxlchar;
volatile uint16_t rxbuf_index = RXBUF_START;
volatile char vals[5];
volatile uint8_t wlan_check = 0;

// sensor data:
volatile int8_t         c_interior_temp = 22;
volatile int8_t         c_exterior_temp = 20;
volatile int8_t         c_interior_humidity = 73;
volatile uint16_t       c_pressure_pa = 4;
volatile uint16_t       c_pressure_torr = 755;
volatile int8_t         c_interior_temp_case = 225;
volatile int8_t			c_wind_dir = -1;
volatile uint8_t		c_wind_spd = 0;
volatile uint16_t charcount = 0;
volatile int16_t jsonblocknum = 0;

// various operation modes
volatile enum E_DISPLAYMODE DISPLAY_MODE = DM_VERBOSE;
volatile enum E_RXMODE rxmode = RM_NORMAL;
volatile enum E_APSTATUS apstatus = AS_NOESP;

extern const JSON_STRUCT node_3h_3;




void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET) {

		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		rxlchar = USART_ReceiveData(USART1);
		rxbuf[rxbuf_index] = rxlchar;
		rxbuf_index++;

		if (rxbuf_index>=RXBUF_LEN-1) {
			rxbuf_index = RXBUF_START;
		} else {
			rxbuf[rxbuf_index] = '\0';
		}
		//TIM_ResetStart();
	}

	if (USART_GetITStatus(USART1,USART_IT_TC)!=RESET) {
		USART_ClearITPendingBit(USART1,USART_IT_TC);
	}
}

void TIM14_IRQHandler(void) {
	if (TIM_GetITStatus(TIM14,TIM_IT_Update)!= RESET) {
		TIM_ClearITPendingBit(TIM14,TIM_IT_Update);
		TIM_Stop();
		rxmode = RM_TIMEOUT;
	}
}


int main(void)
{


	// some chinese delay init library
	Delay_Init(48);

	// start usart, gpio, i2c and spi
	PeriphInit();
	// set up sensors
	SensorInit();
	// set up display data
	Display_Init();
	Display_SendCmd(CMD_CONF_ALLON);










#ifndef USE_API_NONE
	Forecast_Load();
#endif
	ClrBuffer();
	ITInit();
	COMMInit(115200);
	ESPReset();
	apstatus = CheckAP();

	Display_SendCmd(CMD_CONF_NORMAL);
    while(1)
    {

    	// gather sensor data
    	PollSensors();

    	DisplayStats();

#ifndef	USE_API_NONE
		if ((apstatus == AS_CONNAP) && (DISPLAY_MODE == DM_VERBOSE) && (wlan_check <= 0)) {
			GetForecast();
			ProcessForecast();
			FCAST_STATUS = FCS_CHANGED;
		}

    	if (FCAST_STATUS == FCS_CHANGED) {
			FCAST_STATUS = FCS_UNCHANGED;
			Forecast_Save();
		}

		// draw stuff onto screen
		if (DISPLAY_MODE == DM_FORECAST)
			Delay_s(10);
		else
			Delay_s(3);

		//DISPLAY_MODE = DM_FORECAST;
		DISPLAY_MODE++;
		if (DISPLAY_MODE>DM_FORECAST) DISPLAY_MODE = DM_VERBOSE;

#endif

    }


}

void SensorInit() {
	I2CInit();
	Bmp180Calibrate();
	OW_SetBus();
}


void PollSensors() {
	c_interior_temp = Bmp180GetT()/10;
	c_pressure_torr = (uint16_t)(Bmp180GetP(BMP180_OSV8)*mmhgppa);
	c_exterior_temp = DS18_Measure();
}


void ITInit(void) {
	NVIC_InitTypeDef nitd;
	nitd.NVIC_IRQChannel = (USART1_IRQn);
	nitd.NVIC_IRQChannelCmd = ENABLE;
	nitd.NVIC_IRQChannelPriority = 0;
	NVIC_Init(&nitd);

	nitd.NVIC_IRQChannel = (TIM14_IRQn);
	nitd.NVIC_IRQChannelCmd = ENABLE;
	nitd.NVIC_IRQChannelPriority = 0;
	NVIC_Init(&nitd);


	TIM_ITConfig(TIM14,TIM_IT_Update,ENABLE);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);

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

void USendStr(char *str) {
	while (*str!='\0') {
		USend(*str++);
	}
}

void DisplayStats() {
	char vals[5];
	Display_Clear();
	switch (DISPLAY_MODE) {
	case DM_VERBOSE:
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
		uint8_t y = 6;
		while (y--) {
			Display_SetXY(48,y);
			Display_Put(0xFF);
		}

		itoa(c_interior_temp,vals,10);
		Display_DigitVal(vals,52,0);

		if (c_wind_dir>=0) {
			Display_SetXY(52,3);
			itoa(c_wind_spd,vals,10);
			Display_PutS(vals);
			Display_PutS("m/s");
			Display_SetXY(52,4);
			Display_PutS(wdir[c_wind_dir].dir);
		}


		wlan_check++;
		if (wlan_check>WLAN_CHECK_COUNT) {
			wlan_check = 0;
			apstatus = CheckAP();
			if (apstatus == AS_NOCONN) {
				apstatus = ConnectAP();
			}

		}
		WLAN_Status(apstatus,75,5);
		break;

	case DM_FORECAST:
		Graph_Fcast();
		break;

	default: break;
	}

}

void Forecast_Load(void) {
	uint8_t rdaddr = 0;
	uint8_t fci = 0;
	while (rdaddr<FCAST_T_LENGTH*5) {
		fcastt[fci].date = EE_I2C_ReadAt(rdaddr);
		rdaddr++;
		fcastt[fci].temp_day = (int8_t)EE_I2C_ReadAt(rdaddr);
		rdaddr++;
		fcastt[fci].temp_night = (int8_t)EE_I2C_ReadAt(rdaddr);
		rdaddr++;
		fcastt[fci].rain = EE_I2C_ReadAt(rdaddr);
		rdaddr++;
		fcastt[fci].wind  = EE_I2C_ReadAt(rdaddr);
		rdaddr++;
		fci++;
	}
}

void Forecast_Save(void) {
	// start writing procedure and hope for the best
	uint8_t index;
	uint8_t wraddr = 0;
	for (index = 0; index<FCAST_T_LENGTH; index++) {
		EE_I2C_WriteAt(wraddr,fcastt[index].date);
		wraddr++;
		EE_I2C_WriteAt(wraddr,(uint8_t)fcastt[index].temp_day);
		wraddr++;
		EE_I2C_WriteAt(wraddr,(uint8_t)fcastt[index].temp_night);
		wraddr++;
		EE_I2C_WriteAt(wraddr,fcastt[index].rain);
		wraddr++;
		EE_I2C_WriteAt(wraddr,fcastt[index].wind);
		wraddr++;
	}
}

void GetForecast(void) {
	// if we are connected to AP
	uint8_t i;
	for (i = 0; i<PREPARSE_COUNT; i++) {
		preparse[i].day = 0;
		preparse[i].hour = 0;
		preparse[i].rain = 0;
		preparse[i].temp = 0;
		preparse[i].deg = 0;
		preparse[i].spd = 0;
	}

	for (i = 0; i<FCAST_T_LENGTH; i++) {
		fcastt[i].date = 0;
		fcastt[i].rain = 0;
		fcastt[i].wind = 0;
		fcastt[i].temp_day = 0;
		fcastt[i].temp_night = 0;
	}
	preparse_index = 0;


	if (apstatus == AS_CONNAP) {
		// if set unvarnished transmission mode is successful
		if (ESPSendCmd(esp_cmd_settxmode_unvarn) == ESP_RESULT_OK) {
			Display_SetXY(69,5);
			Display_PutChar('?');
			// if connection to openweathermap is successful
			if (ESPSendCmd(esp_cmd_fcastserver) == ESP_RESULT_OK) {
				// if turning send mode on is successful
				if (ESPSendCmd(esp_cmd_send) == ESP_RESULT_OK) {
					// wait for ">" as sign that we can send stuff
					while (rxlchar != '>') {}
					Display_PutIcon8(icon5_hi,69,5);

					ClrBuffer();
					USendStr(owm_getreq_get);
					USendStr(confstr_owm_cityid);
					USendStr(confstr_owm_opt);
					USendStr(confstr_owm_appid);
					USendStr(owm_getreq_body);

					Display_PutIcon8(icon5_lo,69,5);

					// wait until root node starts

					while (ugetchar() != '{') {}
					parseJSONStream();

					Delay_s(1);
					ClrBuffer();

					USendStr(esp_str_cmdret);
					delay_ms(100);
					ESPSendCmd(esp_cmd_cipclose);

				}
			}
			Delay_s(1);
		}
	}
}

void Graph_Fcast() {
	Display_SetXY(0,0);
	uint8_t col_index;
	char conv[5];

	Display_SetXY(4,0);
	Display_PutChar('#');

	Display_PutIcon8(icon5_hi,4,2);
	Display_PutIcon8(icon5_lo,4,3);

	Display_SetXY(4,4);
	Display_PutChar('=');

	Display_PutIcon8(icon12_cloud,0,5);

	for (col_index = 0; col_index<FCAST_T_LENGTH; col_index++) {
		uint8_t cx = col_index*16+20;

		// display day
		Display_SetXY(cx,0);
		itoa(fcastt[col_index].date,conv,10);
		Display_PutS(conv);
		// display day temp

		if (fcastt[col_index].temp_day<0) {
			Display_SetXY(cx-3,2);
			Display_PutChar('-');
		}
		Display_SetXY(cx,2);
		itoa(abs(fcastt[col_index].temp_day),conv,10);
		Display_PutS(conv);

		// display night temp

		if (fcastt[col_index].temp_night<0) {
			Display_SetXY(cx-3,3);
			Display_PutChar('-');
		}
		Display_SetXY(cx,3);
		itoa(abs(fcastt[col_index].temp_night),conv,10);
		Display_PutS(conv);

		// display wind speed
		Display_SetXY(cx,4);


		if (fcastt[col_index].wind>99)
			Display_PutChar('X');
		else {
			itoa(fcastt[col_index].wind,conv,10);
			Display_PutS(conv);
		}


		Display_SetXY(cx,5);
		if (fcastt[col_index].rain == 0)
			Display_PutChar(' ');
		else {
			Display_PutChar('^');
		}

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

void COMMInit(uint32_t usartbaud) {
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
		usart_struct.USART_BaudRate            = usartbaud;
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
			return 86;
}

void ClrBuffer(void) {
	uint16_t i;
	for (i = 0; i<RXBUF_LEN; i++)
		rxbuf[i] = '\0';
	rxbuf_index = RXBUF_START;
	rxlchar = '\0';
}

enum E_ESPRESULT ESPSendCmd(const char *cmd) {
	ClrBuffer();
	USendStr(cmd);
	USend(13);
	USend(10);
	delay_ms(200);
	uint8_t att = 0;
	enum E_ESPRESULT result = ESP_RESULT_NONE;
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
	return result;
}


void ESPReset(void) {
	GPIO_ResetBits(ESP_PORT,ESP_RST);
	Delay_s(1);
	GPIO_SetBits(ESP_PORT,ESP_RST);
	Delay_s(2);

}

void WLAN_Status(enum E_APSTATUS as, uint8_t x, uint8_t y) {
	switch (as) {
		case AS_NOCONN:
			Display_PutIcon8(icon8_wlan_noconn,x,y);
			break;
		case AS_CONNAP:
			Display_PutIcon8(icon8_wlan_conn,x,y);
			break;
		case AS_NOESP:
			Display_PutIcon8(icon8_wlan_noesp,x,y);
			break;
		default: break;
	}
}

void Delay_s(uint8_t s) {
	while (s) {
		delay_ms(1000);
		s--;
	}
}



void I2CInit(void) {

	RCC_APB1PeriphClockCmd(I2C_IfRCC,ENABLE);

	GPIO_InitTypeDef gis;
	gis.GPIO_Pin = I2C_Pin_SCL | I2C_Pin_SDA;
	gis.GPIO_Speed = GPIO_Speed_50MHz;
	gis.GPIO_Mode = GPIO_Mode_AF;
	gis.GPIO_OType = GPIO_OType_OD;
	gis.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(I2C_IfPort, &gis);

	GPIO_PinAFConfig(I2C_IfPort,I2C_PS_SCL, GPIO_AF_4);
	GPIO_PinAFConfig(I2C_IfPort,I2C_PS_SDA, GPIO_AF_4);

	I2C_InitTypeDef i2cis;
	i2cis.I2C_Mode = I2C_Mode_I2C;
	i2cis.I2C_Ack = I2C_Ack_Disable;
	//i2cis.I2C_Timing = 0x00901A53;
	i2cis.I2C_Timing = 0x20303E5D;
	i2cis.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C_If,&i2cis);

	I2C_Cmd(I2C_If,ENABLE);
}



void Display_PutIcon8(uint8_t *icon, uint8_t x, uint8_t y) {
	uint8_t width = *icon++;
	Display_SetXY(x,y);
	while (width) {
		Display_Put(*icon++);
		width--;
	}
}

enum E_APSTATUS CheckAP(void) {
	if (ESPSendCmd(esp_cmd_at) == ESP_RESULT_OK) {
		if (ESPSendCmd(esp_cmd_queryap) == ESP_RESULT_OK) {
			if (strstr(rxbuf,s_noap) != NULL) {
				return AS_NOCONN;
			} else {
				return AS_CONNAP;
			}
		}
	} else {
		return AS_NOESP;
	}
}

enum E_APSTATUS ConnectAP(void) {
	if (ESPSendCmd(esp_cmd_at) == ESP_RESULT_OK) {
		uint16_t len = strlen(esp_cmd_joinap) + strlen(confstr_ap_name) + strlen(confstr_ap_pwd);
		char connect_cmd[len+1];
		strcpy(connect_cmd,esp_cmd_joinap);
		strcat(connect_cmd,confstr_ap_name);
		strcat(connect_cmd,",");
		strcat(connect_cmd,confstr_ap_pwd);
		if (ESPSendCmd(connect_cmd) == ESP_RESULT_OK) {
			return AS_CONNAP;
		} else {
			return AS_NOCONN;
		}
	} else {
		return AS_NOESP;
	}
}

void parseJSONStream() {
	rxlchar = '\0';
	uint8_t level = 1;
	enum E_JSONBLOCK_ID block = JB_NONE;
	JSON_STRUCT * node = &node_3h_3;
	while (level>0) {
		char c = ugetchar();
		switch (c) {
		case '{':
			level++;
			break;
		case '}':
			level--;
			break;
		case '"':
			if (node == 0) {
				node = &node_3h_3;
				block = JB_NONE;
			}
			break;
		default:
			if (node != 0) {
				if (node->key != '\0') {
					node = getnext(node,c);
				} else {
					block = node->bnum;
					processBlock(block);
					node = &node_3h_3;
					block = JB_NONE;
				}
			}
			break;
		}
	}
}

void processBlock(enum E_JSONBLOCK_ID id) {
	switch (id) {
	case JB_TEMP:
		// get temperature
		while (ugetchar() != '"') {}
		while (ugetchar() != ':') {}
		rxbuf_index = RXBUF_START;
		while (ugetchar() != ','); {}
		preparse[preparse_index].temp = (int8_t)atoi(rxbuf);
		break;
	case JB_DEG:
		// get wind direction
		rxbuf_index = RXBUF_START;
		while (ugetchar() != '}') {}
		preparse[preparse_index].deg = (uint16_t)atoi(rxbuf);
		rxlchar = '}';
		break;
	case JB_SPEED:
		// get wind speed
		rxbuf_index = RXBUF_START;
		while (ugetchar() != ',') {}
		preparse[preparse_index].spd = (uint8_t)atoi(rxbuf);;
		break;
	case JB_3H:
		// get rain if any
		preparse[preparse_index].rain = 1;
		break;
	case JB_DT_TXT:
		// get day of date
		while (ugetchar() != '-') {}
		while (ugetchar() != '-') {}
		rxbuf_index = RXBUF_START;
		while (ugetchar() != ' ') {}
		preparse[preparse_index].day = (uint8_t)atoi(rxbuf);

		// get hour of date
		rxbuf_index = RXBUF_START;
		while (ugetchar() != ':') {};
		preparse[preparse_index].hour = (uint8_t)atoi(rxbuf);

		preparse_index++;
		break;
	default:
		break;
	}
}

void ProcessForecast(void) {
	uint8_t fi = 0;
	uint8_t pi = 0;
	uint8_t day = preparse[0].day;

	c_wind_dir = -1;
	c_wind_dir = getWindDir(preparse[0].deg);
	c_wind_spd = preparse[0].spd;


	int8_t t_day = 0;
	int8_t t_night = 0;
	while (fi<FCAST_T_LENGTH) {
		uint8_t d = preparse[pi].day;
		if (d != day) {
			uint8_t h = preparse[pi].hour;
			if (h == 15) {
				fcastt[fi].temp_day = preparse[pi].temp;
				fcastt[fi].wind = preparse[pi].spd;
				fcastt[fi].rain = preparse[pi].rain;
			}
			if (h == 18) {
				fcastt[fi].date = d;
				fcastt[fi].temp_night = preparse[pi].temp;
				fi++;
			}
		}
		pi++;
	}
}

char ugetchar(void) {
	char c;
	while (rxlchar == '\0') {}
	c = rxlchar;
	rxlchar = '\0';
	return c;
}

uint8_t getWindDir(uint16_t deg) {
	uint16_t wd = deg;
	uint8_t i = 0;

	if (wd<21) {
		wd += 338;
	}

	for (i=0; i<8; i++) {
		if ((wd >= wdir[i].deg) && (wd <= wdir[i].deg+45)) {
			break;
		}
	}
	return i;
}


