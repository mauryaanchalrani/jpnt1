#include <avr/io.h>
//#include <avr/iom128.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"
#ifdef _20x4_LCD
#include "lcd.h"
#endif
#ifdef _320x240_GLCD
#include "sed1335.h"
#endif
#include "always.h"
#include "app_uart.h"
#include "common.h"
#include "error.h"
#include "main.h"		//Anand 29.09.2014
//#include "HWSerial.h"		//Anand 29.09.2014
#ifdef GPRS_ENABLE
#include "gprs.h"
#endif
#include "sms.h"
#include "RS_485_device.h"
#include "_debug.h"
#include "pins.h"
//#include "tcp.h"
#include <util/delay.h>
#include <stdlib.h>
//Lets create buffer for UART0 to UART2 
//UART3 (GPRS) is handled separetely in corresponding file
volatile Rx_Buff_t Rx_Buff[3];
volatile Tx_Buff_t Tx_Buff[3];
//static MB_TCP_Payload[MB_TCP_BUFF];
unsigned char FRAME_TIMEOUT[4] = {DEFAULT_FRAME_TIMEOUT, RS485_FRAME_TIMEOUT,DEFAULT_FRAME_TIMEOUT, GPRS_FRAME_TIMEOUT};
	
#ifdef GPRS_ENABLE
extern volatile char gprs_uart_init;
extern volatile gprs_tx_data_buff_t gprs_tx_buff;
extern volatile gprs_rx_data_buff_t gprs_rx_buff;
//extern volatile rx_isr_handler_t rx_isr_handler;		//Anand 07-04-16
extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;
#endif
extern gprs_rx_data_buff_t uart_to_gprs_temp_buff;
//extern tcp_bkp_buff_t tcp_bkp_buff;
//hw_cfg_t hw_cfg;
//extern volatile gprs_temp_rx_buff_t gprs_temp_rx_buff;

void initUart(int uart_num) //, long baud_rate)
 {
	//unsigned int val = ((F_CPU/16/hw_cfg.sr_port[uart_num].baudRate) - 1);
#ifdef GPRS_UART	
	if(uart_num == UART0)
	{
		UCSR0B = (_BV(RXCIE0) | _BV(RXEN0) | /*_BV(TXCIE0) | */_BV(TXEN0));		//Anand 28-4-15
		UCSR0C =  _BV(UCSZ01) | _BV(UCSZ00);
		UBRR0H = (unsigned char)(UBAUD_VALUE_UART0 >> 8);
		UBRR0L = (unsigned char) UBAUD_VALUE_UART0;
	}
#endif	

#ifdef UART_485
	else if(uart_num == UART_485)
	{
		/*UCSR1B = (_BV(RXCIE1) | _BV(RXEN1) | _BV(TXEN1));*/		
		UCSR1B = (_BV(RXCIE1) | _BV(RXEN1) /*| _BV(TXCIE1)*/ | _BV(TXEN1));
		UCSR1C =  _BV(UCSZ11) | _BV(UCSZ10);
		UBRR1H = (unsigned char)(UBAUD_VALUE_UART1 >> 8); //HJ 9-5-15
		UBRR1L = (unsigned char) UBAUD_VALUE_UART1;
		
	}
#endif

#ifdef UART_PC
else if(uart_num == UART2)
{
	/*UCSR2B = (_BV(RXCIE2) | _BV(RXEN2) | _BV(TXEN2));*/	
	UCSR2B = (_BV(RXCIE2) | _BV(RXEN2) /*| _BV(TXCIE2)*/ | _BV(TXEN2));
	UCSR2C =  _BV(UCSZ21) | _BV(UCSZ20);
	UBRR2H = (unsigned char)(UBAUD_VALUE_UART2 >> 8); //HJ 9-5-15
	UBRR2L = (unsigned char) UBAUD_VALUE_UART2;
}
#endif

#ifdef UART3
	else if(uart_num == UART3)
	{
		UCSR3B = (_BV(RXCIE3) | _BV(RXEN3)  /*| _BV(TXCIE3)*/ | _BV(TXEN3));		
		UCSR3C =  _BV(UCSZ31) | _BV(UCSZ30);
		UBRR3H = (unsigned char)(UBAUD_VALUE_UART3 >> 8);
		UBRR3L = (unsigned char) UBAUD_VALUE_UART3;
	}
#endif
		

#ifdef GPRS_UART
	flushRxBuffer(UART0);
	flushTxBuffer(UART0);
#endif
#ifdef UART_485
	flushRxBuffer(UART_485);
	flushTxBuffer(UART_485);
#endif

#ifdef UART_PC
flushRxBuffer(UART2);
flushTxBuffer(UART2);
#endif

#ifdef UART3
flushRxBuffer(UART3);
flushTxBuffer(UART3);
#endif
}

