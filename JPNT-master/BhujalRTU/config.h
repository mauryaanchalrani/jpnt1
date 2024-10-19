#ifndef _CONFIG_H
#define _CONFIG_H

#define HARDWARE_VERSION (1)	

#define GPRS_ENABLE
#ifdef GPRS_ENABLE
#define ONLINE_CONFIG_EN

#endif
//#define TCP_ENABLE
#define ABB_UART			(0)
#define KROHNE_MAR_50_485	(1)

#define FM_TYPE		KROHNE_MAR_50_485

/*#define _4_20 
#define MODBUS
#define PULSE

#define CONN_TYPE	PULSE*/
#ifndef TCP_ENABLE
#define HTTP_ENABLE
#endif

#define FLASH_LOG_DATA
#define RTC_ENABLE

#define v1_0	(1)
#define v2_0	(2)
#define BOARD_VER	(v2_0)

#define SOFT_RST_GSM	//for sim module
//#define TESTING_MODE	//17-06-21 : always disable this debug before release
//#define SUPERCAP

#define JPNT_LIMIT	(1)
#define RSPCB_LIMIT	(2)
#define DAILY_LIMIT_TYPE (RSPCB_LIMIT)

#endif
