

#include <avr/io.h>

#include "I2C.h"
#include "common.h"
#include "ds1307.h"

uint8_t RTC_Read(uint8_t address,uint8_t *data)
{
	uint8_t res;	//result
	
	//Start
	res = I2CStart();
	// HJ 05-07-2017
	if(!res)	return FALSE;
	
	//SLA+W (for dummy write to set register pointer)
	//res=I2CWriteByte(0b11010000);	//DS1307 address + W
	res=I2CWriteByte(RTC_ADDR);	
	//Error
	if(!res)	return FALSE;
	
	//Now send the address of required register
	res=I2CWriteByte(address);
	
	//Error
	if(!res)	return FALSE;
	
	//Repeat Start
	res = I2CStart();
	// HJ 05-07-2017
	if(!res)	return FALSE;
	
	//SLA + R
	//res=I2CWriteByte(0b11010001);	//DS1307 Address + R
	res=I2CWriteByte(RTC_ADDR | 1);	
	//Error
	if(!res)	return FALSE;
	
	//Now read the value with NACK
	res=I2CReadByte(data,0);
	
	//Error
	if(!res)	return FALSE;
	
	//STOP
	res = I2CStop();
	// HJ 05-07-2017
	if(!res)	return FALSE;
	
	return TRUE;
}

uint8_t RTC_Write(uint8_t address,uint8_t data)
{
	uint8_t res;	//result
	//Start
	res = I2CStart();
	// HJ 05-07-2017
	if(!res)	return FALSE;
	
	//SLA+W
	//res=I2CWriteByte(0b11010000);	//DS1307 address + W
	res=I2CWriteByte(RTC_ADDR);	
	//Error
	if(!res)	return FALSE;
	
	//Now send the address of required register
	res=I2CWriteByte(address);
	
	//Error
	if(!res)	return FALSE;
	
	//Now write the value
	res=I2CWriteByte(data);
	
	//Error
	if(!res)	return FALSE;
	
	//STOP
	res = I2CStop();
	// HJ 05-07-2017
	if(!res)	return FALSE;
	
	return TRUE;
}

/*
void configRtc()
{
	uint8_t temp=0;
	//Clear CH bit of RTC
	#define CH 7
	DS1307Read(0x00,&temp);				//Changed By HJS

	//Clear CH Bit
	temp&=(~(1<<CH));

	DS1307Write(0x00,temp);

	//Set 12 Hour Mode
	DS1307Read(0x02,&temp);

	//Set 12Hour BIT
	temp|=(0b01000000);

	//Write Back to DS1307
	DS1307Write(0x02,temp);
}*/	
	
	