void communication(int uart_no, char start)		//Anand 7-9-15
{
	if(start)
	{
#ifdef GPRS_ENABLE
		if((uart_no == GPRS_UART))		//Change following if you change the UART number
		{
			if(!gprs_uart_init)
			{
				gprs_uart_init = TRUE;
				UCSR0C =  _BV(UCSZ01) | _BV(UCSZ00);
				UBRR0H = (unsigned char)(UBAUD_VALUE_UART0 >> 8);
				UBRR0L = (unsigned char) UBAUD_VALUE_UART0;
				UCSR0B |= (/*(_BV(TXCIE3) |*/ _BV(TXEN0));
			}
			UCSR0B |= (_BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0) /*| _BV(TXCIE3)*/);		//Anand 09-04-16
		}
#endif
	}	
	else
	{
#ifdef GPRS_ENABLE
		if((uart_no == GPRS_UART))		//Change following if you change the UART number
		{
			UCSR0B &= ~(_BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) | _BV(TXCIE0));
		}
#endif
	}
}
void enable_tx_intr(int uart_no)
{
	if(uart_no == UART0)
	{
		UCSR0B |= (1 << TXCIE0);
	}
	else if(uart_no == UART1)
	{
		UCSR1B |= (1 << TXCIE1);
	}
	else if(uart_no == UART2)
	{
		UCSR2B |= (1 << TXCIE2);
	}
	else if(uart_no == UART3)
	{
		UCSR3B |= (1 << TXCIE3);
	}
}
void disable_tx_intr(int uart_no)
{
	if(uart_no == UART0)
	{
		UCSR0B &= ~((1 << TXCIE0));
	}
	else if(uart_no == UART1)
	{
		UCSR1B &= ~((1 << TXCIE1));
	}
	else if(uart_no == UART2)
	{
		UCSR2B &= ~((1 << TXCIE2));
	}
	else if(uart_no == UART3)
	{
		UCSR3B &= ~((1 << TXCIE3));
	}
}
void enable_rx_intr(int uart_no)
{
	if(uart_no == UART0)
	{
		UCSR0B |= (1 << RXCIE0);
	}
	else if(uart_no == UART1)
	{
		UCSR1B |= (1 << RXCIE1);
	}
	else if(uart_no == UART2)
	{
		UCSR2B |= (1 << RXCIE2);
	}
	else if(uart_no == UART3)
	{
		UCSR3B |= (1 << RXCIE3);
	}
}
void disable_rx_intr(int uart_no)
{
	if(uart_no == UART0)
	{
		UCSR0B &= ~((1 << RXCIE0));
	}
	else if(uart_no == UART1)
	{
		UCSR1B &= ~((1 << RXCIE1));
	}
	else if(uart_no == UART2)
	{
		UCSR2B &= ~((1 << RXCIE2));
	}
	else if(uart_no == UART3)
	{
		UCSR3B &= ~((1 << RXCIE3));
	}
}
#ifdef GPRS_ENABLE

