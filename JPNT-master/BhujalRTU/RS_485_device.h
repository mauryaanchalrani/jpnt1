/*
 * PLC_comms.h
 *
 * Created: 7/12/18 1:00:39 PM
 *  Author: Mindfield
 */ 

#ifndef RS_485_DEVICE_H_
#define RS_485_DEVICE_H_

#include "ModbusRtu.h"
#include "common.h"
#include <stddef.h>
#include <inttypes.h>

#define MB_ERROR_RESP_PCK_SIZE (6)
#define MB_RSP_DATA_INDX	(3)

#define EM_PF_AVG		(0)
#define EM_PF_R			(4)
#define EM_PF_Y			(8)
#define EM_PF_B			(12)
#define EM_V_R			(0)
#define EM_V_Y			(4)
#define EM_V_B			(8)
#define EM_I_R			(16)
#define EM_I_Y			(20)
#define EM_I_B			(24)
#define EM_KWH			(32)
#define EM_KWAH			(36)

typedef enum
{
	QUERY_0,
/*	QUERY_1,
	QUERY_2,
	QUERY_3,
	QUERY_4,
	QUERY_5,
	QUERY_6,
*/	
	QUERY_MAX
}MB_query_id_t;

typedef struct
{
	MB_query_id_t query_id;
	uint8_t		slave_id;
	uint8_t		fun_code;
	uint16_t	regi_start_addr;
	uint16_t	length;
	//uint16_t*	data_ptr;
}modbus_payload_t;

typedef enum
{
	MB_QUERY,   
	MB_RD_POLL,	
	MB_WR_POLL,	
		
}MB_handler_state_t;



/*
extern unsigned int data1;
extern unsigned int data2;
extern unsigned int data3;
extern unsigned int data4;
extern unsigned int data5;
*/



// 22-2-19 VC: need to change for req. slave id, reg_addr and num of bytes
const modbus_payload_t modbus_cmd[5]=
{	//Query id			//slave_id				fun_code				regi_start_addr		length	
	//{	QUERY_0,	DEV_EM_MB_ADDR,		MB_FC_READ_REGISTERS,			0x009C,				1},
	{	QUERY_0,	DEV_EM_MB_ADDR,		MB_FC_READ_INPUT_REGISTER,		0x0002,				0x02},
	/*{	QUERY_0,	DEV_EM_MB_ADDR,		MB_FC_READ_REGISTERS,			0x8E,			20},
	{	QUERY_1,	DEV_EM_MB_ADDR,		MB_FC_READ_REGISTERS,			0x74,			8},
	{	QUERY_2,	DEV_EM_MB_ADDR,		MB_FC_WRITE_REGISTER,			0x9E,			0x64},
	{	QUERY_3,	DEV_PLC_MB_ADDR,	MB_FC_READ_COILS,				0x4000,			  	0x05},*/



	//....................................................................................//				
	{	QUERY_MAX,		0xFF,				0x00,							0x0000},
};

float ModbusHex2Float(char *);
void copyFromModbusReg(uint16_t, char*);
unsigned long ModbusHex2ulong(char *);

class RS485_device
{
	uint8_t device_sts[MAX_NUM_MB_DEVICES];
	uint8_t req_port;
	public:
	void send_data(modbus_t);
	int get_data(void);
	void slave_handler(void);
	uint16_t* get_data_from_cmd_table(modbus_t *);
	void update_dev_sts(uint8_t,uint8_t);
	void set_req_port(uint8_t);
	uint8_t get_req_port(void);
	void reRouteData(int, unsigned char );
	int uart_no;
	unsigned char len;
};




#endif
//#endif /* RS_485_DEVICE_H_ */