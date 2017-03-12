// define port
#define Display_PORT	GPIOA
#define Display_PORT_C	RCC_AHBPeriph_GPIOA

#define DisplaySPI		SPI1

// define pins
//#define Display_DAT		6
//#define Display_CLK		7
#define Display_CE		GPIO_Pin_4
#define Display_RST		GPIO_Pin_13
#define Display_DC		GPIO_Pin_14

// define display commands
#define CMD_FUNCSET			(uint8_t)0b00100000
#define CMD_EXTENDED		(uint8_t)0b00100001
#define CMD_CONF_BLANK		(uint8_t)0b00001000
#define	CMD_CONF_NORMAL		(uint8_t)0b00000000
#define CMD_CONF_ALLON		(uint8_t)0b00001001
#define CMD_CONF_INVERSE	(uint8_t)0b00001101
#define CMD_SETX			(uint8_t)0b10000000
#define CMD_SETY			(uint8_t)0b01000000

// define display dimensions
#define Display_Width 		(uint8_t)84
#define Display_Height		(uint8_t)48
#define Display_Length 		Display_Width*6

#define FONT_WIDE

#ifdef FONT_WIDE
#define FONT_CHAR_WIDTH 6
#else
#define FONT_CHAR_WIDTH 3
#endif
#define DIGIT_BMP_WIDTH	16



// comment/uncomment this to enable buffered mode
// if you have enough ram ( > 84*48*sizeof(uint8_t))
// this will drastically increase drawing speed
//#define BUFFERED



void Display_SetPort(void);
void Display_Init(void);
void Display_Reset(void);
void Display_SendData(uint8_t data);
void Display_SendCmd(uint8_t cmd);
void Display_Tx(uint8_t byte);
void Display_TxF(uint8_t byte);
void Display_PutChar(char c);
void Display_DigitVal(char *s, uint8_t x, uint8_t y);
void Display_PutS(const char *s);
void Display_PutDigit(uint8_t n, uint8_t x, uint8_t y);
void Display_SetXY(uint8_t x, uint8_t y);
void Display_Clear(void);

#ifdef BUFFERED
volatile static uint8_t dbuf[Display_Length];
volatile static uint16_t dcurx = 0;
void Display_Swap(void);
void Display_Put(uint8_t data);
void Display_PutAt(uint8_t data, uint8_t x, uint8_t y);
uint16_t Display_CalcP(uint8_t x, uint8_t y);
uint8_t Display_Get();
uint8_t Display_GetAt(uint8_t x, uint8_t y);
void Display_SetCur(uint8_t x, uint8_t y);
#endif