ISR(GPRS_USART_RXC)
{
		cli();
		
		char tmpudr;
		//sei();
		tmpudr = GPRS_UDR;
		gprs_rx_isr_handler.state = GPRS_RX_INPROG;
		
		//UDR2 = tmpudr;
		
		gprs_rx_isr_handler.elapsed = 0;		//Anand 07-04-16
		
		/*if(get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)
		{
			if (gprs_temp_rx_buff.index < GPRS_TEMP_RX_BUFFER_MAX)
			{
				gprs_temp_rx_buff.buff[gprs_temp_rx_buff.index++] = tmpudr;
			}
		}
		else*/
		{
			if (gprs_rx_buff.index < GPRS_RX_READ_BUFFER_MAX)
			{
				gprs_rx_buff.buffer[gprs_rx_buff.index++] = tmpudr;
				//UDR0 = tmpudr; // VC : 31/07/2018 checking for total data is coming properly or not
				//tcp_bkp_buff.buff[tcp_bkp_buff.length++] = tmpudr;
			}
		}
		
		sei();
		/*
		#ifdef DEBUG_SPCL
		UWriteData('E',UART_PC);
		#endif*/	
}

ISR (GPRS_USART_TXC)//
{
//NOTE:- Lets give highest priority to pulser interrupt and keep interrupts on this ISR.
//sei();
//cli();
gprs_tx_buff.index++;
if ((gprs_tx_buff.index >= GPRS_TX_BUFFER_MAX) || 	(gprs_tx_buff.buffer[gprs_tx_buff.index] == '\0'))
{
	unlock(gprs_tx_buff.locked);
	disable_tx_intr(GPRS_UART);	//HJ 12-08-2016
}
else
{
	if (GPRS_UCSRA & (1 << GPRS_UDRE))
	{
		GPRS_UDR = gprs_tx_buff.buffer[gprs_tx_buff.index];
		//UCSR0A &= ~(_BV(TXC0));
	}
}

//sei();
/*
#ifdef DEBUG_SPCL
UWriteData('F',UART_PC);
#endif	*/
}
#endif

ISR(RS485_UART_RX)
{
	cli();
	//sei();
	char udata=0;
	//static unsigned char len=0;
	udata = UDR1;
	Rx_Buff[UART_485].elapsed = 0;
	if(Rx_Buff[UART_485].rx_indx < RX_BUFFER_MAX)
	{
		Rx_Buff[UART_485].rx_buffer[Rx_Buff[UART_485].rx_indx++] = udata;
	}
	
/*
#ifdef DEBUG_FM
	UWriteData(Rx_Buff[UART_485].rx_indx,UART_PC);
	//UWriteData(udata,UART_PC);
#endif*/
	//UWriteData(udata,UART_485);
#if 0
/*
#ifdef DEBUG_RS485
	//UWriteData(udata,UART_PC);
	UDR2 = udata;
#endif*/
	
	//Modbus protocol
	switch(Rx_Buff[UART_485].rx_state)
	{
		case START:

		if((udata == DEV_PLC_MB_ADDR)  
#ifdef MODBUS_SLAVE2_EN  			
			|| (udata == DEV_EM_MB_ADDR) 
#endif
#ifdef MODBUS_SLAVE3_EN
			|| (udata == DEV_PLC)
#endif
			)
			{

			Rx_Buff[UART_485].rx_buffer[Rx_Buff[UART_485].rx_indx++]= udata;
			Rx_Buff[UART_485].rx_state = FUNC_CODE;
			
		}
		break;
		case FUNC_CODE:
	
			Rx_Buff[UART_485].rx_buffer[Rx_Buff[UART_485].rx_indx++]= udata;
			/*
			if(udata >= MB_FC_WRITE_COIL)			
			{
				Rx_Buff[UART_485].rx_state = DATA_READ;
				//len = 0;
			}
			else
			{
				Rx_Buff[UART_485].rx_state = READ_LEN;
			}*/
			
			Rx_Buff[UART_485].rx_state = READ_LEN;
			
		break;
		case READ_LEN :
	
			Rx_Buff[UART_485].rx_buffer[Rx_Buff[UART_485].rx_indx++]= udata;
			//len = udata;
			Rx_Buff[UART_485].rx_state = DATA_READ;
			break;
		case DATA_READ:

			if(Rx_Buff[UART_485].rx_indx < RX_BUFFER_MAX)
				Rx_Buff[UART_485].rx_buffer[Rx_Buff[UART_485].rx_indx++]= udata;
			
			/*if(Rx_Buff[UART_485].rx_indx >= ((len)/ * + 5* / ))					//
			{
				Rx_Buff[UART_485].rx_state = READY;
			}*/
			break;
			
			//Lets remove warning of unused cases
		case EOP:
		case PROCESS:
		case READY:

		default:
			break;
	}
#endif
	sei();
	
	//Rx_Buff[UART_485].elapsed =  0;
/*
#ifdef DEBUG_SPCL
UWriteData('C',UART_PC);
#endif	*/
}
ISR (RS485_UART_TX)
{
	cli();
	sei();
/*
#ifdef DEBUG_SPCL
UWriteData('D',UART_PC);
#endif*/	
}

