
/*
 * app_uart.c
 *
 * Created: 11-12-2013 12:48:23
 *  Author: Mindfield
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"
#ifdef _20x4_LCD
#include "lcd.h"
#endif
#include <string.h>
#include "app_uart.h"
#include "ds1307.h"

#include <util/delay.h>
#include "main.h"
#include "language.h"
#include "app_eeprom.h"
#include "error.h"
#include "flash.h"
#include "common.h"
#include "RS_485_device.h"
#include <stdlib.h>
#include "I2C.h"			//Anand 13.10.2014
#ifdef _20x4_LCD
#include "lcd.h"
#endif
#include "sys_info.h"
#ifdef GPRS_ENABLE		//Anand 06-04-16
#include "gprs.h"
#endif
#include "adc.h"
#include <string.h>
#include "config.h"

#include "main.h"
#include "sms.h"
#include "sensor.h"
#include "modbus_addr.h"
#include "graphic_lcd_ks0108.h"
#include "app_lcd.h"
#include "ModbusRtu.h"
#include <avr/wdt.h>
extern volatile Rx_Buff_t Rx_Buff[3];
extern volatile Tx_Buff_t Tx_Buff[3];
extern freq_updated_data_t tm_freq_updated_data;
extern time_main_t time_main;		//Anand 30.06.2014

#ifdef GPRS_ENABLE
extern volatile char gprs_uart_init;
extern volatile gprs_tx_data_buff_t gprs_tx_buff;
extern volatile gprs_rx_data_buff_t gprs_rx_buff;
extern gprs_config_data_t gprs_config_data;

#endif
extern RS485_device RS485_device_handler;

extern FL_rtu_log_t FL_lvl_log;
extern Modbus master;
#ifdef GPRS_ENABLE

extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;		//Anand 07-04-16
#endif

sys_info_t sys_info;	//KP 20-5-2015
product_info_t product_info;		//HJ 0-08-2016
#ifdef ONLINE_CONFIG_EN
product_info_config_t product_info_config;
#endif

extern em_data_t em_data;
extern calbration_data_t calbration_data;
extern freq_updated_data_t freq_updated_data; // 23/11/2018 for clear FL logs
extern FL_rtu_log_t FL_rtu_log;

extern ram_data_t ram_data;//abdul majeed

extern unsigned long int yest_total;


extern e2p_hex_file_hdr_t e2p_hex_file_hdr;

void UWriteInt(unsigned long num, char no)
{
	char temp[30];
	ltoa(num, temp, 10);
	UWriteString((char*)temp, no);
}

void UWriteData(char data, int no)		//Anand 28-4-15
{
	switch(no)
	{
#ifdef UART0	
		case UART0 :
			while(!(UCSR0A & (1<<UDRE0)));
			UDR0=data;
		break;
#endif
#ifdef UART1	
		case UART1 :
			while(!(UCSR1A & (1<<UDRE1)));
			UDR1=data;
		break;
#endif

#ifdef UART2
		case UART2 :
		while(!(UCSR2A & (1<<UDRE2)));
		UDR2=data;
		break;

#endif
#ifdef UART3
		case UART3 :
		while(!(UCSR3A & (1<<UDRE3)));
		UDR3=data;
		break;

#endif

	}
}

char UWriteString(const char *str, int no)		//07-04-16 Anand char for GPRS
{
	char retVal = 0;
#ifdef GPRS_ENABLE
#ifdef	GPRS_DEBUG
	
#endif
	if(no == GPRS_UART)
	{
		
		 if (str[0] == '\0') retVal = IS_SUCCESS;
		 else
		 {
			 if (1)//this is unlocked when last byte is sent out in TX complete ISR
			 {
#ifdef	GPRS_DEBUG
				
#endif
				 if (GPRS_UCSRA & (1 << GPRS_UDRE)) {
#ifdef	GPRS_DEBUG
					
#endif
					 
					 if(strlen(str) < GPRS_TX_BUFFER_MAX)
					 {
						 strcpy((char*)gprs_tx_buff.buffer, (const char*)str);
					 }
					 else
					 {
						 strcpy((char*)gprs_tx_buff.buffer, (const char*)"BUFFER SIZE ERROR");
					 }
					 enable_tx_intr(GPRS_UART);
					 GPRS_UDR = gprs_tx_buff.buffer[0];
					 gprs_tx_buff.index = 0;
					 retVal = IS_SUCCESS;
				 }
			 }
			 else
			 {
#ifdef	GPRS_DEBUG
				
#endif
				 retVal = IS_FAILURE;
			 }
		 }
	}
	else
#endif
	{
		while((*str)!='\0') //here is the problem for null
		{
			UWriteData(*str,no);
			str++;
		}
		UWriteData('\0',no);
	}
	
	return retVal;		//Anand 07-04-16
}

char UWriteStringtoCSV(char *str, int no)		//07-04-16 Anand char for GPRS
{
	char retVal = 0;
	#ifdef GPRS_ENABLE
	#ifdef	GPRS_DEBUG
	
	#endif
	if(no == GPRS_UART)
	{
		
		if (str[0] == '\0') retVal = IS_SUCCESS;
		else
		{
			if (1)//this is unlocked when last byte is sent out in TX complete ISR
			{
				#ifdef	GPRS_DEBUG
				
				#endif
				if (GPRS_UCSRA & (1 << GPRS_UDRE)) {
					#ifdef	GPRS_DEBUG
					
					#endif
					
					if(strlen(str) < GPRS_TX_BUFFER_MAX)
					{
						strcpy((char*)gprs_tx_buff.buffer, (const char*)str);
					}
					else
					{
						strcpy((char*)gprs_tx_buff.buffer, (const char*)"BUFFER SIZE ERROR");
					}
					enable_tx_intr(GPRS_UART);
					GPRS_UDR = gprs_tx_buff.buffer[0];
					gprs_tx_buff.index = 0;
					retVal = IS_SUCCESS;
				}
			}
			else
			{
				#ifdef	GPRS_DEBUG
				
				#endif
				retVal = IS_FAILURE;
			}
		}
	}
	else
	#endif
	{
		while((*str)!='\0')
		{
			UWriteData(*str,no);
			str++;
		}
		UWriteData(',',no);
	}
	
	return retVal;		//Anand 07-04-16
}

////////////
void UWriteBytes(char *str, int len, int no)		//07-04-16 Anand char for GPRS
{	
	while(len)
	{
		UWriteData(*str, no);
		str++;
		len--;
	}
		
}
///////////
void Usendbuffer(int no)
{
#ifdef GPRS_ENABLE
	if(no == GPRS_UART)
	{
		for (int i=0;gprs_tx_buff.buffer[i] != '\0';i++)
		{
			UWriteData(gprs_tx_buff.buffer[i],no);
		}
	}
	else
#endif
	{
		for (int i=0;i<Tx_Buff[no].tx_indx;i++)
		{
			UWriteData(Tx_Buff[no].tx_buffer[i],no);
		}
	}
	
}


void flushTxBuffer(int no)
{
	if(no <= UART2)
	{
		memset((void*)&Tx_Buff[no].tx_buffer[0], 0, sizeof(Tx_Buff[no].tx_buffer));	
		if(no == UART_PC) // 27/11/2018 VC: need to thing about the data correption
		{
			Tx_Buff[no].tx_indx = 0;
			Tx_Buff[no].tx_ready = 0;
		}
	}
	
#ifdef GPRS_ENABLE		//Anand 06-04-16
	if(no == GPRS_UART)
	{
		memset((void*)&gprs_tx_buff, 0, sizeof(gprs_tx_data_buff_t));
	}
#endif
}
void flushRxBuffer(int no)
{
	
	if(no <= UART2)
	{
		memset((void*)&Rx_Buff[no], 0, (sizeof(Rx_Buff[no])));
	}
#ifdef GPRS_ENABLE		//Anand 06-04-16
	if(no == GPRS_UART)
	{
		memset((void*)&gprs_rx_isr_handler, 0, (sizeof(gprs_rx_isr_handler_t)));
		memset((void*)&gprs_rx_buff, 0, (sizeof(gprs_rx_data_buff_t)));
		
	}	
#endif
}


void decodeMsgUart0(void) // need to be change the name 
{
	static UI_handler_state_t UI_handler_state = UI_IDLE; 
	static cmd_t cmd;
	
	static unsigned int timeout = 0;	
	unsigned char slave_id;
	
	
	switch (UI_handler_state)
	{
		case UI_IDLE:
			if(Rx_Buff[UART_PC].rx_state == READY)
			{
				cmd = (cmd_t)Rx_Buff[UART_PC].rx_buffer[CMD_INDX];
				
				if(cmd == CMD_MB_QUERY)
				{
					slave_id = Rx_Buff[UART_PC].rx_buffer[DATA_INDX];
					 

					//check the destination
					if(slave_id == product_info.industry_id)
					{
						 
					}
					else 	// command for slave device 
					{ 
						if((slave_id < DEFAULT_RTU_SLV_ADDR) && (slave_id < MAX_NUM_MB_DEVICES))
						{	
						
							if(Tx_Buff[UART_485].tx_ready != READY)
							{
			
								memcpy((void*)Tx_Buff[UART_485].tx_buffer,(const void*)&Rx_Buff[UART_PC].rx_buffer[DATA_INDX],Rx_Buff[UART_PC].rx_buffer[LEN_INDX]-1);
						
								Tx_Buff[UART_485].tx_indx = Rx_Buff[UART_PC].rx_buffer[LEN_INDX]-1;
								Tx_Buff[UART_485].tx_ready = TRUE;
							
								timeout = 0;
								UI_handler_state = UI_MB_RSP_WAIT;
								RS485_device_handler.set_req_port(UART_PC);

							}
							else
							{
								
							}
						}
					
					}
				}
				else
				{	
					//Lets handle commands other than MB cmd 
					response((cmd_t)Rx_Buff[UART_PC].rx_buffer[CMD_INDX]);
				}
				
				flushRxBuffer(UART_PC);
			}
		break;
		case UI_MB_RSP_WAIT:

				if(Tx_Buff[UART_PC].tx_ready == TRUE)
				{
					Tx_Buff[UART_PC].tx_ready = FALSE;					
					
					
					reRoueMBResponsePacket();
					Usendbuffer(UART_PC);		//Send data
					flushTxBuffer(UART_PC);		//Flush buffer
					UI_handler_state = UI_IDLE;				
			
				}
				else
				{
					//timeout
					if(timeout++>= PORT_RSP_TIMEOUT )	//3
					{
						timeout = 0;
					
						flushTxBuffer(UART_PC);		//Flush buffer
						UI_handler_state = UI_IDLE;
					}
				}
				
						
		break;
	}

}


void preparePacket(int uart_no, char data, char len)		//
{
	
	Tx_Buff[uart_no].tx_indx = 0;
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = 0xAA;
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = len;
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = data;
	

	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = getChecksum((unsigned char*)&Tx_Buff[uart_no].tx_buffer[CMD_INDX], len);
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = 0x55;
}

void prepareResponsePacket(int uart_no, cmd_t cmd, char* data, unsigned char len)		//
{
	flushTxBuffer(uart_no);
	
	int i=0;
	Tx_Buff[uart_no].tx_indx = 0;
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = 0xAA;
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = len;
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = (cmd | 0x80);		//Anand 21.05.2014
	while(i<(len-1))
	{
		Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx] = data[i];
		i++;
		Tx_Buff[uart_no].tx_indx++;
	}
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = getChecksum((unsigned char*)&Tx_Buff[uart_no].tx_buffer[CMD_INDX], len);
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = 0x55;
	
	Tx_Buff[uart_no].tx_ready = TRUE;		//Anand 21.05.2014
}


void prepareResponsePacket(int uart_no, char cmd, char* data, unsigned char len, int sendAsIs, int immediateSend)		//Anand 17-4-15
{
	flushTxBuffer(uart_no);
	
	int i=0;
	
	if(!sendAsIs)
	{		
		Tx_Buff[uart_no].tx_indx = 0;
		
		Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = 0xAA;
		
		Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = len;
		
		Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = (cmd | 0x80);		//Anand 21.05.2014
		while(i<(len-1))
		{
			Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx] = data[i];
			i++;
			Tx_Buff[uart_no].tx_indx++;
		}
		
		Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = getChecksum((unsigned char*)&Tx_Buff[uart_no].tx_buffer[CMD_INDX], len);
		
		Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = 0x55;	
	}
	else
	{
		Tx_Buff[uart_no].tx_indx = 0;
		
		while(Tx_Buff[uart_no].tx_indx < len)
		{
			Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx] = data[Tx_Buff[uart_no].tx_indx];
			++Tx_Buff[uart_no].tx_indx;
		}
	}
	
	Tx_Buff[uart_no].tx_ready = TRUE;		//Anand 21.05.2014
	
	if(immediateSend)
	{
		Tx_Buff[uart_no].tx_ready = FALSE;
		Usendbuffer(uart_no);
	}
}

void reRoueMBResponsePacket(void)		//
{
	
	
	int i=0, uart_no = UART_PC ;
	int len = Tx_Buff[uart_no].tx_indx;
	char temp[TX_BUFFER_MAX];

	while(i<len)
	{
		temp[i] = Tx_Buff[uart_no].tx_buffer[i];
		i++;
	}
	
	Tx_Buff[uart_no].tx_indx = 0;
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = 0xAA;
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = len;
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = (CMD_MB_QUERY | 0x80);		//Anand 21.05.2014
	
	i=0;
	while(i<(len-1))
	{
		Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = temp[i];
		i++;
		
	}
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = getChecksum((unsigned char*)&Tx_Buff[uart_no].tx_buffer[CMD_INDX], len);
	
	Tx_Buff[uart_no].tx_buffer[Tx_Buff[uart_no].tx_indx++] = 0x55;
	
	Tx_Buff[uart_no].tx_ready = TRUE;		//Anand 21.05.2014
}

void response(cmd_t cmd)
{

	
	char resp[3];		//Anand 04-07-15
	resp[0] = FALSE;	//Anand 22.05.2014
	
	switch (cmd)
	{
		case CMD_CONNECT :
			setRtuState(RTU_UART_CONFIG);
			GLCD_ClearScreen();
			GLCD_GoTo(0,4);
			GLCD_WriteString((char*)"      PC CONFIG      ");
			resp[0] = ACK;

		break;
		case CMD_DISCONNECT:
			setRtuState(RTU_NORMAL);
			GLCD_ClearScreen();
			data_screen();
			resp[0] = ACK;

		break;
			case CMD_UPDATE_DATE_TIME :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				int indx = DATA_INDX;
				
				set_rtc((unsigned char *)&Rx_Buff[UART_PC].rx_buffer[indx]);

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_FACTORY_DEFAULTS:
			if(getRtuState() == RTU_UART_CONFIG)
			{
#ifdef FLASH_LOG_DATA
				clear_logs();
				flashClrTransID();
#ifdef FLASH_WP_ENABLE
				remove_block_protection();
#endif
				WREN();
				Chip_Erase();
				Wait_Busy();
				WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
				WBPR(0);
#endif
#else	//FLASH_LOG_DATA
				clear_e2p_log();
#endif	//FLASH_LOG_DATA
				//////////
				writeDefaults();
				GLCD_ClearScreen();
				GLCD_GoTo(0,3);
				GLCD_WriteString((char*)"     PRESS RESET     ");
				
				resp[0] = ACK;
				ACTIVITY_LED_PORT &= ~(1 << ACTIVITY_LED);
				while(1)
				{
					wdt_reset();
				}
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
#ifdef GPRS_ENABLE	
		case CMD_GET_GPRS_CFG:
			if(getRtuState() == RTU_UART_CONFIG)
			{
				eepromreadGPRSConfig();		
				prepareResponsePacket(UART_PC, CMD_GET_GPRS_CFG, (char*)&gprs_config_data.apn, APN_LEN + 1);
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		
		case CMD_SET_GPRS_CFG:		//Anand 22-4-15
			if(getRtuState() == RTU_UART_CONFIG)
			{				
				int indx = DATA_INDX, i;
							
				memset(&gprs_config_data.apn, 0, sizeof(gprs_config_data.apn));
				//
				for (i=0; Rx_Buff[UART_PC].rx_buffer[indx]!= '\0'; i++)		//Anand 22-4-15 / *i<* /
				{
					gprs_config_data.apn[i] = Rx_Buff[UART_PC].rx_buffer[indx++];
				}
				gprs_config_data.apn[i] = '\0';
				

				eepromWriteGPRSConfig(&gprs_config_data);			//Packet Updated, Now Update the E2P Data

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_GET_PORT_ADDR :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				eepromreadGPRSConfig();
				prepareResponsePacket(UART_PC, CMD_GET_PORT_ADDR, (char*)&gprs_config_data.port_address, sizeof(gprs_config_data.port_address) + 1);
				

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		
		case CMD_SET_PORT_ADDR :
			if(getRtuState() == RTU_UART_CONFIG)
			{								
				gprs_config_data.port_address = 0;
				gprs_config_data.port_address = (unsigned int)Rx_Buff[UART_PC].rx_buffer[DATA_INDX];
				gprs_config_data.port_address |= (unsigned int)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 1] << 8;
				
				if((gprs_config_data.port_address > MIN_PORT_ADDR) && (gprs_config_data.port_address > MAX_PORT_ADDR ))
				{
					eepromWriteGPRSConfig(&gprs_config_data);	
				}
				
			

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		

		case CMD_GET_CUM_DATA :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				// need to update with cumlative_struct with flash 12/12/2018 VC:
				FL_rtu_log_t temp_data;			//HJ 06-08-2016
				unsigned long addr = 0;
				unsigned long int temp_cum_data; 
				addr = flashGetCurrDataWriteAddr(RTU) - FL_RTU_LOG_LEN;
				
				cli();
				WREN();
				readContToBuff(addr, FL_RTU_LOG_LEN, (char *)&temp_data);
				Wait_Busy();
				WRDI();				   // Write Disable
				sei();
				
				temp_cum_data = temp_data.ram_data.cuml_reading;
				prepareResponsePacket(UART_PC, CMD_GET_CUM_DATA,(char *)temp_cum_data,sizeof(temp_data.ram_data.cuml_reading)+1);

				
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_SET_CUM_DATA :
			if(getRtuState() == RTU_UART_CONFIG)
			{
			// need to update with cumlative_struct with flash 12/12/2018 VC:

				
				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_GET_CALIB_FACTOR :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				eepromReadProductInfo();
				prepareResponsePacket(UART_PC, CMD_GET_CALIB_FACTOR, (char*)&product_info.ADC_offset, CALIB_FACTOR_LEN + 1);

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		
		case CMD_SET_CALIB_FACTOR :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				unsigned long temp1,temp2;	
				temp1 = ((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX])|((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 1]<<8);
				temp2 = ((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 2])|((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 3]<<8);
				
				product_info.ADC_offset = temp1;
				product_info.calib_factor = temp2;
				eepromWriteProductInfo();	



				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_GET_DATA_LOG :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				
				unsigned long num_of_records = ((flashGetCurrDataWriteAddr(RTU) - flashGetCurrUploadAddr(RTU))/FL_RTU_LOG_LEN);
				

				if(flashGetCurrUploadAddr(RTU) > flashGetCurrDataWriteAddr(RTU))
				{
					num_of_records = ((( FL_RTU_MAX_ADDR - flashGetCurrUploadAddr(RTU) ) + (flashGetCurrDataWriteAddr(RTU) - FL_RTU_START_ADDR))/FL_RTU_LOG_LEN);
				}

				if(Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 1] == ACK)
				{
					Tx_Buff[UART_PC].wait_for_ack = FALSE;
					Tx_Buff[UART_PC].pending_command = cmd;
					Tx_Buff[UART_PC].curr_pck_num = 1;

				}
				else
				{
					if(num_of_records)		//Anand 09.07.2014
					Tx_Buff[UART_PC].wait_for_ack = TRUE;

					resp[0] = num_of_records & 0xff;		//Anand 24.05.2014
					resp[1] = (num_of_records>>8) & 0xff;
					prepareResponsePacket(UART_PC, CMD_GET_DATA_LOG, resp, 3);		//Anand 21.05.2014

				}


			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_CLEAR_DATA_LOG : // clear
			if(getRtuState() == RTU_UART_CONFIG)
			{
#ifdef FLASH_LOG_DATA
				freq_updated_data.cnt = 0;
				freq_updated_data.unsent_rtu_logs_GPRS = 0;
				FL_rtu_log.transaction_id_MSB_rtu_log = 0;
				FL_rtu_log.transaction_id_rtu_log = 0;

				flashClearFreqUpdatedConfig();

				flashClrTransID();
				clear_logs();
#else	//FLASH_LOG_DATA
				clear_e2p_log();
				
#endif	//FLASH_LOG_DATA
				clear_ram_logs();
				

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_GET_KWH :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				
				/*eepromReadEmInfo();*/
				prepareResponsePacket(UART_PC, CMD_GET_KWH, (char*)&em_data.kwh_offset, sizeof(em_data.kwh_offset) + 1);

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		
		case CMD_SET_KWH :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				em_data.kwh_offset = 0;
				em_data.kwh_offset = (unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX];
				em_data.kwh_offset |= (unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 1]<<8;
				em_data.kwh_offset |= (unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 2]<<16;
				em_data.kwh_offset |= (unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 3]<<24;
				
			

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		
		case CMD_GET_KWAH :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				
				prepareResponsePacket(UART_PC, CMD_GET_KWAH, (char*)&em_data.kwah_offset, sizeof(em_data.kwah_offset) + 1);
				

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		
		case CMD_SET_KWAH :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				em_data.kwah_offset = 0;
				em_data.kwah_offset = Rx_Buff[UART_PC].rx_buffer[DATA_INDX];
				em_data.kwah_offset |= (unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 1] << 8;
				em_data.kwah_offset |= (unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 2] << 16;
				em_data.kwah_offset |= (unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 3] << 24;
				
			

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_GET_CALIBRATION :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				unsigned char cal_cmd = Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 1];
				switch(cal_cmd)
				{
					case CAL_FLOW:
						eepromReadCalData();
						prepareResponsePacket(UART_PC, CMD_GET_CALIBRATION, (char*)&calbration_data.cal_data_flow1, sizeof(calbration_data.cal_data_flow1) + 1);
					break;
					case CAL_PRESSURE:
						eepromReadCalData();
						prepareResponsePacket(UART_PC, CMD_GET_CALIBRATION, (char*)&calbration_data.cal_data_pressure1, sizeof(calbration_data.cal_data_pressure1) + 1);
					break;
					case CAL_LVL:
						eepromReadCalData();
						prepareResponsePacket(UART_PC, CMD_GET_CALIBRATION, (char*)&calbration_data.cal_data_lvl1, sizeof(calbration_data.cal_data_lvl1) + 1);
					break;
					
					default:
					break;		
				}

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_SET_CALIBRATION :
			if(getRtuState() == RTU_UART_CONFIG)
			{
				unsigned char cal_cmd = Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 1];
				switch(cal_cmd)
				{
					case CAL_FLOW: 
						calbration_data.cal_data_flow1 = (unsigned int)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 1];
						calbration_data.cal_data_flow1 |= (unsigned int)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 2] << 8;
						
						eepromWriteCalData();
					break;
					case CAL_PRESSURE:
						calbration_data.cal_data_pressure1 = (unsigned int)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 1];
						calbration_data.cal_data_pressure1 |= (unsigned int)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 2] << 8;
						
						eepromWriteCalData();
					break;
					case CAL_LVL:
						calbration_data.cal_data_lvl1 = (unsigned int)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 1];
						calbration_data.cal_data_lvl1 |= (unsigned int)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 2] << 8;
					
						eepromWriteCalData();
					break;
					
					default:
					break;
				}	
					

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
#endif
		case CMD_GET_SIM_NUM:
			if(getRtuState() == RTU_UART_CONFIG)
			{
				
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case CMD_SET_SIM_NUM:
			if(getRtuState() == RTU_UART_CONFIG)
			{
				int  i;
				
				for(i = 0; i<SIM_NUM_LEN - 1; i++)
				{
					
				}
				
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
			
		
		case CMD_REPET_SEND_DATA_LOGS :
			unsigned long pck_num;
			if(getRtuState() == RTU_UART_CONFIG)
			{
				pck_num = Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 1];
				pck_num += (((unsigned long)Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 2])<<8);		//Anand 06-07-15
				pck_num += (((unsigned long)Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 3])<<16);		//Anand 06-07-15
				
				{

					if(pck_num >  (((flashGetCurrDataWriteAddr(RTU) - FL_RTU_START_ADDR))/FL_RTU_LOG_LEN))
					{
														
						resp[0] = pck_num;
						resp[1] = pck_num<<8;
						resp[2] = pck_num<<16;
						prepareResponsePacket(UART_PC, CMD_GET_DATA_LOG, resp, 4);		
						Tx_Buff[UART_PC].wait_for_ack = FALSE;
						Tx_Buff[UART_PC].pending_command = cmd;
						Tx_Buff[UART_PC].curr_pck_num = 1;		
					}
					else
					{
						Tx_Buff[UART_PC].wait_for_ack = FALSE;
						Tx_Buff[UART_PC].pending_command = cmd;



						Tx_Buff[UART_PC].curr_pck_num = ((((flashGetCurrDataWriteAddr(RTU) - FL_RTU_START_ADDR))/FL_RTU_LOG_LEN) - pck_num) ;				

					}
				}
				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_SYS_STATE;
			}
		break;
		case SET_DAILY_LIMIT:
		if(getRtuState() == RTU_UART_CONFIG)
		{
			resp[0] = NACK;		//If everything's OK we'll make it ACK :)
			
			product_info.daily_limit = Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 1] + (((unsigned long)Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 2]) << 8);// + (((unsigned long)Rx_Buff[uart_no].rx_buffer[CMD_INDX + 3]) << 16) + (((unsigned long)Rx_Buff[uart_no].rx_buffer[CMD_INDX + 4]) << 24);
			product_info.daily_limit_dp = Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 3];
			eepromWriteProductInfo();		//Packet Updated, Now Update the E2P Data
			resp[0] = ACK;
		}
		else
		{
			resp[0] = ERROR_INVALID_SYS_STATE;
		}
		break;
		case GET_DAILY_LIMIT:
		if(getRtuState() == RTU_UART_CONFIG)
		{
			char data[10];
			
			memcpy(&data[0],&product_info.daily_limit,sizeof(unsigned int));
			memcpy(&data[2],&product_info.daily_limit_dp,sizeof(unsigned char));
			prepareResponsePacket(UART_PC, GET_PRODUCT_SER_NUM, (char *)data, 3+1);		//KP 20-5-15
		}
		else
		{
			resp[0] = ERROR_INVALID_SYS_STATE;
		}
		break;
		case READ_GPRS_CFG:
		if(getRtuState() == RTU_UART_CONFIG)
		{
			char *data = get_gprs_config((char *)&Rx_Buff[UART_PC].rx_buffer[CMD_INDX+1]);
			prepareResponsePacket(UART_PC, READ_GPRS_CFG, data, APN_LEN + 1);
		}
		else
		{
			resp[0] = ERROR_INVALID_SYS_STATE;
		}
		break;
		
		
		case EDIT_GPRS_CFG:		//Anand 22-4-15
		if(getRtuState() == RTU_UART_CONFIG)
		{
			update_gprs_config((char *)&Rx_Buff[UART_PC].rx_buffer[CMD_INDX+1]);

			resp[0] = ACK;
		}
		else
		{
			resp[0] = ERROR_INVALID_SYS_STATE;
		}
		break;
		
		case SET_PRODUCT_SER_NUM:
		if(getRtuState() == RTU_UART_CONFIG)
		{
			resp[0] = NACK;		//If everything's OK we'll make it ACK :)
			
			product_info.industry_id = Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 1] + (((unsigned long)Rx_Buff[UART_PC].rx_buffer[CMD_INDX + 2]) << 8);// + (((unsigned long)Rx_Buff[uart_no].rx_buffer[CMD_INDX + 3]) << 16) + (((unsigned long)Rx_Buff[uart_no].rx_buffer[CMD_INDX + 4]) << 24);
			eepromWriteProductInfo();		//Packet Updated, Now Update the E2P Data
			resp[0] = ACK;
		}
		else
		{
			resp[0] = ERROR_INVALID_SYS_STATE;
		}
		break;
		case GET_PRODUCT_SER_NUM:
		if(getRtuState() == RTU_UART_CONFIG)
		{
			prepareResponsePacket(UART_PC, GET_PRODUCT_SER_NUM, (char *)&product_info.industry_id, (sizeof(unsigned int) + 1));		//KP 20-5-15
		}
		else
		{
			resp[0] = ERROR_INVALID_SYS_STATE;
		}
		break;
	
		case GET_TOTALIZER://abdul majeed
		if(getRtuState() == RTU_UART_CONFIG)
		{
			FL_rtu_log_t temp_data;
			unsigned long addr = 0;
			char temp_buff[18];
			memset(temp_buff,0,sizeof(temp_buff));
			
			
			addr = flashGetCurrDataWriteAddr(RTU) - FL_RTU_LOG_LEN;
			cli();
			WREN();
			readContToBuff(addr, FL_RTU_LOG_LEN, (char *)&temp_data);
			Wait_Busy();
			WRDI();				   // Write Disable
			sei();
			memcpy(&temp_buff,&temp_data.ram_data.todays_total,(sizeof(ram_data.todays_total)+sizeof(ram_data.months_total)+sizeof(ram_data.cuml_reading)));
			memcpy(&temp_buff[12],&yest_total,sizeof(yest_total));
			

			prepareResponsePacket(UART_PC, GET_TOTALIZER, temp_buff, (sizeof(ram_data.todays_total)+sizeof(ram_data.months_total)+sizeof(ram_data.cuml_reading)+sizeof(yest_total)+1));//size of today tot+mon tot+cum tot+1
			
		}
		else
		{
			resp[0] = ERROR_INVALID_SYS_STATE;
		}
		break;
		
		case SET_TOTALIZER:
		if(getRtuState() == RTU_UART_CONFIG)
		{
			resp[0] = NACK;		//If everything's OK we'll make it ACK :)
			
			
			ram_data.todays_total =	((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX])|((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 1] << 8);
			ram_data.todays_total |=((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 2] << 16)|((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 3] << 24);
			
			ram_data.months_total = ((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX+4])|((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 5] << 8);
			ram_data.months_total |=((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 6] << 16)|((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 7] << 24);
			
			ram_data.cuml_reading = ((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX+8]) |((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 9] << 8);
			ram_data.cuml_reading |=((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 10] << 16)|((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 11] << 24);
			
			yest_total = ((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX+ 12])|((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 13] << 8);
			yest_total |= ((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 14] << 16)|((unsigned long)Rx_Buff[UART_PC].rx_buffer[DATA_INDX + 15] << 24);
			
			
#ifdef FLASH_LOG_DATA
				prepare_FL_LogData();
				save_FL_rtu_data();
#else
				prepare_e2p_logdata();
				e2p_write_log_data();
#endif
			prepare_e2p_y_total();
			e2p_write_y_total();
			resp[0] = ACK;
			
		}
		else
		{
			resp[0] = ERROR_INVALID_SYS_STATE;
		}		
		break;
	
		case DEBUG_RS485_CMD:
		{
			flushTxBuffer(UART_485);
			memcpy((void *)&Tx_Buff[UART_485].tx_buffer[0],(void *)&Rx_Buff[UART_PC].rx_buffer[DATA_INDX],(Rx_Buff[UART_PC].rx_indx - 5));
			Tx_Buff[UART_485].tx_indx = Rx_Buff[UART_PC].rx_indx - 5;
			
			Usendbuffer(UART_485);

			flushRxBuffer(UART_485);

			
		}
		break;

		default:
			{
				resp[0] = ERROR_CMD;
			}
		break;
	}
	
	if(resp[0] && (!Tx_Buff[UART_PC].tx_ready))
	{
		prepareResponsePacket(UART_PC, cmd, resp, 2);		//Anand 21.05.2014
	}

}

