/*
 * clock.cpp
 *
 * Created: 17-10-2017 22:35:28
 *  Author: HemeshJain
 */ 

#include <avr/interrupt.h>
#include <string.h>

#include "I2C.h"
#include "clock.h"
#include "ds1307.h"
#include "_debug.h"
#include "common.h"
#include "app_uart.h"
#include "gprs.h"
#include <ctype.h>
#include "config.h"
#include <stdio.h>
#include "Time_Diff.h"

time_main_t time_main;
time_stamp_t time_stamp;
timeDiff_t timeDiff = {0, 0, 0};//abdul majeed

extern gprs_date_time_t gprs_date_time;
void getDateTime(void)
{
	char error = 0;
	int i;
	cli();
	I2CInit();		//Anand 13.07.2014
	uint8_t data;
	memset((void *)&time_main, 1, sizeof(time_main_t));
	for (i = 0; i < 3; i++)
	{	
		error = 0;
		RTC_Read(0x00,&data);  //sec
		time_main.sec = ((((data & 0b01110000) / 16) * 10) + (data & 0b00001111));
		
		RTC_Read(0x01,&data); //min
		time_main.minute = ((((data & 0b01110000) / 16) * 10) + (data & 0b00001111));
		
		RTC_Read(0x02,&data);  //hour
		time_main.hour = ((((data & 0b00010000)/16) * 10) + (data & 0b00001111));
		
		if(data & 0b00100000)
		{
			time_main.ampm = 1;
			if (time_main.hour < 12)
			{
				time_main.hour = time_main.hour+12;
			}
		}
		else
		{
			time_main.ampm = 0;
			if (time_main.hour == 12)
			{
				time_main.hour = 0;
			}
		}
		RTC_Read(0x06,&data); //year
		
		time_main.year = ((((data & 0b11110000)/ 16) * 10) +(data & 0b00001111));
		
		RTC_Read(0x05,&data); //month
		time_main.month = ((((data & 0b00010000) / 16) * 10) + (data & 0b00001111));
		
		RTC_Read(0x04,&data); //date
		time_main.date = ((((data & 0b00110000)/16) * 10) + (data & 0b00001111));
		
		RTC_Read(0x03,&data); //day
		time_main.dan=data & 0b00000111;
		
		if (((time_main.date <= 0) || (time_main.date > 31)) || 
			((time_main.month <= 0) || (time_main.month > 12)) || 
			((time_main.year < 16) || (time_main.year > 99))||			// Assuming that RTC will never go below 2016.
			((time_main.hour < 0) || (time_main.hour > 23)) || 
			((time_main.minute < 0) || (time_main.minute > 59)) ||
			((time_main.sec < 0) || (time_main.sec > 59)))
		{
			error = 1;

		}		
		if (!error)
		{

			
			break;
		}
	}
	I2CDeinit();		//Anand 13.07.2014
	sei();
	if (i >= 3)
	{
		// Generate RTC Error flag in diagnosis.
		time_main.date = 1;
		time_main.month = 1;
		time_main.year = 0;
		time_main.hour = 0;
		time_main.minute = 1;
		time_main.sec = 0;
		error = 1;	
		
	}
	
#ifdef GPRS_ENABLE
	if (gprs_date_time.update_time_aval == TRUE)
	{

		time_sync();		//for E4E RTU
	}
#endif
}
void get_present_time(time_stamp_t *time_stamp)
{
	getDateTime();
	time_stamp->sec = time_main.sec;
	time_stamp->min = time_main.minute;
	time_stamp->hr = time_main.hour;
	time_stamp->dd = time_main.date;
	time_stamp->mm = time_main.month;
	time_stamp->yr = time_main.year;


}
void set_rtc(unsigned char *time)
{
	int indx = 0;
	
	cli();
	I2CInit();
	
	time[indx] = ((time[indx]/10)<<4) | (time[indx]%10);		//Year
	RTC_Write(0x06, time[indx++]);		//Year

	time[indx] = ((time[indx]/10)<<4) | (time[indx]%10);		//Month
	RTC_Write(0x05, (time[indx++] & 0x1F));		//Month

	time[indx] = ((time[indx]/10)<<4) | (time[indx]%10);		//Date
	RTC_Write(0x04, (time[indx++] & 0x3F));		//Date
	
#ifdef RTC_DS3231
	RTC_Write(0x03, (time[indx++] & 0x07));
#endif
#ifdef RTC_MCP79410
	RTC_Write(0x03, ((time[indx++] & 0x07) | (1 << RTC_VBAT_EN_BIT)));		//Day: 1-7, 1=Sunday
#endif	

	if(time[indx] >= 12)
	{
		if(time[indx] > 12)
		{
			time[indx] %= 12;
		}

		time[indx] = ((time[indx]/10)<<4) | (time[indx]%10);		//hr
		time[indx] |= (1<<5);
	}
	else
	{
		if(time[indx] == 0)
		{
			time[indx] = 12;
		}
		time[indx] = ((time[indx]/10)<<4) | (time[indx]%10);		//hr
	}
	time[indx] |= (1<<6);		//Anand 02.07.2014

	RTC_Write(0x02, (time[indx++] & 0x7F));		//hr

	time[indx] = ((time[indx]/10)<<4) | (time[indx]%10);		//Minute
	RTC_Write(0x01, (time[indx] & 0x7F)); //min

#ifdef RTC_DS3231
	RTC_Write(0x00, 0x00);
#endif
#ifdef RTC_MCP79410
	RTC_Write(0x00, (0x30 | (1 << RTC_OSC_EN_BIT)));
#endif	
	I2CDeinit();
	sei();
}

