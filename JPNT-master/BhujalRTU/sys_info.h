
/*
 * sys_info.h
 *
 * Created: 20-May-15 10:32:08 AM
 *  Author: myPc
 */ 


#ifndef SYS_INFO_H_
#define SYS_INFO_H_

#include "config.h"

#define PRODUCT_CODE			"RTU"
#define VER_LEN					(8)
#define PROTOCOL_VER			"01.00"
#ifdef TESTING_MODE
	#define FIRMWARE_VER			"01.23"
#else
	#define FIRMWARE_VER			"01.23"
#endif
#define SYS_INFO_RESERVED_BYTES	(7)
#define FIRMWARE_VER_LEN		(5)

#define SER_NUM_LEN					(10)

#define PRODUCT_INFO_RESERVED_BYTES (2)

	
#define INDUS_NAME_LEN				(20)	
#define MIN_PRODUCT_ID		(0)
#define MAX_PRODUCT_ID		(65535)
#define MIN_PORT_ADDR		(1024)
#define MAX_PORT_ADDR		(65535)


#define CALIB_FACTOR_LEN	(4)
//for online configuration
#ifdef ONLINE_CONFIG_EN
#define ONLINE_CFG_RESERVED_BYTES	(11)
#define CONFIG_RESP_LENGTH	(73)
#define VER_CNT_RESP_LENGTH	(2)
#define DAILY_LIMIT_LEN		(3)
#define TOTALIZER_LEN		(10)
#define TOTALIZER_DP		(8)
#define TIME_SLOT_LEN		(4)
#define CAL_FAC_LEN_ON_CNFG	(5)
#define ADC_OFFSET_LEN		(4)
#define FM_MAKE_LEN			(2)
#define CON_TYPE_LEN		(1)
#endif
/*
typedef enum
{
	
};*/
typedef struct
{
	char protocol_ver[VER_LEN];
	char firmware_ver[VER_LEN];
	char user_type;
	char reserved[SYS_INFO_RESERVED_BYTES];
}sys_info_t;
/*
typedef struct
{
	//unsigned long int product_serial_num;
	//unsigned char product_serial_num[SER_NUM_LEN];
	unsigned long product_id;
	unsigned char reserved[PRODUCT_INFO_RESERVED_BYTES];
	unsigned char chksum;
}__attribute__((packed)) product_info_t;
*/

#ifdef ONLINE_CONFIG_EN			//20-03-2020 SK: added for online configuration
typedef enum
{
	CFG_VER_CNT,
	CFG_DW_DATA,
	CFG_UPDATED,
}online_cfg_state_t;
#endif
typedef enum
{
	ABB = 1,
	DMS,
	KROHNE_MAR_50,
	KROHNE_MAR_100,
	KROHNE_MAR_11,
	KROHNE_MAR_300,
}flow_meter_make_t;

typedef enum
{
	FM_CONN_ADC420 = 1,
	FM_CONN_MB,
	FM_CONN_PULSE,
	FM_CONN_485,
	FM_CONN_232,
	FM_CONN_ADC
}connection_type_t;


typedef struct
{
	unsigned int industry_id;							//2
	unsigned int daily_limit;							//2
	unsigned char ind_name[INDUS_NAME_LEN];				//20
	//flow_meter_type 
	unsigned int ADC_offset;							//2
	unsigned int calib_factor;							//2
	unsigned char daily_limit_dp;						//1
	unsigned char reserved[PRODUCT_INFO_RESERVED_BYTES];//2
	unsigned char chksum;								//1
}__attribute__((packed)) product_info_t;				//32

#ifdef ONLINE_CONFIG_EN
typedef struct
{
	unsigned int time_slot;					//2
	flow_meter_make_t flow_meter_make;//unsigned char flow_meter_make;			//1
	connection_type_t connection_type;//unsigned char connection_type;			//1
	unsigned char reserved[ONLINE_CFG_RESERVED_BYTES];//11
	unsigned char chksum;					//1

}__attribute__((packed)) product_info_config_t;				//16
#endif
#endif /* SYS_INFO_H_ */