void clear_logs(void)
{
	
	flashClrRTULogs();
	flashClrMRBR();
	
	updateFlashCurrAddr();		//Anand 19.04.2014	
}


void completePendingCommand()
{
	unsigned long addr = 0;

	
	FL_rtu_log_t dispense_upload_data;			//HJ 06-08-2016


	addr = flashGetCurrUploadAddr(RTU)  + ((Tx_Buff[UART_PC].curr_pck_num-1)*FL_RTU_LOG_LEN);

	if(addr>=FL_RTU_MAX_ADDR)
	{
		addr = ((addr%FL_RTU_MAX_ADDR)+FL_RTU_START_ADDR);		//Anand 16-7-15	Problem at addr=FL_MAX_DR_ADDR so don't need subtraction!
	}
	

	WREN();
	
	readContToBuff(addr, FL_RTU_LOG_LEN, (char *)&dispense_upload_data);		//HJ 06-08-2016
	Wait_Busy();	//??
	WRDI();				// HJ 29-12-2015    // Write Disable
	
	prepareResponsePacket(UART_PC, CMD_GET_DATA_LOG, (char*)&dispense_upload_data, FL_RTU_LOG_LEN + 1 - FL_RESERVED_BYTE - 1);		//HJ 06-08-2016

	if(addr == flashGetCurrDataWriteAddr(RTU))
	{
		
		Tx_Buff[UART_PC].pending_command = FALSE;
		Tx_Buff[UART_PC].curr_pck_num = 0;
		char resp[3];		//Anand 04-07-15
		resp[0] = resp[1] = resp[2] = 0xFF;
		prepareResponsePacket(UART_PC, CMD_GET_DATA_LOG, resp, 4);		//??		//Anand 04-07-15

	}

	else
	{

	}						
	
}