ISR(PC_UART_RX)
{

cli();
static int msglen=0,datacnt=0 ;
//static ISR_rx_state_t isr_rx_state = START;

char rsp=0;
char udata=0;
udata = UDR2;
//UWriteData(udata,UART_PC);
//MODBUS_PORT |= (1 << MOD_TXEN_PIN);
//_delay_us(10);
UWriteData(udata,UART_485);
//_delay_us(100);
//MODBUS_PORT &= ~(1 << MOD_TXEN_PIN);
//#if 0
switch(Rx_Buff[UART_PC].rx_state)
{
	case START :
	{
		if(udata == 0xAA)		//if(udata =='#')		//Anand
		{
			//if (Rx_Buff[UART_PC].rx_state == READY)		//R
			{
				flushRxBuffer(UART_PC);
				Rx_Buff[UART_PC].rx_buffer[Rx_Buff[UART_PC].rx_indx++]= udata;
				//isr_rx_state = READ_LEN;
				Rx_Buff[UART_PC].rx_state = READ_LEN;
				datacnt = 0;
			}
		}
	}break;

	case READ_LEN :
	{
		msglen = udata;
		Rx_Buff[UART_PC].rx_buffer[Rx_Buff[UART_PC].rx_indx++]= udata;
		//datacnt++;		//R
		//isr_rx_state = DATA_READ;
		Rx_Buff[UART_PC].rx_state = DATA_READ;
	}break;
	
	case DATA_READ :
	{
		Rx_Buff[UART_PC].rx_buffer[Rx_Buff[UART_PC].rx_indx++]= udata;
		datacnt++;
		
		if (datacnt>=RX_BUFFER_MAX)
		{
			flushRxBuffer(UART_PC);
			//	isr_rx_state = START;
			Rx_Buff[UART_PC].rx_state = START;
		}

		if(datacnt>msglen)		//R changed >= -> >
		{
			//isr_rx_state = EOP;
			Rx_Buff[UART_PC].rx_state = EOP;
		}
		
	}break;

	case EOP :
	{
		if(udata == 0x55)		//if(udata=='$')		//Anand
		{
			Rx_Buff[UART_PC].rx_buffer[Rx_Buff[UART_PC].rx_indx++]= udata;		//R		//Anand 17-4-15
			
			//if(getChecksum(&Rx_Buff[UART_PC].rx_buffer[LEN_INDX+1], msglen)==Rx_Buff[UART_PC].rx_buffer[msglen])//Rx0_Buff.rx0_buffer[msglen])Rx_Buff[UART_PC].rx_buffer[msglen]
			if(getChecksum((unsigned char*)&Rx_Buff[UART_PC].rx_buffer[LEN_INDX+1], msglen) == Rx_Buff[UART_PC].rx_buffer[msglen + LEN_INDX + 1])		//R
			{
				rsp = ACK;
				//isr_rx_state = START;
				//isr_rx_state = READY;		//Anand
				//Rx_Buff[UART_PC].rx_state = RX_MSG_READY;
				Rx_Buff[UART_PC].rx_state = READY;
			}
			else
			{
				rsp = NACK;
				//flushRxBuffer(UART_PC);		//Anand 22.05.2014
				//Rx_Buff[UART_PC].rx_state = RX_READY;
				Rx_Buff[UART_PC].rx_state = START;
				//isr_rx_state = START;
			}
		}
		else
		{
			//isr_rx_state = START;
			//Rx_Buff[UART_PC].rx_state = RX_READY;
			Rx_Buff[UART_PC].rx_state = START;
			//flushRxBuffer(UART_PC);
			rsp=NACK;
		}
		if(rsp)	//will be true for ACK/NACK
		{
			//uart_packet_resp(rsp,UART_PC);
			if((!Tx_Buff[UART_PC].wait_for_ack) || (rsp==NACK))
			prepareResponsePacket(UART_PC, (cmd_t)Rx_Buff[UART_PC].rx_buffer[CMD_INDX], &rsp, 2);		//Anand 21.05.2014
			
			if(rsp == NACK)
			{
				flushRxBuffer(UART_PC);		//Anand 22.04.2014
			}
			
			//Let the main program know
			Tx_Buff[UART_PC].tx_ready = TRUE;
			
			Usendbuffer(UART_PC);		//Send data			//TT: 19 Jul 18 Lets ack from here only instead of main
			flushTxBuffer(UART_PC);
		}
	}
	break;
	
	default:
	break;
	}
//#endif
	sei();
}


