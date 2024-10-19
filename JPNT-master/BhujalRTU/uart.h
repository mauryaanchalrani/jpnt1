#ifndef __BOD_UART_H__
#define __BOD_UART_H__

#include "common.h"					//Include By HJS 06-05-2015

#define UBAUD_VALUE_UART0 ((F_CPU/16/BAUD_RATE_UART0) - 1)

#define UBAUD_VALUE_UART1 ((F_CPU/16/BAUD_RATE_UART1) - 1)

#define UBAUD_VALUE_UART2 ((F_CPU/16/BAUD_RATE_UART2) - 1)

#define UBAUD_VALUE_UART3 ((F_CPU/16/BAUD_RATE_UART3) - 1)

#define RS485_UART_RX	(USART1_RX_vect)
#define RS485_UART_TX	(USART1_TX_vect)

#define PC_UART_RX		(USART2_RX_vect)
#define PC_UART_TX		(USART2_TX_vect)

#define TX_BUFFER_MAX (64)
#define RX_BUFFER_MAX (64)
//#define MB_TCP_BUFF   (4)

#define GSM_TX_WARMUP_SECS	3
//#define GSM_WARMUP_SECS     (10 *(1000/ 200))
#define IS_SUCCESS		    1
#define IS_FAILURE			0
typedef enum {GPRS_NO_NEW_MSG, GPRS_MATCH_FAIL, GPRS_MATCH_OK} match_t;

#define DEFAULT_FRAME_TIMEOUT	(3)			//100 msec
#define GPRS_FRAME_TIMEOUT		(5)			//30 msec
#define RS485_FRAME_TIMEOUT		(5)			//2*10 msec


//~Anand 16-3-16 For GPRS
//#define RX_CHAR_TIMEOUT DEFAULT_FRAME_TIMEOUT //=>if no char comes within this*10ms, buffer is cleared

typedef enum
{
	START,
	PROCESS,
	READY,
	READ_LEN,
	DATA_READ,
	FUNC_CODE,
	WRITE_DATA,
	CMD,       // 20/11/2018 VC: for the command
	EOP
	
}ISR_rx_state_t;

typedef struct  
{
	unsigned char rx_buffer[RX_BUFFER_MAX];
	int rx_indx;
	//rx_msg_state_t rx_state;
	unsigned int elapsed;
	ISR_rx_state_t rx_state;
	           
}Rx_Buff_t;


//Note: GPRS Buffer is defined in GPRS.H file
typedef enum
{
	MBTCP_TRID_HB,
	MBTCP_TRID_LB,
	MBTCP_PROTO_TYPE_HB,	
	MBTCP_PROTO_TYPE_LB,	
}MB_TCP_payload_t;

typedef struct
{
	char tx_buffer[TX_BUFFER_MAX];
	int tx_indx;
	char tx_ready;
	char pending_command;			//Anand 21.05.2014
	unsigned long curr_pck_num;		//Anand 02.07.2014
	char wait_for_ack;				//Anand 23.05.2014
}Tx_Buff_t;


//Note: GPRS Buffer is defined in GPRS.H file

typedef struct
{
	unsigned int baudRate;
}__attribute__((packed)) sr_port_t;

void initUart(int /*,long*/ );		//Anand 29.09.2014
void communication(int , char);		//Anand 7-9-15
void enable_tx_intr(int);		//HJ12-08-2016
void disable_tx_intr(int);		//HJ12-08-2016
void enable_rx_intr(int);		//HJ12-08-2016
void disable_rx_intr(int);		//HJ12-08-2016
/*char GetChecksum(char* buff, char len);*/
char check_string(const char *str, char *copy_here, int*);
char check_string_nobuf(const char *str);
char skip_to(char *str, char *copy_here);

void checkforUARTFrameTimeout(int);

//void UWriteInt(unsigned long num, char no);
#endif


