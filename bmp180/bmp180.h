#ifndef _BMP180_H
#define _BMP180_H

#define BMP_I2C			I2C1
#define BMPI2C_Port		GPIOA
#define BMPI2C_PortRCC 	RCC_AHBPeriph_GPIOA
#define BMPI2C_RCC		RCC_APB1Periph_I2C1


#define BMP180_A_RD 			0xEF
#define BMP180_A_WR 			0xEE

#define BMP180_CMD_RESET 		0xB6
#define BMP180_CMD_Temp			0x2E
#define BMP180_CMD_Pres			0x34
#define BMP180_OSV0				0x00
#define BMP180_OSV2				0x01
#define BMP180_OSV4				0x02
#define BMP180_OSV8				0x03

#define BMP180_CID				0x55

#define BMP180_REG_MCTRL		0xF4
#define BMP180_REG_SRST			0xE0

#define BMP180_REG_OMSB			0xF6
#define BMP180_REG_OLSB			0xF7
#define BMP180_REG_OXLSB		0xF8

#define BMP180_REG_CALStart		0xBF
#define BMP180_REG_CALEnd	  	0xAA
#define BMP180_REG_CID			0xD0

#define pow2_15					32768
#define pow2_4					16
#define pow2_11					2048


//
void Bmp180InitI2C(void);

// Reads REGADDR register from a sensor
uint8_t Bmp180ReadRegister(uint8_t regaddr);

// Writes REGVALUE to REGADDR of a sensor
void Bmp180WriteRegister(uint8_t regaddr,uint8_t regvalue);

// Reads two-byte register, REGADDR is an address of an MSB of a register
long Bmp180ReadRegister16( uint8_t regaddr);

// Checks if sensor is available by reading Chip ID, returns 1 if present, 0 if no or communication failure
uint8_t Bmp180CheckCID(void);

// Reads uncompensated temperature value
long Bmp180ReadUT(void);

// Reads uncompensated pressure value
unsigned long Bmp180ReadUP(uint8_t oss);

// Calculates true temperature in C degrees
long Bmp180GetT(void);

// Calculates true pressure in Pa
long Bmp180GetP(uint8_t oss);

// Reads calibration data from sensor's EEPROM
void Bmp180Calibrate(void);

#endif