ISR (PC_UART_TX)
{
	cli();
	sei();
}




ISR(USART3_RX_vect)//
{
	cli();
	sei();

}


#if 0
ISR(GPRS_USART_RXC)
{
	char tmpudr;
	//NOTE:- Lets give highest priority to pulser interrupt and keep interrupts on this ISR.
	//sei();
	tmpudr = GPRS_UDR;
	//UDR0 = tmpudr;
	cli();
	
	int buff_id = 0xFF;
	
	for (int i=0;i<NUM_OF_GPRS_RX_BUFF;i++)
	{
		if(!(gprs_rx_buff[i].locked))
		{
			buff_id = i;
			UDR0 = '0'+i;
			break;
		}		
	}

	if (buff_id != 0xFF)
	{
		switch (gprs_rx_isr_handler.state)
		{
			case (GPRS_RX_IDLE):
			if (tmpudr == '\r') 
			{
				//gprs_rx_buff[buff_id].buffer[gprs_rx_buff[buff_id].index++] = tmpudr;
				gprs_rx_isr_handler.state = GPRS_RX_START_GET_LF;		//Anand 07-04-16
			}
			gprs_rx_isr_handler.elapsed = 0;		//Anand 07-04-16
			break;
			case (GPRS_RX_START_GET_LF):
			if (tmpudr == '\n')
			{
				 gprs_rx_isr_handler.state = GPRS_RX_MSG;		//Anand 07-04-16
				 //gprs_rx_buff[buff_id].buffer[gprs_rx_buff[buff_id].index++] = tmpudr;
			}
			else 
			{
				gprs_rx_isr_handler.state = GPRS_RX_IDLE;		//Anand 07-04-16
				gprs_rx_buff[buff_id].index = 0;
			}			
			gprs_rx_isr_handler.elapsed = 0;		//Anand 07-04-16
			break;
			case (GPRS_RX_MSG):
			//if (rx_isr_handler.elapsed < RX_CHAR_TIMEOUT)
			//UDR0 = tmpudr;
			{
				if (gprs_rx_buff[buff_id].index < GPRS_RX_BUFFER_MAX)
				{
					gprs_rx_buff[buff_id].buffer[gprs_rx_buff[buff_id].index++] = tmpudr;
					//UDR0 = gprs_rx_buff[buff_id].index + '0';
					//UDR0 = tmpudr;
				}
				/*else                   //Check wheather the buffer is overflow
				{
					UWriteData(tmpudr, UART_PC);
				}*/
				
				if (tmpudr == '\n')
				{
					//gprs_rx_isr_handler.state = GPRS_RX_IDLE;		//Anand 07-04-16
					if(gprs_rx_buff[buff_id].buffer[gprs_rx_buff[buff_id].index - 2] == '\r')
					{
						if(gprs_rx_buff[buff_id].index <= 2)
						{
							gprs_rx_buff[buff_id].index = 0;						
						}
						else
						{
							lock(gprs_rx_buff[buff_id].locked);				
							//Lets go to next buff with index 0
							buff_id++;
							buff_id %= NUM_OF_GPRS_RX_BUFF;						
					//		gprs_rx_buff[buff_id].buffer[gprs_rx_buff[buff_id].index]= 0;
							if (!(gprs_rx_buff[buff_id].locked))
							{
								gprs_rx_buff[buff_id].index = 0;						
							}
							
							//??? risk of overwrting before decode
						}
					}
				}
			}
			/*else
			{
				if (tmpudr == '\r') rx_isr_handler.state = RX_START_GET_LF;
				else rx_isr_handler.state = RX_IDLE;
			}*/
			gprs_rx_isr_handler.elapsed = 0;		//Anand 07-04-16
			break;
			default:
				gprs_rx_buff[buff_id].index = 0;
				gprs_rx_isr_handler.state = GPRS_RX_IDLE;		//Anand 07-04-16
			break;
		}
	}
	sei();
#ifdef GPRS_DEBUG		//Anand 5-4-16
	if(buff_id == 0xff)
	{
		/*UWriteString("\nBoth Locked\n", UART_PC);
		UWriteData('$', UART_PC);
		for(int i=0 ; i < gprs_rx_buff[0].index ; ++i)
		{
			UWriteData(gprs_rx_buff[0].buffer[i], UART_PC);
		}
		UWriteData('$', UART_PC);
		for(int i=0 ; i < gprs_rx_buff[1].index ; ++i)
		{
			UWriteData(gprs_rx_buff[1].buffer[i], UART_PC);
		}
		UWriteData('$', UART_PC);*/
	}
	UWriteData(tmpudr, UART_PC);
	/*if(gprs_rx_buff[0].index > 0)		//ANand 6-4-16
	{
		UWriteData(gprs_rx_buff[buff_id].buffer[gprs_rx_buff[buff_id].index - 1], UART_PC);
	}*/
#endif
}
#endif