void gettime_from_time_main(time_stamp_t *time_stamp)
{
	time_stamp->sec = time_main.sec;
	time_stamp->min = time_main.minute;
	time_stamp->hr = time_main.hour;
	time_stamp->dd = time_main.date;
	time_stamp->mm = time_main.month;
	time_stamp->yr = time_main.year;
}

void gettime_from_gprs(time_stamp_t *time_stamp)
{
	time_stamp->sec = gprs_date_time.sec;
	time_stamp->min = gprs_date_time.min;
	time_stamp->hr = gprs_date_time.hr;
	time_stamp->dd = gprs_date_time.dd;
	time_stamp->mm = gprs_date_time.mm;
	time_stamp->yr = gprs_date_time.yy;
}

void time_sync(void)
{
	unsigned char data;
	
	if ((gprs_date_time.yy == time_main.year) && (gprs_date_time.mm == time_main.month) && (gprs_date_time.dd == time_main.date) && (gprs_date_time.hr == time_main.hour))
	{

	}
	else
	{
		//get time_main in gprs_date_time format
		
		time_stamp_t temp_maintime,temp_gprstime;
		gettime_from_time_main(&temp_maintime);//get data in timestamp_t format from time_main to temp
		gettime_from_gprs(&temp_gprstime);
		timeDiff = calcTimeDiff(temp_gprstime,temp_maintime);
		if(timeDiff.diffMins>0	&&	timeDiff.diffMins<=10	&&	timeDiff.diffHrs==0		&&	timeDiff.diffDays==0)
		{
			
		}
		
		
		else
		{
			cli();
			I2CInit();
			data = (((gprs_date_time.yy / 10) * 16) | (gprs_date_time.yy % 10));
			RTC_Write(0x06, data);		//Year
			data = (((gprs_date_time.mm / 10) * 16) | (gprs_date_time.mm % 10));
			RTC_Write(0x05, data & 0x1F);		//Month
			data = (((gprs_date_time.dd / 10) * 16) | (gprs_date_time.dd % 10));
			RTC_Write(0x04, data & 0x3F);		//Date
#ifdef RTC_DS3231
			RTC_Write(0x03, (1 & 0x07));
#endif
#ifdef RTC_MCP79410
			RTC_Write(0x03, ((1 & 0x07) | (1 << RTC_VBAT_EN_BIT)));		//Day: 1-7, 1=Sunday
#endif
			if(gprs_date_time.hr >= 12)
			{
				if(gprs_date_time.hr > 12)
				{
					gprs_date_time.hr %= 12;
				}
				gprs_date_time.hr = ((gprs_date_time.hr/10)<<4) | (gprs_date_time.hr%10);		//hr
				gprs_date_time.hr |= (1<<5);
			}
			else
			{
				if(gprs_date_time.hr == 0)
				{
					gprs_date_time.hr = 12;
				}
				gprs_date_time.hr = ((gprs_date_time.hr/10)<<4) | (gprs_date_time.hr%10);		//hr
			}
			gprs_date_time.hr |= (1<<6);		//Anand 02.07.2014

			RTC_Write(0x02, (gprs_date_time.hr & 0x7F));		//hr
			data = (((gprs_date_time.min / 10) * 16) | (gprs_date_time.min % 10));
			RTC_Write(0x01, (data & 0x7F)); //min
			
			data = (((gprs_date_time.sec / 10) * 16) | (gprs_date_time.sec % 10));
			
#ifdef RTC_DS3231
			RTC_Write(0x00, (data & 0x7F)); //sec
#endif
#ifdef RTC_MCP79410
			RTC_Write(0x00, ((data & 0x7F) | (1 << RTC_OSC_EN_BIT)));
#endif
			I2CDeinit();		//Anand 13.07.2014
			sei();
			
			gprs_date_time.update_time_aval = FALSE;

		}
	}
}

unsigned char check_date_time(char *str, char *arr)
{
	int i = 0;
	char *ptr = arr;
	*ptr = 0;
	for (i = 0; i < 17; i++)
	{
		if(isdigit(str[i]))
		{
			*ptr *= 10;
			*ptr += (str[i] - '0');
		}
		else
		{
			if ((i + 1) % 3)
			{
				break;
			}
			else
			{
				switch(i)
				{
					case 2:
					case 5:
						if(str[i] == '/')
						{
							ptr++;
							*ptr = 0;
						}
					break;
					case 8:
						if(str[i] == ',')
						{
							ptr++;
							*ptr = 0;
						}
					break;
					case 11:
					case 14:
						if(str[i] == ':')
						{
							ptr++;
							*ptr = 0;
						}
					break;
					default:
					break;
				}
				
			}
		}
	}
	if (i == 17)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void updatetimefromegprs(time_stamp_t *time_stamp)
{
	time_stamp->yr = gprs_date_time.yy;
	time_stamp->mm = gprs_date_time.mm;
	time_stamp->dd = gprs_date_time.dd;
	time_stamp->hr = gprs_date_time.hr;
	time_stamp->min = gprs_date_time.min;
	time_stamp->sec = gprs_date_time.sec;
}