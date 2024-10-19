#ifndef _COMMON_H
#define _COMMON_H

#include "config.h"
#include "ModbusRtu.h"
#define ON (1)
#define OFF (0)

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define LEN_INDX	1
#define CMD_INDX	2
#define DATA_INDX	3

#define UART0 (0)
#define UART1 (1)		//Anand 28-4-15				//On By HJS 06-05-2015
#define UART2 (2)
#define UART3 (3)		//Anand 28-4-15				//On By HJS 06-05-2015

#ifdef PRINTER_ENABLE
#define REMAINING_PC_DATA_LEN	(3)
#endif
//#define UART2 (2)
//#define UART3 (3)

#define MAX_UART_NUM (2)	//Dont change this. Keep this always 4. Irrespective of number of uart used
#define MINUTE		(60)
/*
enum
{
UART0,
UART1,
//Uncomment only if u need it otherwise unnecessary tx and rx buffer will be created

//UART2,
//UART3,

MAX_UART_NUM,	
};*/

typedef union
{
	float float_value;
	unsigned long long_value;
}IEEE_converter_t;

#define WEB_COMMS_SCHEDULAR_TIME (100)


#ifdef GPRS_ENABLE
#define GPRS_UART			(UART0)
#define GPRS_BAUD_RATE		(38400UL)
#define BAUD_RATE_UART0		(GPRS_BAUD_RATE)
#endif

#define UART_485			(UART1)
#define BAUD_RATE_485		(4800UL)//(19200UL)
#define BAUD_RATE_UART1		(BAUD_RATE_485)

#define UART_PC				(UART2)
#define BAUD_RATE_PC		/*(230400UL)*/	38400UL
#define BAUD_RATE_UART2		(BAUD_RATE_PC)

#define UNUSED_UART			(UART3)
#define UNUSED_BAUD_RATE	(9600UL)
#define BAUD_RATE_UART3		(UNUSED_BAUD_RATE)

#define GPRS_PC_RXC (USART2_RX_vect)
#define GPRS_PC_TXC (USART2_TX_vect)

#define delay(ms)	_delay_ms(ms)

#define MEGA (1)
#define delay(ms)	_delay_ms(ms)
#define OUTPUT 1
#define INPUT 0
#define HIGH				1
#define LOW					0

#define UNSINGED_INT	(65535)

//#define	RTU_MB_ADDR		(0)
#define	DEFAULT_RTU_SLV_ADDR (200)

//#define MODBUS_SLAVE1_EN		
#define MODBUS_SLAVE2_EN		
//#define MODBUS_SLAVE3_EN		

#define SAVE_PACKET_DB		(20)

enum
{
	#ifdef MODBUS_SLAVE1_EN
	DEV_PLC_MB_ADDR	 = 2,
	#endif

	#ifdef MODBUS_SLAVE2_EN
	DEV_EM_MB_ADDR	=	1,
	#endif

	#ifdef MODBUS_SLAVE3_EN
	DEV_x2_MB_ADDR,
	#endif
	
	MAX_NUM_MB_DEVICES
};

typedef unsigned char byte;
//typedef unsigned char byte;
//#define byte unsigned char
/*
#define TRUE 1
#define FALSE 0
#define HIGH 0
*/

/*#define NULL 0*/
typedef char* String;
#define _delay(ms) {int _t = ms; while(_t-- > 0) delay(1);}
#define LCD_DISP_REC_DEBUGOFF 0
typedef __SIZE_TYPE__ size_t;
#define DEC (10)
//Anand
//Uncomment if you need All Pin Support
/*
#define GSM_PORT PORTD
#define GSM_PIN PIND
#define pinMode(pin, mode) (mode)? (GSM_PIN |= (1<<pin)) : (GSM_PIN &= (~(1<<pin)))
#define digitalWrite(pin, mode) (mode)? (GSM_PORT |= (1<<pin)) : (GSM_PORT &= (~(1<<pin)))*/
//#define F_CPU 80000000UL

/*unsigned*/ char getChecksum(unsigned char*, char);
int atoi_(char*, char);		//Anand 31.03.2014
void ftoa_(float, char*);		//Anand 31.03.2014
void ftoa_1(float, char*);		//Anand 15.05.2014
void ftoa_1_7Seg(float val, char result[]);
/*float atof_(char s[], char, char);		//Anand 02.05.2014*/
void removeUnderScores(char *str);		//Anand 11.11.2014
char getHighBitIndex(char);		//Anand 15.05.2014
char strstr_(char s1[], char s2[]);		//Anand 19.06.2014
char isalnum_(char);		//Anand 19.06.2014
void ftoa_gps(float val, char result[]);
void convertToBigEndien(unsigned char *, int);
void convertToMbFormat(unsigned char*, int);
/*
float convertToBigEndien(float);
unsigned long convertToBigEndien(unsigned long);*/
float my_atof(char *);
unsigned char isStringDigit(char *);
unsigned char hex2int(char *str);
float convert_hex_to_float(unsigned long val);
unsigned long convert_float_to_hex(float val);
unsigned long my_atoul(char *str, int len, int dp);

#endif