ISR(USART3_TX_vect)
{
	cli();
	sei();
}








#ifdef GPRS_ENABLE
/**
 * checks for str in rx buffer, and also gives rx buffer in copy_here pointer
 * if needed.
 * Return value:
 * MATCH_OK: if str is found in rx buffer
 * MATCH_FAIL: if str is not found
 * NO_NEW_MSG: If no new message has arrived in rx buffer since last check
 */
char check_string(const char *str, char *copy_here, int* numbytes)
{
	//char tmpstr[BUFFER_MAX];
	char retval = GPRS_NO_NEW_MSG;
	int i, /*numbytes=0,*/ j,ip_str_len;
	ip_str_len = strlen(str);
	char *lock_ptr = NULL;
	if (!copy_here) 
	{

		retval = IS_FAILURE;
		return retval;
	}
	/*if(get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)
	{
		lock_ptr = (char*)&gprs_temp_rx_buff.locked;
	}
	else*/
	{
		lock_ptr = (char*)&gprs_rx_buff.locked;
	}	
	//if (gprs_rx_isr_handler.elapsed > GPRS_FRAME_TIMEOUT) 
	//if(gprs_rx_buff.locked == LOCKED )
	if(*lock_ptr == LOCKED )
	{

/*
#ifdef GPRS_DEBUG
UWriteString("rcv:",UART_PC);
if(get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)
	UWriteString((char*)gprs_temp_rx_buff.buff,UART_PC);
else
	UWriteString((char*)gprs_rx_buff.buffer,UART_PC);
#endif*/
		//gprs_rx_buff.locked = UNLOCKED;
		*lock_ptr = UNLOCKED;
/*
#ifdef GPRS_DEBUG
UWriteString("get_rx:1",UART_PC);
#endif*/
		*numbytes = get_rx_data(copy_here);
/*
#ifdef GPRS_DEBUG
UWriteString("get_rx:2",UART_PC);
#endif*/
		//memset(uart_to_gprs_temp_buff.buffer,0,sizeof(uart_to_gprs_temp_buff.buffer));
		//uart_to_gprs_temp_buff.index = 0;
		//If we dont have anything to check it means we have to return whatever we got
		if((str[0]) != '\0')
		{			
			if(*numbytes > 0)
			{
				for (i=0; i < *numbytes; i++) 
				{
					if (!memcmp(&copy_here[i], str, ip_str_len)) break;
				}			

				if(i >= *numbytes) 
				{
					return retval = GPRS_MATCH_FAIL;
				}		
				retval = GPRS_MATCH_OK;
				for(j = 0; j < (*numbytes - i - ip_str_len); j++)
				{
					copy_here[j] = copy_here[j + i + ip_str_len];
				}
				*numbytes = j;
				copy_here[j] = '\0';
			
			}
		}
		else
		{
			retval = GPRS_MATCH_OK;
		}
	}

	return (retval);
}

