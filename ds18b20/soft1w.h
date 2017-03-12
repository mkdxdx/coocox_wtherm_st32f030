#define OW_PORT GPIOB
#define OW_DQ GPIO_Pin_1
#define OW_PORT_C	RCC_AHBPeriph_GPIOB

#define OW_CMD_SRROM			(uint8_t)0xF0	// search rom
#define OW_CMD_RDROM			(uint8_t)0x33	// read rom
#define OW_CMD_MTROM			(uint8_t)0x55	// match rom
#define OW_CMD_SKROM			(uint8_t)0xCC	// skip rom
#define OW_CMD_WRSCR			(uint8_t)0x4E	// write scratchpad
#define OW_CMD_ASRCH			(uint8_t)0xEC	// alarm search
#define OW_CMD_RDSCR			(uint8_t)0xBE	// read scratchpad
#define OW_CMD_CPSCR			(uint8_t)0x48	// copy scratchpad
#define OW_CMD_RECEM			(uint8_t)0xB8	// recall EEPROM
#define OW_CMD_RDPWS			(uint8_t)0xB4	// read powersupply



void OW_SetBus(void);
uint8_t OW_Reset(void);
void OW_Write(uint8_t bit);
void OW_WriteByte(uint8_t data);
uint8_t OW_Read(void);
uint8_t OW_ReadByte(void);
uint8_t OW_SkipROM(void);

