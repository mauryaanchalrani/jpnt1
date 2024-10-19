
/*
 * sensor.h
 *
 * Created: 8/22/18 12:13:19 PM
 *  Author: Mindfield
 */ 
#ifndef _SENSOR_H
#define _SENSOR_H

#include "sms.h"


#define CALIBRATION_CONST	(50.25)
#define UNUSED_CAL_DATA		(51)
/*
typedef enum
{
	CAL_DATA_IP_INDEX,
	CAL_DATA_OP_INDEX,
	CAL_DATA_RECV_IP_INDEX,
	CAL_DATA_RECV_OP_INDEX,	
};*/

typedef struct{
	char rtu_id[MOB_NUM_LEN];				// SIM Number of RTU
	unsigned int cal_constant;				// Calibration Constant
} vtc_config_data_t;

typedef struct{
	char sno;								// Serial Number
	char cmltv_msB;							// Most Significant Byte
   	unsigned int cmltv_lsB;					// Lesser Significant Bytes
	char cmltv_dp;							// Decimal Part
	char chksum;							// Checksum
}__attribute__((packed))fm_e2p_data_t;		// this struct is use for cumulative data stored in flash currently not in use

typedef struct
{
	unsigned int cal_data_flow1;
	unsigned int cal_data_flow2;		
	unsigned int cal_data_pressure1;
	unsigned int cal_data_pressure2;	
	unsigned int cal_data_lvl1;
	unsigned int cal_data_lvl2;		
//	unsigned long cum_flow; // 22/11/2018 VC:
	unsigned char unused[UNUSED_CAL_DATA];
	unsigned char chksum;
}__attribute__((packed))calbration_data_t;

float calculateCmltv(float, float);
void storeCumulativeData(void);
void convertCumulativeData(float );
void getCumulativeData(void);
void convertCumulativeData(char*);

#endif