char check_string_nobuf(const char *str)
{

	int len = 0;
	char tmpstr[GPRS_RX_BUFFER_MAX];
	
	return check_string(str, tmpstr, &len);
}

char skip_to(char *str, char *copy_here)
{
	unsigned char i, numbytes=0, j, length, retval = GSM_NO_NEW_MSG;;
	if (!copy_here) {
		
		return retval = IS_FAILURE;
	}
	length = strlen(str);
	numbytes = strlen(copy_here);

	for (i=0; i < numbytes; i++) {
		if (!memcmp(&copy_here[i], str, strlen(str))) break;
	}

	if(i >= numbytes) {
		return retval = GSM_MATCH_FAIL;
	}

	
	retval = GSM_MATCH_OK;

	for(j = 0; j < (numbytes - i - length); j++){
		copy_here[j] = copy_here[j + i + length];
	}
	
	copy_here[j] = '\0';

	return (retval);
}
#endif
void checkforUARTFrameTimeout(int uart_no)
{
	if(uart_no == GPRS_UART)
	{
		if(gprs_rx_isr_handler.elapsed >= FRAME_TIMEOUT[uart_no])
		{
			gprs_rx_isr_handler.elapsed = 0;
			
			/*if(get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)
			{
				if((gprs_temp_rx_buff.index > 0))
				{
					gprs_temp_rx_buff.locked = LOCKED;
					gprs_rx_isr_handler.state = GPRS_RX_IDLE;
				}
			}
			else*/
			{
				if((gprs_rx_buff.index > 0))
				{
					gprs_rx_buff.locked = LOCKED;
					gprs_rx_isr_handler.state = GPRS_RX_IDLE;
				}
			}
		}
	}
	else if(uart_no == UART_485)
	{
		
		if(Rx_Buff[uart_no].elapsed >= FRAME_TIMEOUT[uart_no])
		{
			Rx_Buff[uart_no].elapsed = 0;
/*
#ifdef DEBUG_FM
UWriteData('#',UART_PC);
UWriteData(Rx_Buff[UART_485].rx_indx,UART_PC);
//UWriteData(udata,UART_PC);
#endif*/			
			if(Rx_Buff[uart_no].rx_indx > 0)
			{
				Rx_Buff[uart_no].rx_state = READY;
/*
#ifdef DEBUG_FM
UWriteString("rx_ready",UART_PC);
#endif*/
#ifdef DEBUG_RS485
				//UWriteBytes((char *)&Rx_Buff[UART_485].rx_buffer,Rx_Buff[UART_485].rx_indx,UART_PC);
#endif
				//flushRxBuffer(UART_485);
			}
			else
			{
				//UWriteString("rx_in:",UART_PC);
				//UWriteInt(Rx_Buff[UART_485].rx_indx,UART_PC);
				
			}
		}
	}
	/*else if(uart_no <=UART2)
	{	
		if(Rx_Buff[uart_no].elapsed >= FRAME_TIMEOUT[uart_no])
		{
			Rx_Buff[uart_no].elapsed = 0;
			if(Rx_Buff[uart_no].rx_indx > 0)
			{
				Rx_Buff[uart_no].rx_state = READY;
			}
		}
	}*/
}

/*
void UWriteInt(unsigned long num, char no)
{
	char temp[30];
	ltoa(num, temp, 10);
	UWriteString((char*)temp, no);
}*/