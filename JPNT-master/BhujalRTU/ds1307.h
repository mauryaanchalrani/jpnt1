#ifndef DS1307_H
#define DS1307_H

#include <avr/io.h>

#define RTC_DS3231	

#ifndef RTC_DS3231
#define RTC_MCP79410	
#endif

#ifdef RTC_DS3231
#define RTC_ADDR		(0xD0)				// For DS3231
#endif
#ifdef RTC_MCP79410
#define RTC_ADDR		(0xDE)				// For MCP79410
#define RTC_OSC_EN_BIT		(7)	
#define RTC_VBAT_EN_BIT		(3)
#endif

#define RTC_ADDR_SEC	(0x00)
#define RTC_ADDR_MIN	(0x01)
#define RTC_ADDR_HOUR	(0x02)
#define RTC_ADDR_WKDAY	(0x03)
#define RTC_ADDR_DATE	(0x04)
#define RTC_ADDR_MONTH	(0x05)
#define RTC_ADDR_YEAR	(0x06)

uint8_t RTC_Read(uint8_t address,uint8_t *data);
uint8_t RTC_Write(uint8_t address,uint8_t data);

#endif
