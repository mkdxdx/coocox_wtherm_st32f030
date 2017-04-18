/*

	STM32F030 WiFi Forecast station
	by mkdxdx
	MIT license

*/

//#define USE_SHDEBUG	// use semihosting?

#include <stm32f0xx.h>
#include <stdlib.h>
#include <delay.h>
#include <stdint.h>
#include <string.h>


#include <confstr.h> /* This file contains config information in the following format */
/*
const char confstr_ap_name[] = "\"WIFI_AP_NAME\"";
const char confstr_ap_pwd[] = "\"WIFI_AP_PASSWORD\"";
const char confstr_owm_cityid[] = "id=CITY_ID";
const char confstr_owm_appid[] = "&appid=OPENWEATHERMAP_API_KEY";
const char confstr_owm_opt[] = "&OPTION=VALUE"; // e.g. "&units=metric"
 */

#include <json_blocks.h>
#include <ds18b20/soft1w.h>
#include <PCD8544/pcd8544.h>
#include <bmp180/bmp180.h>
#include "jsontrie.h"

#define LED_GPIO_CLK   RCC_AHBPeriph_GPIOA
#define LED_PORT   	   GPIOA
#define LED_PIN        GPIO_Pin_4

#define ESP_PORT		GPIOA
#define ESP_RST			GPIO_Pin_6
#define	ESP_GPIO_CLK	RCC_AHBPeriph_GPIOA

#define I2C_If			I2C1
#define I2C_IfPort		GPIOA
#define I2C_IfPortRCC 	RCC_AHBPeriph_GPIOA
#define I2C_IfRCC		RCC_APB1Periph_I2C1
#define I2C_Pin_SCL		GPIO_Pin_9
#define I2C_Pin_SDA		GPIO_Pin_10
#define I2C_PS_SCL		GPIO_PinSource9
#define I2C_PS_SDA		GPIO_PinSource10


#define RXBUF_LEN 64
#define RXBUF_START 0
#define DIGIT_BMP_WIDTH	16
#define DS18_CMD_CONVT			0x44	// Convert T
#define mmhgppa 0.007500617F
#define FCAST_T_LENGTH	4
#define PREPARSE_COUNT 40
#define CMD_STR_LEN = 11
#define ESP_MAX_ATTEMPTS	7
#define WLAN_CHECK_COUNT	10
#define FCAST_SAVE_TICKS	10

enum E_DISPLAYMODE {
	DM_TEMPONLY = 0,
	DM_VERBOSE,
	DM_FORECAST,
	DM_STATUS
};
enum E_APSTATUS {
	AS_NOCONN = 0,	// ESP is present and not connected
	AS_NOESP,		// ESP is not available (AT command timeout)
	AS_CONNAP		// ESP is connected to AP
};
enum E_RXMODE {
	RM_NORMAL = 0,
	RM_CHARWAIT,
	RM_STRWAIT,
	RM_TIMEOUT
};
enum E_ESPRESULT {
	ESP_RESULT_NONE = 0,
	ESP_RESULT_OK,
	ESP_RESULT_FAIL,
	ESP_RESULT_ERROR,
	ESP_RESULT_TIMEOUT
};
enum E_FCSTATUS {
	FCS_UNCHANGED = 0,
	FCS_CHANGED
};

enum E_PARSEMODE {
	PM_NAME = 0,
	PM_VALUE
};

// forecast table
typedef struct FCAST_C{
  uint8_t date;
  int8_t temp_day;
  int8_t temp_night;
  uint8_t wind;
  uint8_t rain;
};

// temporary json parse result
typedef struct PREPARSE_STRUCT_T {
	uint8_t day;
	uint8_t hour;
	int8_t temp;
	uint8_t rain;
	uint8_t spd;
	uint16_t deg;
};

// wind constants table
typedef struct WINDDIR_T {
	uint16_t deg;
	char dir[3];
};

void LED_Init();
void PeriphInit();
void ESPReset();
void SensorInit();
void PollSensors();
void DisplayStats();
int8_t DS18_Measure();
enum E_ESPRESULT ESPSendCmd(const char *cmd);
void ClrBuffer(void);
void Delay_s(uint8_t s);
void WLAN_Status(enum E_APSTATUS as, uint8_t x, uint8_t y);
void COMMInit(uint32_t usartbaud);
void TIM_ResetStart(void);
void TIM_Stop(void);
void ITInit(void);
void Display_PutIcon8(uint8_t *icon, uint8_t x, uint8_t y);
enum E_APSTATUS CheckAP(void);
enum E_APSTATUS ConnectAP(void);
void parseJSONStream();
char ugetchar(void);
void processBlock(enum E_JSONBLOCK_ID id);
uint8_t getWindDir(uint16_t deg);


#ifndef USE_API_NONE
void Graph_Fcast();
void Forecast_Save(void);
void Forecast_Load(void);
void GetForecast(void);
#endif
