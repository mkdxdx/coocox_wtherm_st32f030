/*
	BMP180 Comm library
	by mkdxdx
	use it as you want
*/

// BMP180 Definitions

#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_i2c.h"
#include "stm32f0xx_gpio.h"
#include "bmp180.h"


// Calibration data
short AC1, AC2, AC3, B1, B2, MB, MC, MD;
unsigned short AC4, AC5, AC6;
long B5;

void Bmp180InitI2C(void) {
	RCC_APB1PeriphClockCmd(BMPI2C_RCC,ENABLE);

	GPIO_InitTypeDef gis;
	gis.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	gis.GPIO_Speed = GPIO_Speed_50MHz;
	gis.GPIO_Mode = GPIO_Mode_AF;
	gis.GPIO_OType = GPIO_OType_OD;
	gis.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(BMPI2C_Port, &gis);

	GPIO_PinAFConfig(BMPI2C_Port,GPIO_PinSource9, GPIO_AF_4);
	GPIO_PinAFConfig(BMPI2C_Port,GPIO_PinSource10, GPIO_AF_4);

	I2C_InitTypeDef i2cis;
	i2cis.I2C_Mode = I2C_Mode_I2C;
	i2cis.I2C_Ack = I2C_Ack_Disable;
	i2cis.I2C_Timing = 0x00901A53;
	i2cis.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(BMP_I2C,&i2cis);

	I2C_Cmd(BMP_I2C,ENABLE);
}


uint8_t Bmp180ReadRegister(uint8_t regaddr)
{

	uint8_t regvalue;

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_BUSY) == SET);

	I2C_TransferHandling(BMP_I2C,BMP180_A_WR,1,I2C_SoftEnd_Mode,I2C_Generate_Start_Write);

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_TXIS) == RESET);

	I2C_SendData(BMP_I2C,regaddr);

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_TC) == RESET);

	I2C_TransferHandling(BMP_I2C,BMP180_A_WR,1,I2C_AutoEnd_Mode,I2C_Generate_Start_Read);

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_RXNE) == RESET);

	regvalue = I2C_ReceiveData(BMP_I2C);

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_STOPF) == RESET);

	I2C_ClearFlag(BMP_I2C,I2C_FLAG_STOPF);

	return regvalue;
}


long Bmp180ReadRegister16(uint8_t regaddr)
{
	uint8_t msb = Bmp180ReadRegister(regaddr);
	delay_ms(1);
	uint8_t lsb = Bmp180ReadRegister(regaddr+1);
	return ((msb<<8) + lsb);
}

void Bmp180WriteRegister(uint8_t regaddr,uint8_t regvalue)
{

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_BUSY) == SET);

	I2C_TransferHandling(BMP_I2C,BMP180_A_WR,1,I2C_Reload_Mode,I2C_Generate_Start_Write);

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_TXIS) == RESET);

	I2C_SendData(BMP_I2C,regaddr);

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_TCR) == RESET);

	I2C_TransferHandling(BMP_I2C,BMP180_A_WR,1,I2C_AutoEnd_Mode,I2C_No_StartStop);

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_TXIS) == RESET);

	I2C_SendData(BMP_I2C,regvalue);

	while(I2C_GetFlagStatus(BMP_I2C,I2C_FLAG_STOPF) == RESET);

	I2C_ClearFlag(BMP_I2C,I2C_FLAG_STOPF);
}



uint8_t Bmp180CheckCID(void)
{
	if (Bmp180ReadRegister(BMP180_REG_CID) == BMP180_CID) return 1;
	else return 0;
}


long Bmp180ReadUT(void)
{
	long UT;
	Bmp180WriteRegister(BMP180_REG_MCTRL,BMP180_CMD_Temp);
	delay_ms(7);
	UT = Bmp180ReadRegister16(BMP180_REG_OMSB);
	return UT;	
}

unsigned long Bmp180ReadUP(uint8_t oss)
{
	unsigned long UP;
	
	Bmp180WriteRegister(BMP180_REG_MCTRL,BMP180_CMD_Pres | (oss<<6));
	delay_ms(35);
	uint8_t msb = Bmp180ReadRegister(BMP180_REG_OMSB);
	delay_ms(1);
	// remove 1ms delays
	// and write multibyte register read routine via count argument
	uint8_t lsb = Bmp180ReadRegister(BMP180_REG_OLSB);
	delay_ms(1);
	uint8_t xlsb = Bmp180ReadRegister(BMP180_REG_OXLSB);
	UP = ((((unsigned long)msb<<16) | ((unsigned long)lsb<<8) | (unsigned long)xlsb) >> (8 - oss));
	return 	UP;
}

long Bmp180GetT(void)
{
	long UT = Bmp180ReadUT();
	long X1,X2,T;
	
	X1 = (UT - AC6) * AC5 / pow2_15;
	X2 = (MC * pow2_11) / (X1 + MD);
	B5 = (X1 + X2);
	T = (B5 + 8) / pow2_4;
	return T;
}

long Bmp180GetP(uint8_t oss)
{
	long X1, X2, X3, B3, B6, p;
	unsigned long B4, B7;
	unsigned long UP = Bmp180ReadUP(oss);
	
	B6 = B5 - 4000;
	X1 = (B2 * (B6 * B6)>>12)>>11;
	X2 = (AC2 * B6)>>11;
	X3 = X1 + X2;
	B3 = (((((long)AC1)*4 + X3)<<oss)+2)>>2;
	X1 = (AC3 * B6)>>13;
	X2 = (B1 * ((B6*B6)>>12))>>16;
	X3 = ((X1+X2)+2)>>2;
	B4 = (AC4 * (unsigned long)(X3 + 32768))>>15;
	B7 = ((unsigned long)UP-B3)*(50000>>oss);
	if (B7<0x80000000UL) p = (B7<<1)/B4;
	else p = (B7/B4)<<1;
	X1 = (p>>8) * (p>>8);
	X1 = (X1*3038)>>16;
	X2 = (-7357*p)>>16;
	p += (X1 + X2 + 3791)>>4;
	
	return p;
}

void Bmp180Calibrate(void)
{
	AC1 = Bmp180ReadRegister16(0xAA);
	AC2 = Bmp180ReadRegister16(0xAC);
	AC3 = Bmp180ReadRegister16(0xAE);
	AC4 = Bmp180ReadRegister16(0xB0);
	AC5 = Bmp180ReadRegister16(0xB2);
	AC6 = Bmp180ReadRegister16(0xB4);
	B1 = Bmp180ReadRegister16(0xB6);
	B2 = Bmp180ReadRegister16(0xB8);
	MB = Bmp180ReadRegister16(0xBA);
	MC = Bmp180ReadRegister16(0xBC);
	MD = Bmp180ReadRegister16(0xBE);
}
