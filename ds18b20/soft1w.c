/*

	ds18b20 soft1w lib
	by mkdxdx
	use it as you want

*/


#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "soft1w.h"
#include "delay.h"

#define OW_DQ_LO				GPIO_ResetBits(OW_PORT,OW_DQ)
#define OW_DQ_HI				GPIO_SetBits(OW_PORT,OW_DQ)
#define OW_DELAY_LONG			delay_us(90)
#define OW_DELAY_SHORT			delay_us(8)
#define OW_DELAY_RESET			delay_us(500)


inline int8_t OW_Get(void)
{
	return (GPIO_ReadInputDataBit(OW_PORT,OW_DQ));

}

void OW_SetBus()
{
	GPIO_InitTypeDef gis;
	RCC_AHBPeriphClockCmd(OW_PORT_C, ENABLE);

	gis.GPIO_Pin = OW_DQ;
	gis.GPIO_Speed = GPIO_Speed_50MHz;
	gis.GPIO_Mode = GPIO_Mode_OUT;
	gis.GPIO_OType = GPIO_OType_OD;
	gis.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(OW_PORT, &gis);

	OW_DQ_HI;
}

uint8_t OW_Reset(void)
{
	OW_DQ_LO;
	OW_DELAY_RESET;
	OW_DQ_HI;
	OW_DELAY_SHORT;
	uint8_t t;
	for (t = 80; t; t--)
	{
		if (!OW_Get())
		{
			while (!OW_Get());
			return 1;
		}
		delay_us(1);
	}
	return 0;
}



void OW_Write(uint8_t bit)
{
	OW_DQ_LO;
	if (bit)
	{
		OW_DELAY_SHORT;
		OW_DQ_HI;
		OW_DELAY_LONG;
	} else {
		OW_DELAY_LONG;
		OW_DQ_HI;
		OW_DELAY_SHORT;
	}
}

void OW_WriteByte(uint8_t data)
{
	uint8_t c;
	for (c = 8; c; c--)
	{
		OW_Write(data & 1);
		data >>= 1;
	}
}

uint8_t OW_Read(void)
{
	OW_DQ_LO;
	delay_us(2);
	OW_DQ_HI;
	OW_DELAY_SHORT;
	uint8_t bit = OW_Get();
	OW_DELAY_LONG;
	return bit;
}

uint8_t OW_ReadByte(void)
{
	uint8_t byte = 0;
	uint8_t c;
	for (c = 8; c; c--)
	{
		byte >>= 1;
		if (OW_Read())
			byte |= 0x80;
	}
	return byte;
}

uint8_t OW_SkipROM(void)
{
	if (!OW_Reset())
		return 0;
	OW_WriteByte(OW_CMD_SKROM);
	return 1;
}


