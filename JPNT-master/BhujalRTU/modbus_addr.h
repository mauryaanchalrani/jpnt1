
/*
 * modbus_addr.h
 *
 * Created: 10/5/18 9:54:49 AM
 *  Author: Mindfield
 */ 

#ifndef _MODBUS_ADDR_H_
#define _MODBUS_ADDR_H_

#include "clock.h"
#include <stdio.h>
//#include "gprs.h"
#include "main.h"

#define NUM_OF_MB_IO					(16)
#define NUM_OF_REG						(23)
#define MAX_MODBUS_PKT_SIZE				(96)
#define INVALID_INDEX_OF_MB_REG_ADDR	(0x40025)
#define INVALID_INDEX_OF_MB_REG			(0x40023)
#define START_MB_REG_ADDR				(0x40001)
#define DATA_INDX_OF_MBTCP				(11)
#define KEEP_ALIVE_MBTCP_HEADER_SIZE	(7)

#define POS_ON_MAP_TABLE				(8)
#define MB_DEVICE_ID					(100)

typedef struct
{
	unsigned long mb_add;
	//unsigned char rtu_io_num;
	unsigned char* rtu_ports;
	unsigned char bit_num;
}rtu_pins_t;

typedef struct
{
	long start_addr;
	void*  ram_addr;
}rtu_register_addr_t;


typedef struct
{
	// Data status
	unsigned int data_status;  //2
	// Pump data
	unsigned int system_status;  //2
	unsigned int io_status;     //2
	//unsigned char op_status;  //2
	unsigned int inst_flow;     //2   
	unsigned long cum_flow;     //4
	unsigned int pressure_in;   //2
	unsigned int pressure_out;  //2
	
	// Energy meter data
	float V_R; //unsigned char & int //4
	//unsigned char V_R_dp; //unsigned char & int
	float V_Y;                      //4
	//unsigned char V_Y_dp;
	float V_B;                      //4
	//unsigned char V_B_dp;
	float I_R;                      //4
	//unsigned char I_R_DP;
	float I_Y;                     //4  
	//unsigned char I_Y_DP;
	float I_B;                    //4
	//unsigned char I_B_DP;
	float PF_R;                   //4
	//char PF_R_dp;
	float PF_Y;                   //4 
	//char PF_Y_dp;
	float PF_B;                   //4 
	//char PF_B_dp;
	float PF_AVG;                //4
	//char PF_AVG_dp;
	float KWh;                  //4
	//unsigned char KWh_dp; // char
	float KVah;                 //4
	//unsigned char KVah_dp;
	
	// time data
	time_stamp_t time;         //6   
	
}mb_ram_data_t; // 70 byte

typedef struct
{
	unsigned int transction_id;  //2
	unsigned int protocol_id;    //2
	unsigned int total_pkt_lgth; //2
	unsigned char slave_id;      //1
	unsigned char func_code;     //1
	unsigned char length_of_data; //1
} __attribute__((packed)) mbtcp_payload_data_t;

void copyRamDataToMbRamData(ram_data_t *);
int mbPrepareGPRSData(char *, long, int );
void updateMbTcpPayload(void);
void addMbTcpHeader(char * );
void resetMbTcpTransctionCnt(void);

#endif