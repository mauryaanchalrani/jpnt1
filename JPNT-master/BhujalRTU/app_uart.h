
/*
 * app_uart.h
 *
 * Created: 11-12-2013 10:41:11
 *  Author: HemeshJain
 */ 


#ifndef APP_UART_H_
#define APP_UART_H_
#include <stdint-gcc.h>
#define PORT_RSP_TIMEOUT (3)
#define NUM_OF_RESPONSE_BYTE	(9)

typedef enum 
{
	CMD_CONNECT = 0x01,				//1	connect
	CMD_DISCONNECT,					//2 disconnect
	CMD_GET_CALIBRATION,			//3	calibration mode with cal step
	CMD_SET_CALIBRATION,			//4	calibration mode with cal step
	CMD_UPDATE_DATE_TIME,			//5 set Date and time
	CMD_FACTORY_DEFAULTS,			//6 set Factory Default
	CMD_GET_GPRS_CFG,				//7 read GPRS Configuration
	CMD_SET_GPRS_CFG,				//8 edit GPRS Configuration
	CMD_GET_SIM_NUM,				//9 read SIM num
	CMD_SET_SIM_NUM,				//A edit SIM num
	CMD_GET_PORT_ADDR,				//B read PORT addr
	CMD_SET_PORT_ADDR,				//C edit port addr
	CMD_GET_IP_ADDR,				//D read ip addr
	CMD_SET_IP_ADDR,				//E edit ip addr
	CMD_GET_CUM_DATA,				//F read SIM Configuration
	CMD_SET_CUM_DATA,				//10 edit SIM Configuration
	CMD_GET_CALIB_FACTOR,			//11 read product id
	CMD_SET_CALIB_FACTOR,			//12 edit product id
	CMD_GET_DATA_LOG,				//13 read data log
	CMD_CLEAR_DATA_LOG,				//14 edit data log
	CMD_GET_KWH,					//15 read Kwh
	CMD_SET_KWH,					//16 edit kwh
	CMD_GET_KWAH,					//17 read kwah
	CMD_SET_KWAH,					//18 edit kwah
	CMD_REPET_SEND_DATA_LOGS,		//19
	CMD_MB_QUERY,					//1A MB query
	GET_DAILY_LIMIT,				//1B
	SET_DAILY_LIMIT,				//1C
	READ_GPRS_CFG			= 0x20,//20H
	EDIT_GPRS_CFG			= 0x21,//21H
	GET_PRODUCT_SER_NUM,		//22
	SET_PRODUCT_SER_NUM,		//23
	GET_TOTALIZER,				//24
	SET_TOTALIZER,				//25
	//SET_Y_TOTAL,				//26	//abdul majeed
	DEBUG_RS485_CMD			= 0x27,
	DEMO_HEX_FILE_WRITE_INIT= 0x41,		//41
	DEMO_HEX_FILE_WRITE,				//42
	DEMO_HEX_FILE_READ,					//43
	SET_E2P_HEX_BIT,				//44
	
}cmd_t;

/*
typedef enum
{
	SET_MODE = 0x01,			//1
	READ_BOWSER_STS,			//2
	READ_BOWSER_CFG,			//3
	EDIT_BOWSER_CFG,			//4
	ADD_TRUCK_ID,				//5
	DEL_TRUCK_ID,				//6
	EDIT_TRUCK_ID,				//7
	READ_AUTHORIZED_IDS,		//8
	UPLOAD_DISPENSE_LOGS,		//9
	UPLOAD_REFUEL_LOGS,			//A
	CLEAR_LOGS,					//B
	CLEAR_TRUCK_INFO,			//C
	REPEAT_SEND_LOG,			//D
	UPDATE_DATE_TIME,			//E
	READ_CAL_DATA_LS,			//F
	EDIT_CAL_DATA_LS,			//10
	EDIT_CAL_DATA_FM,			//11		//Anand 08.10.2014
	READ_CAL_DATA_FM,			//12
	MASTER_E2P_RESET,			//13		//Anand 13.10.2014
	READ_PRINTER_CFG,			//14
	EDIT_PRINTER_CFG,			//15
	READ_WIFI_CFG,				//16
	EDIT_WIFI_CFG,				//17
	LOAD_FACTORY_DEFAULTS,		//18		//HJ 10.05.2015
	GET_SYS_INFO,				//19		//KP 20-5-15
	READ_LAST_TRANS_ID,			//1A		//Anand 04-07-15
	EDIT_LAST_TRANS_ID,			//1B		//Anand 04-07-15
	READ_RETAIL_CFG,			//1C		//HJ 24-10-15
	EDIT_RETAIL_CFG,			//1D		//HJ 24-10-15
	CHG_BAUD_RATE,				//1E
	RESET_TANK_LEVEL,			//1F
	READ_GPRS_CFG,				//20
	EDIT_GPRS_CFG,				//21
	GET_PRODUCT_SER_NUM,		//22
	SET_PRODUCT_SER_NUM,		//23
	//CHG_MAX_DISPENSE_LIMLT,	//9		//Anand 21.05.2014
	//CHG_PSWD,					//C		//Anand
	//CHANGE_DATE,				//E		//Anand
	//CHANGE_TIME				//F		//Anand
}cmd_t;*/
typedef enum
{
	READ_RF_ID,
	WRITE_RF_ID
}RF_ID_cmd_t;
typedef enum
{
	ADDR_HI		= 5,
	ADDR_LO,
	ADDR_NB_HI,	
	ADDR_NB_LO,
	WRITE_MB_DATA	
}mb_cmd_addr_t;

typedef enum
{
	START_INDEX = 0,
	SERIAL_DATA_LEN,
	CMD_INDEX,
	S_ID,
	FUN_CODE,
	DATA_LENGTH,
	DATA_INDEX	
}serial_mbrtu_response_index_t;

typedef enum
{
	UI_IDLE,
	UI_MB_RSP_WAIT
}UI_handler_state_t;
typedef enum
{
	CAL_FLOW,
	CAL_PRESSURE,
	CAL_LVL	
}calibration_parameter_type;

//void uart_packet_resp(unsigned char resp,int uart_no);			//send ack/nack response
void UWriteInt(unsigned long, char);								//write a integer on uart
void UWriteData(char data,int no);							//write a char on uart
char UWriteString(const char *str,int no);						//write a string on uart
char UWriteStringtoCSV(char *str, int no);		//String To CSV conversion
//void uartsendresponse(char *ptr,int size,int uartno);		//write packet with data to buffer
void flushTxBuffer(int no);								//name suggest
void flushRxBuffer(int no);								//name suggest
void Usendbuffer(int no);									//write buffer to uart
void decodeMsgUart0(void);								//decode message from uart
void preparePacket(int, char*, char);						//
void prepareResponsePacket(int, cmd_t, char*, unsigned char);		//Anand 21.05.2014
void prepareResponsePacket(int, char, char*, unsigned char, int, int);
void response(cmd_t);										//Anand 21.05.2014
void completePendingCommand();								//Anand 21.05.2014
void clear_logs(void);										//HJ 10.05.2015
void preparePacket(int uart_no, char data, char len);		// HJ 30-8-2017
void reRoueMBResponsePacket(void);
void UWriteBytes(char *, int, int);
//void Usendpacket(char *,int ,int );
void mb_rtu_decode_Packet(void);
int validateRtuMbReq(uint8_t);
void prepareMbRtuPacket(uint8_t,uint8_t,uint16_t*);
void prepareMbRtuException(uint8_t,int);

#endif /* APP_UART_H_ */
