/*
 * GPRS.c
 *
 * Created: 3/9/2016 11:40:15 AM
 *  Author: Trilok
 */ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "gprs.h"
#include "common.h"
#include "_debug.h"
#include "uart.h"
#include "app_uart.h"
#include "app_eeprom.h"
#include "sms.h"
#include "gprs_config.h" 
#include "pins.h"
#include "web_comm.h"
#include "config.h"

unsigned int cfun_cnt = 0;

#ifdef GPRS_ENABLE
gprs_t gprs;
gprs_config_data_t gprs_config_data;
gprs_date_time_t gprs_date_time;
extern unsigned int db_save_status;
extern freq_updated_data_t freq_updated_data;	//AM 12/07/21

volatile char gprs_uart_init = FALSE;
volatile gprs_tx_data_buff_t gprs_tx_buff;
volatile gprs_rx_data_buff_t gprs_rx_buff;

volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;		//Anand 07-04-16

#ifdef HTTP_ENABLE
http_t http;
#endif
gprs_conn_status _gprs_conn_status;

gprs_rx_data_buff_t uart_to_gprs_temp_buff;

extern ram_data_t ram_data;

#ifdef ONLINE_CONFIG_EN
online_cfg_t online_cfg;

void init_online_cfg(void)
{
	memset((void*)&online_cfg, 0, sizeof(online_cfg_t));
	online_cfg.cfg_pending = TRUE;
	online_cfg.online_cfg_state = CFG_VER_CNT;
}
#endif

con_status_t gprs_connect(void)
{
	con_status_t sts = CON_IN_PRG;
	static char gprs_retry_count = 0, sim_module_reset = 0;
	static unsigned int timeout = 0;			//Can be used with all commands
	int num_byte = 0;

	static gprs_connct_state_t gprs_connct_state = GPRS_CONNCT_POWER_UP;	
	
	switch(gprs_connct_state)
	{
		case GPRS_CONNCT_POWER_UP:
		{
			if (timeout++ >= GSM_WARMUP_SECS)		//not getting expected data,
			{
				gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
				timeout = 0;
			}
		}
		break;
		
		
		case GPRS_CONNCT_CMD_ECHO_OFF:
		{
			UWriteString("ATE0\r", GPRS_UART);
#ifdef GPRS_DEBUG
UWriteString("GR:e_of", UART_PC);
#endif
			gprs_connct_state = GPRS_CONNCT_RSP_ECHO_OFF;
			
		}
		break;
		
		case GPRS_CONNCT_RSP_ECHO_OFF:
		
		{
			//Lets clear all buffers since we have not handle its response
#ifdef GPRS_DEBUG
UWriteString("GR:R_e_of", UART_PC);
#endif
			flushRxBuffer(GPRS_UART);
			
			gprs_connct_state = GPRS_CONNCT_CMD_IMEI;
		}
		break;
		
		case GPRS_CONNCT_CMD_IMEI:
		{
			
			flushTxBuffer(GPRS_UART);
			UWriteString("AT+CGSN\r", GPRS_UART);
			
#ifdef GPRS_DEBUG
UWriteString("IMEI", UART_PC);
#endif
			gprs_connct_state = GPRS_CONNCT_RSP_IMEI;
		}
		break;
		
		case GPRS_CONNCT_RSP_IMEI:
		{
			char tmpstr[IMEI_NUM_LEN + 9];
			static char wait_for_data = 0;
			int num_bytes = 0;
			char resp = check_string("\r\n", tmpstr,&num_bytes);

			switch (resp)
			{
				case (GPRS_MATCH_FAIL):
				{
					gprs_connct_state = GPRS_CONNCT_CMD_IMEI;
					flushRxBuffer(GPRS_UART);
					gprs_retry_count++;
					if (gprs_retry_count >= GPRS_RETRY_CNT)
					{
						gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
						
						gprs_retry_count = 0;
						
#ifdef GPRS_DEBUG
UWriteString("IMEI_R:mf", UART_PC);
#endif
						if(++sim_module_reset >= SIM_MODULE_RESET_CNT)
						{
							sim_module_reset = 0;
							gprs.module_status = NOT_AVBL;
							set_conn_state(CONNECT_POWER_ON);
							sts = CON_FAIL;
						
#ifdef GPRS_DEBUG
UWriteString("gprs_power_up:",UART_PC);
#endif
							gprs_connct_state = GPRS_CONNCT_POWER_UP;			
							gprs_retry_count = 0;
							flushRxBuffer(GPRS_UART);
						}
					}
				}
				break;
				case (GPRS_MATCH_OK):
				{
					
					if(num_bytes >= IMEI_NUM_LEN )
					{
						
						if(isStringDigit(tmpstr))
						{
							set_imei(tmpstr);
							
	#ifdef GPRS_DEBUG
	UWriteString("IMEI_R", UART_PC);
	UWriteString((char*)gprs.imei,UART_PC);
	#endif
							gprs_connct_state = GPRS_CONNCT_CMD_CLTS;
							
						}
						else
						{
							gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
#ifdef GPRS_DEBUG
UWriteString("IMEI_R:f", UART_PC);
UWriteString(tmpstr,UART_PC);
#endif
							sts = CON_FAIL;
						}
					}
					else if(wait_for_data++ >= IMEI_WAIT_TIME)
					{
						wait_for_data = 0;
						
						gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
						sts = CON_FAIL;
#ifdef GPRS_DEBUG
UWriteString("IMEI_R:wf", UART_PC);
#endif
					}
					
				}
				break;
				case (GPRS_NO_NEW_MSG):
				{
	
					if(timeout++ >= GPRS_RETRY_CNT)
					{
						timeout = 0;
					
						if (gprs_retry_count++ >= GPRS_RETRY_CNT)
						{
							gprs.module_status = NOT_AVBL;
							set_conn_state(CONNECT_POWER_ON);
							sts = CON_FAIL;
							
#ifdef GPRS_DEBUG
UWriteString("gprs_power_up:",UART_PC);
#endif
							gprs_connct_state = GPRS_CONNCT_POWER_UP;			//
							gprs_retry_count = 0;
							flushRxBuffer(GPRS_UART);
						}
					}
				}
				break;
			}
		}
		break;

		
		case GPRS_CONNCT_CMD_CFUN:
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CFUN", UART_PC);
#endif
			flushTxBuffer(GPRS_UART);
			UWriteString((char *)"AT+CFUN?\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_CFUN;
		break;
		case GPRS_CONNCT_RSP_CFUN:
			switch (check_string_nobuf("+CFUN: 1"))
		{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
					UWriteString((char *)"CFUN:f", UART_PC);
#endif					
					gprs_connct_state = GPRS_CONNCT_CMD_CFUN;
			
			if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs_retry_count = 0;
						
						gprs.module_status = NOT_AVBL;
						
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CFUN;
						sts = CON_FAIL;
						
						gprs_connct_state = GPRS_CONNCT_RESET;			//
						timeout = 0;
					}
					break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
					UWriteString((char *)"CFUN:k", UART_PC);
#endif					
					
					gprs_connct_state = GPRS_CONNCT_CMD_AT;
					gprs_retry_count = 0;
					timeout = 0;
					break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
					UWriteString("CFUN:w", UART_PC);
#endif

					if(timeout++ >= CCALR_TIMEOUT)
					{
						timeout = 0;
					
						gprs_connct_state = GPRS_CONNCT_CMD_CFUN;
					
					if (gprs_retry_count++ >= RETRY_CNT)
						{
							gprs_retry_count = 0;
						
							gprs.module_status = NOT_AVBL;
						
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CFUN;
							sts = CON_FAIL;
							
							gprs_connct_state = GPRS_CONNCT_RESET;			//
							gprs_retry_count = 0;
						}
					}
				break;
				default:
				break;
			}
		break;
		
		case GPRS_CONNCT_CMD_AT:
		{
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: AT", UART_PC);
#endif
			flushRxBuffer(GPRS_UART);
			UWriteString((char *)"AT\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_AT;
		}
		break;
		case GPRS_CONNCT_RSP_AT:
			switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
					UWriteString((char *)"AT:f", UART_PC);
#endif					
				
					gprs_connct_state = GPRS_CONNCT_CMD_AT;
				
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs.module_status = NOT_AVBL;
						
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_AT;
						sts = CON_FAIL;
						
						gprs_connct_state = GPRS_CONNCT_RESET;			//
						gprs_retry_count = 0;
					}
				break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
					UWriteString((char *)"AT:k", UART_PC);
#endif					
					
					gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
					gprs_retry_count = 0;
					timeout = 0;
				break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
					UWriteString("AT:w", UART_PC);
#endif							
					if(timeout++ >= GPRS_AT_TIMEOUT)
					{
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_AT;
					
						if (gprs_retry_count++ >= RETRY_CNT)
						{
							gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_AT;
							sts = CON_FAIL;
							
							gprs_connct_state = GPRS_CONNCT_RESET;			//
							gprs_retry_count = 0;
						}
					}
				break;
				
				default:
				break;
			}
		break;
		
		case GPRS_CONNCT_CMD_CCLK:
		{
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CCLK", UART_PC);
#endif
			gprs_date_time.update_time_aval = FALSE;
			UWriteString((char *)"AT+CCLK?\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_CCLK;
		}
		break;
		case GPRS_CONNCT_RSP_CCLK:
		{
			char tmpstr[GPRS_RX_BUFFER_MAX];
			gprs_date_time_t temp_date_time;
			char resp = check_string("+CCLK: \"", tmpstr, &num_byte);

#ifdef GPRS_DEBUG		//Anand 27-04-16
			UWriteString((char *)"RSP_CCLK:", UART_PC);
			UWriteString(tmpstr, UART_PC);
#endif
			switch (resp)
			{
				case (GPRS_MATCH_FAIL): 
#ifdef GPRS_DEBUG
					UWriteString((char *)"CCLK:f", UART_PC);
#endif					
					gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
					gprs_retry_count++;
					if (gprs_retry_count >= RETRY_CNT) 
					{
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK;
						
						gprs_connct_state = GPRS_CONNCT_RESET;
						gprs_retry_count = 0;
					}
					break;
				case (GPRS_MATCH_OK):
					{
#ifdef GPRS_DEBUG
						UWriteString((char *)"CCLK:k", UART_PC);
#endif						
						if (check_date_time(tmpstr, (char *)&temp_date_time))
						{

							if (((temp_date_time.yy >= DEFAULT_YEAR) && (temp_date_time.yy <= 99)) && 			// Assuming that RTC will never go below 2020.
							((temp_date_time.mm >= 1) && (temp_date_time.mm <= 12)) &&
							((temp_date_time.dd >= 1) && (temp_date_time.dd <= 31))&&
							((temp_date_time.hr >= 0) && (temp_date_time.hr <= 23)) &&
							((temp_date_time.min >= 0) && (temp_date_time.min <= 59)) &&
							((temp_date_time.sec >= 0) && (temp_date_time.sec <= 59)))

							{
								memcpy(&gprs_date_time,&temp_date_time,sizeof(gprs_date_time_t));
								gprs_date_time.update_time_aval = TRUE;
							}
							else
							{
							}
						}

						gprs_connct_state = GPRS_CONNCT_CMD_CSQ;
						gprs_retry_count = 0;
					}
					break;
				case (GPRS_NO_NEW_MSG): 
#ifdef GPRS_DEBUG
					UWriteString((char *)"CCLK:w", UART_PC);
#endif					
					if(timeout++ >= CCLK_TIMEOUT)
					{
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
						
						if (gprs_retry_count++ >= RETRY_CNT)
						{
							gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK;
							
							gprs_connct_state = GPRS_CONNCT_RESET;			
							gprs_retry_count = 0;
						}
					}
					break;
				default:
					break;
			}
		}
		break;
		
		case GPRS_CONNCT_CMD_CSQ:
		{
			flushRxBuffer(GPRS_UART);
			
#ifdef GPRS_DEBUG		//Anand 27-04-16
UWriteString("CSQ sent", UART_PC);
#endif
			UWriteString("AT+CSQ\r", GPRS_UART); //interrupt 07/08/2018
			gprs.network_status = NOT_AVBL;				//Lets see if we have network
			gprs_connct_state = GPRS_CONNCT_RSP_CSQ;
		}
		break;
	
		case GPRS_CONNCT_RSP_CSQ:
		{
			char tmpstr[GPRS_RX_BUFFER_MAX];
			char resp = check_string("+CSQ: ", tmpstr, &num_byte);

			switch (resp)
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
UWriteString("CSQ:f", UART_PC);
#endif
				gprs_connct_state = GPRS_CONNCT_CMD_CSQ;
				flushRxBuffer(GPRS_UART);
				gprs_retry_count++;
				if (gprs_retry_count >= RETRY_CNT)
				{
					gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CSQ;
					sts = CON_FAIL;
					
					gprs_connct_state = GPRS_CONNCT_RESET;
					gprs_retry_count = 0;
				}
				break;
				
				case (GPRS_MATCH_OK):
				{
#ifdef GPRS_DEBUG
UWriteString("CSQ:k", UART_PC);
#endif
					//Lets update network strength
					gprs.network_strength = (tmpstr[0]-'0')*10 + (tmpstr[1]-'0');
					
					if((gprs.network_strength >= MIN_NETWORK_STRENGTH_DB) && (gprs.network_strength <= MAX_NETWORK_STRENGTH_DB))// network strength upto
						gprs.network_status = AVBL;
					gprs_connct_state = GPRS_CONNCT_CMD_CGATT;// GPRS_CONNCT_RSP_1_CSQ;
					gprs_retry_count = 0;
				}
				break;
				
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
UWriteString("CSQ:w", UART_PC);
#endif
				if(timeout++ >= CSQ_TIMEOUT)
				{
					timeout = 0;
					gprs_connct_state = GPRS_CONNCT_CMD_CSQ;
					
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs.module_status = NOT_AVBL;
						
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CSQ;
						sts = CON_FAIL;
						
						gprs_connct_state = GPRS_CONNCT_RESET;		
						gprs_retry_count = 0;
					}
				}
				break;
				default:
				break;
			}
		}
		break;
		case GPRS_CONNCT_CMD_CGATT:
		{
			flushRxBuffer(GPRS_UART);
			
			UWriteString("AT+CGATT?\r", GPRS_UART); //interrupt 07/08/2018

			gprs_connct_state = GPRS_CONNCT_RSP_CGATT;
		}
		break;
		case GPRS_CONNCT_RSP_CGATT:
#ifdef GPRS_DEBUG		//Anand 27-04-16
UWriteString("RSP CGATT:\n", UART_PC);
#endif
		switch (check_string_nobuf("+CGATT: 1"))
		{
			case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
UWriteString("CG:f", UART_PC);
#endif
				gprs_connct_state = GPRS_CONNCT_CMD_CGATT;
			
				if (gprs_retry_count++ >= RETRY_CNT)
				{
					gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CGATT;
					sts = CON_FAIL;
				
					gprs_connct_state = GPRS_CONNCT_RESET;			//
					gprs_retry_count = 0;
				}
			break;
			
			case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
UWriteString("CG:k", UART_PC);
#endif
				
#ifdef HTTP_ENABLE
						gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_TYPE;
#endif	//HTTP_ENABLE

				gprs_retry_count = 0;
				timeout = 0;
			break;
			case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
UWriteString("CG:w", UART_PC);
#endif
				if(timeout++ >= CGATT_TIMEOUT)
				{
					timeout = 0;
					gprs_connct_state = GPRS_CONNCT_CMD_CGATT;
				
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs.module_status = NOT_AVBL;
					
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CGATT;
						sts = CON_FAIL;
					
						gprs_connct_state = GPRS_CONNCT_RESET;			
						gprs_retry_count = 0;
					}
				}
			break;
			
			default:
			break;
		}
		
		break;
#ifdef HTTP_ENABLE
		case GPRS_CONNCT_CMD_SAPBR_TYPE:
		{
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CONTYPE",UART_PC);
#endif			
			flushRxBuffer(GPRS_UART);
			UWriteString((char *)"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_SAPBR_TYPE;
		}
		break;	
		case GPRS_CONNCT_RSP_SAPBR_TYPE:
			switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
					UWriteString((char *)"SAT:f", UART_PC);
#endif				
					gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_TYPE;
				
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_TYPE;
						sts = CON_FAIL;
						
						gprs_connct_state = GPRS_CONNCT_RESET;			
						gprs_retry_count = 0;
					}
				break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
					UWriteString((char *)"SAT:k", UART_PC);
#endif					
					gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_APN;
					gprs_retry_count = 0;
					timeout = 0;
				break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
					UWriteString((char *)"SAT:w", UART_PC);
#endif					
					if(timeout++ >= SAPBR_TYPE_TIMEOUT)
					{
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_TYPE;
					
						if (gprs_retry_count++ >= RETRY_CNT)
						{
							gprs.module_status = NOT_AVBL;
					
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_TYPE;
							sts = CON_FAIL;
							
							gprs_connct_state = GPRS_CONNCT_RESET;			//
							gprs_retry_count = 0;
						}
					}
				break;
				
				default:
				break;
			}
		break;
		case GPRS_CONNCT_CMD_SAPBR_APN:
		{
			char tmpstr[CMD_BUFFER_LEN];
#ifdef GPRS_DEBUG			
			UWriteString((char *)"cmd: APN",UART_PC);
#endif			
			flushRxBuffer(GPRS_UART);
			sprintf(tmpstr,"AT+SAPBR=3,1,\"APN\",\"%s\"\r", gprs_config_data.apn);
						
			UWriteString(tmpstr, GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_SAPBR_APN;		//Anand 27-04-16
		}
		break;	
		case GPRS_CONNCT_RSP_SAPBR_APN:
			switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
					UWriteString((char *)"SAA:f", UART_PC);
#endif					
					gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_APN;
				
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_APN;
						sts = CON_FAIL;
						
						gprs_connct_state = GPRS_CONNCT_RESET;			
						gprs_retry_count = 0;
					}
				break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
					UWriteString((char *)"SAA:k", UART_PC);
#endif					
					gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_IP;
					gprs_retry_count = 0;
					timeout = 0;
				break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
					UWriteString("SAA:w", UART_PC);
#endif								
					if(timeout++ >= SAPBR_APN_TIMEOUT)
					{
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_APN;
					
						if (gprs_retry_count++ >= RETRY_CNT)
						{
							gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_APN;
							sts = CON_FAIL;
							
							gprs_connct_state = GPRS_CONNCT_RESET;			
							gprs_retry_count = 0;
						}
					}
				break;
				
				default:
				break;
			}
		break;
		case GPRS_CONNCT_CMD_SAPBR_IP:
		{
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: IP", UART_PC);
#endif			
			flushRxBuffer(GPRS_UART);
			UWriteString((char *)"AT+SAPBR=1,1\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_SAPBR_IP;
		}
		break;	
		case GPRS_CONNCT_RSP_SAPBR_IP:
			switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
					UWriteString((char *)"IP:f", UART_PC);
#endif				
					gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_IP;
					
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_IP;
					
						gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_DISCON;			
						gprs_retry_count = 0;
					}
				break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
					UWriteString((char *)"IP:k", UART_PC);
#endif					
					//We have passed all steps so lets make status ok
					sts = CON_OK;
					gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
					
					gprs_retry_count = 0;
					timeout = 0;
				break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
					UWriteString("IP:w", UART_PC);
#endif					
					if(timeout++ >= SAPBR_APN_TIMEOUT)
					{
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_IP;
					
						if (gprs_retry_count++ >= RETRY_CNT)
						{
							gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_IP;
							sts = CON_FAIL;
							
							gprs_connct_state = GPRS_CONNCT_RESET;			//
							gprs_retry_count = 0;
						}
					}
				break;
				
				default:
				break;
			}
		break;
		case GPRS_CONNCT_CMD_SAPBR_DISCON:
		{
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: DISCON", UART_PC);
#endif			
			flushRxBuffer(GPRS_UART);
			UWriteString((char *)"AT+SAPBR=0,1\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_SAPBR_DISCON;
		}
		break;
		case GPRS_CONNCT_RSP_SAPBR_DISCON:
		switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
					UWriteString((char *)"DISCON:f", UART_PC);
#endif				
					gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_DISCON;
					//flushRxBuffer(GPRS_UART);
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_DISCON;
						sts = CON_FAIL;
						
						gprs_connct_state = GPRS_CONNCT_RESET;			//
						gprs_retry_count = 0;
					}
				break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
					UWriteString((char *)"DISCON:k", UART_PC);
#endif				
					
					gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
					
					gprs_retry_count = 0;
					timeout = 0;
				break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
					UWriteString("DISCON:w", UART_PC);
#endif					
					if(timeout++ >= SAPBR_DISCON_TIMEOUT)
					{
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_DISCON;
					
						if (gprs_retry_count++ >= RETRY_CNT)
						{
							gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_DISCON;
							sts = CON_FAIL;
							
							gprs_connct_state = GPRS_CONNCT_RESET;			//
							gprs_retry_count = 0;
						}
					}
				break;				
				default:
				break;
			}
		break;
#endif	//HTTP_ENABLE
		case GPRS_CONNCT_CMD_CLTS:
		{
			#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CLTS", UART_PC);
			#endif
			flushRxBuffer(GPRS_UART);
			UWriteString((char *)"AT+CLTS=1;&w\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_CLTS;
		}
		break;
		case GPRS_CONNCT_RSP_CLTS:
		switch (check_string_nobuf("OK"))
		{
			case (GPRS_MATCH_FAIL):
			#ifdef GPRS_DEBUG
			UWriteString((char *)"CLTS:f", UART_PC);
			#endif
			gprs_connct_state = GPRS_CONNCT_CMD_CLTS;
			
			if (gprs_retry_count++ >= RETRY_CNT)
			{
				gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CLTS;
				sts = CON_FAIL;
				
				gprs_connct_state = GPRS_CONNCT_RESET;			//
				gprs_retry_count = 0;
			}
			break;
			case (GPRS_MATCH_OK):
			#ifdef GPRS_DEBUG
			UWriteString((char *)"CLTS:k", UART_PC);
			#endif
			
			gprs_connct_state = GPRS_CONNCT_CMD_CFUN;
			
			gprs_retry_count = 0;
			timeout = 0;
			break;
			case (GPRS_NO_NEW_MSG):
			#ifdef GPRS_DEBUG
			UWriteString("CLTS:w", UART_PC);
			#endif
			if(timeout++ >= CLTS_TIMEOUT)
			{
				timeout = 0;
				gprs_connct_state = GPRS_CONNCT_CMD_CLTS;
				
				if (gprs_retry_count++ >= RETRY_CNT)
				{
					gprs.module_status = NOT_AVBL;
					
					gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CLTS;
					sts = CON_FAIL;
					
					gprs_connct_state = GPRS_CONNCT_RESET;			//
					gprs_retry_count = 0;
				}
			}
			break;
			
			default:
			break;
		}
		break;

		case GPRS_CONNCT_RESET:
		{
			timeout = 0;
			gprs_retry_count = 0;
#ifdef SOFT_RST_GSM
			gprs_connct_state = GPRS_CONNCT_CMD_RFUN;
#else
			gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
#endif
		}
		break;
		
#ifdef SOFT_RST_GSM
		case GPRS_CONNCT_CMD_RFUN:
		{
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: RFUN", UART_PC);
#endif
			flushRxBuffer(GPRS_UART);
			UWriteString((char *)"AT+CFUN=1,1\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_RFUN;
			
UWriteString((char *)"CFUN_CNT", UART_PC);			
UWriteInt(cfun_cnt++, UART_PC);

		}
		break;
		
		case GPRS_CONNCT_RSP_RFUN:
		switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
				{
#ifdef GPRS_DEBUG
					UWriteString((char *)"RFUN:f", UART_PC);
#endif
					gprs_connct_state = GPRS_CONNCT_CMD_RFUN;
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_RFUN;
						sts = CON_FAIL;
				
						gprs_connct_state = GPRS_CONNCT_RFUN_WAIT;
						gprs_retry_count = 0;
						timeout = 0;
					}
				}
				break;
				case (GPRS_MATCH_OK):
				{
#ifdef GPRS_DEBUG
					UWriteString((char *)"RFUN:k", UART_PC);
#endif
					
					gprs_connct_state = GPRS_CONNCT_RFUN_WAIT;
			
					gprs_retry_count = 0;
					timeout = 0;
				}
				break;
				case (GPRS_NO_NEW_MSG):
				{
#ifdef GPRS_DEBUG
					UWriteString("RFUN:w", UART_PC);
#endif
					if(timeout++ >= SOFT_RST_TIMEOUT)
					{
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_RFUN;
				
						if (gprs_retry_count++ >= RETRY_CNT)
						{
							gprs.module_status = NOT_AVBL;
					
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_RFUN;
							sts = CON_FAIL;
					
							gprs_connct_state = GPRS_CONNCT_POWER_UP;			//
							gprs_retry_count = 0;
						}
					}
				}
				break;
			}
		case GPRS_CONNCT_RFUN_WAIT:
		{
			if (timeout++ >= GSM_RST_SECS)		//not getting expected data,
			{
				flushRxBuffer(GPRS_UART);
				gprs_connct_state = GPRS_CONNCT_POWER_UP;
				timeout = 0;
			}
		}
		break;
#endif
		default:
			gprs_connct_state = GPRS_CONNCT_RESET;
		break;	
	}
	
	return sts;
}

con_status_t gprs_connect_status(void)
{
	con_status_t sts = CON_IN_PRG;
	
	static char gprs_retry_count = 0;
	static unsigned int timeout = 0;			//Can be used with all commands
	int num_byte = 0;

#ifdef HTTP_ENABLE
	static gprs_connct_state_t gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_CONN_STS;
#endif	//HTTP_ENABLE


	switch(gprs_connct_state)
	{
		case GPRS_CONNCT_CMD_CCLK:
		{
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CCLK", UART_PC);
#endif
			flushRxBuffer(GPRS_UART);
			gprs_date_time.update_time_aval = FALSE;

			UWriteString((char *)"AT+CCLK?\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_CCLK;
		}
		break;
		case GPRS_CONNCT_RSP_CCLK:
		{
			char tmpstr[GPRS_RX_BUFFER_MAX];
			gprs_date_time_t temp_date_time;
			char resp = check_string("+CCLK: \"", tmpstr, &num_byte);

#ifdef GPRS_DEBUG		//Anand 27-04-16
			UWriteString((char *)"RSP_CCLK:", UART_PC);
			UWriteString(tmpstr, UART_PC);
#endif
			switch (resp)
			{
				case (GPRS_MATCH_FAIL): 
#ifdef GPRS_DEBUG
					UWriteString((char *)"CCLK:f", UART_PC);
#endif					
					gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
					
					if (gprs_retry_count++ >= RETRY_CNT) 
					{
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK;
						
					
						gprs_retry_count = 0;
					}
					break;
				case (GPRS_MATCH_OK):
					{
#ifdef GPRS_DEBUG
						UWriteString((char *)"CCLK:k", UART_PC);
#endif						
						
						if (check_date_time(tmpstr, (char *)&temp_date_time))
						{
							if (((temp_date_time.yy >= DEFAULT_YEAR) && (temp_date_time.yy <= 99)) && 			// Assuming that RTC will never go below 2020.
							((temp_date_time.mm >= 1) && (temp_date_time.mm <= 12)) &&
							((temp_date_time.dd >= 1) && (temp_date_time.dd <= 31))&&
							((temp_date_time.hr >= 0) && (temp_date_time.hr <= 23)) &&
							((temp_date_time.min >= 0) && (temp_date_time.min <= 59)) &&
							((temp_date_time.sec >= 0) && (temp_date_time.sec <= 59)))
							{

								memcpy(&gprs_date_time,&temp_date_time,sizeof(gprs_date_time_t));
								gprs_date_time.update_time_aval = TRUE;
							}
							else
							{

							}
						}

						

#ifdef HTTP_ENABLE
						gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_CONN_STS;
#endif	//HTTP_ENABLE


						gprs_retry_count = 0;
						timeout = 0;
					}
					break;
				case (GPRS_NO_NEW_MSG): 
#ifdef GPRS_DEBUG
					UWriteString((char *)"CCLK:w", UART_PC);
#endif					
					if(timeout++ >= CCLK_TIMEOUT)
					{
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
						
						if (gprs_retry_count++ >= RETRY_CNT)
						{
							gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK;
							//sts = CON_FAIL;
							

#ifdef HTTP_ENABLE
							gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_CONN_STS;
#endif	//HTTP_ENABLE

							gprs_retry_count = 0;
						}
					}
					break;
				default:
					break;
			}
		}
		break;
#ifdef HTTP_ENABLE
		case GPRS_CONNCT_CMD_SAPBR_CONN_STS:
		{
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CONN_STS", UART_PC);
#endif
			flushRxBuffer(GPRS_UART);
			UWriteString((char *)"AT+SAPBR=2,1\r", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_SAPBR_CONN_STS;
		}
		break;
		case GPRS_CONNCT_RSP_SAPBR_CONN_STS:
		switch (check_string_nobuf("+SAPBR: 1,1"))
		{
			case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
			UWriteString((char *)"CONN_STS:f", UART_PC);
#endif
			gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_CONN_STS;
			
			if (gprs_retry_count++ >= RETRY_CNT)
			{
				gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_CONN_STS;
				sts = CON_FAIL;
				
				gprs_connct_state = GPRS_CONNCT_CMD_CCLK;			//
				gprs_retry_count = 0;
			}
			break;
			case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
			UWriteString((char *)"CONN_STS:k", UART_PC);
#endif
			//We have passed all steps so lets make status ok
			sts = CON_OK;
			gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
			
			gprs_retry_count = 0;
			timeout = 0;
			break;
			case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
			
#endif
			if(timeout++ >= SAPBR_STS_TIMEOUT)
			{
				timeout = 0;
				gprs_connct_state = GPRS_CONNCT_CMD_SAPBR_CONN_STS;
				
				if (gprs_retry_count++ >= RETRY_CNT)
				{
					gprs.server_status = NOT_AVBL;
					
					gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_SAPBR_CONN_STS;
					sts = CON_FAIL;
					
					gprs_connct_state = GPRS_CONNCT_CMD_CCLK;			//
					gprs_retry_count = 0;
				}
			}
			break;
			
			default:
			break;
		}
		break;
#endif	//HTTP_ENABLE

		default:
		break;
	}

	return sts;
}


#ifdef HTTP_ENABLE
http_status_t http_connect (void)
{
	static gprs_http_state_t http_state = GPRS_HTTP_CMD_INIT;
	static char http_retry_cnt = 0;
	
	http_status_t sts = HTTP_IN_PRG;
	static unsigned int timeout = 0;			//Can be used with all commands
	
	switch(http_state)
	{
		case GPRS_HTTP_IDLE:
		break;
		case GPRS_HTTP_CMD_INIT:
#ifdef GPRS_DEBUG
			UWriteString("cmd: INIT", UART_PC);
#endif			
			flushRxBuffer(GPRS_UART);
			UWriteString((char *)"AT+HTTPINIT\r", GPRS_UART);
			http_state = GPRS_HTTP_RSP_INIT;
		break;
		case GPRS_HTTP_RSP_INIT:
			switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
					UWriteString("INIT:f", UART_PC);
#endif				
					http_state = GPRS_HTTP_CMD_INIT;
				
					if (http_retry_cnt++ >= HTTP_RETRY_CNT)
					{
						http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_INIT;
						sts = HTTP_FAIL;
						
						http_state = GPRS_HTTP_CMD_INIT;			//
						http_retry_cnt = 0;
					}
					flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
				break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
					UWriteString("INIT:k", UART_PC);
#endif					  
					
					http_retry_cnt = 0;
					timeout = 0;
				break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
					UWriteString("INIT:w", UART_PC);
#endif					
					if(timeout++ >= HTTPINIT_TIMEOUT)
					{
						timeout = 0;
						http_state = GPRS_HTTP_CMD_INIT;
					
						if (http_retry_cnt++ >= HTTP_RETRY_CNT)
						{							
							http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_INIT;
							sts = HTTP_FAIL;
							
							http_state = GPRS_HTTP_CMD_INIT;			//
							http_retry_cnt = 0;
						}
						flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
					}
				break;
				default:
				break;
			}
		break;


		default:
		break;
	}
	return sts;
}
http_status_t post_request(char *url_str, char *server_response)
{
	static gprs_http_state_t http_state = GPRS_HTTP_CMD_URL;
	static char http_retry_cnt = 0;
	int num_byte = 0;
	
	http_status_t sts = HTTP_IN_PRG;
	static unsigned int timeout = 0;
	static unsigned char gprs_sync_retry_cnt = 0;		// 15-2-19 VC: 
#ifdef ONLINE_CONFIG_EN
	static unsigned char gprs_cfg_retry_cnt = 0;
#endif
	switch (http_state)
	{
		case GPRS_HTTP_CMD_URL:
		{
			{
#ifdef GPRS_DEBUG_URL
				UWriteString((char*) url_str, UART_PC);
#endif
				flushRxBuffer(GPRS_UART);
				
				UWriteString((char*) url_str, GPRS_UART);		//Anand 27-04-16
			}
			
			http_state = GPRS_HTTP_RSP_URL;
		}
		break;
		case GPRS_HTTP_RSP_URL:
			switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
					UWriteString("URL:f", UART_PC);
#endif				
					http_state = GPRS_HTTP_CMD_URL;
				
					if (http_retry_cnt++ >= HTTP_RETRY_CNT)
					{
						http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_URL;
						sts = HTTP_FAIL;
						http_state = GPRS_HTTP_CMD_URL;
						
						http_retry_cnt = 0;
					}
					
				break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG_URL
					UWriteString("URL:k", UART_PC);
#endif					
					http_state = GPRS_HTTP_CMD_POST;//GPRS_HTTP_CMD_DATA_LEN;
					flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
					
					http_retry_cnt = 0;
					timeout = 0;
				break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
					UWriteString("URL:w", UART_PC);
#endif					
					if(timeout++ >= HTTPURL_TIMEOUT)
					{
						timeout = 0;
						http_state = GPRS_HTTP_CMD_URL;
					
						if (http_retry_cnt++ >= HTTP_RETRY_CNT)
						{							
							http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_URL;
							sts = HTTP_FAIL;
							http_state = GPRS_HTTP_CMD_URL;			//
							http_retry_cnt = 0;
						}
						flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
					}
				break;
				default:
				break;
			}
		break;
		case GPRS_HTTP_CMD_POST:
#ifdef GPRS_DEBUG_URL
			UWriteString("cmd: POST", UART_PC);
#endif			
			flushRxBuffer(GPRS_UART);
			UWriteString((char *)"AT+HTTPACTION=0\r", GPRS_UART);
			http_state = GPRS_HTTP_RSP_POST;
		break;
		case GPRS_HTTP_RSP_POST:
			switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG_URL
					UWriteString("POST:f", UART_PC);
#endif				
					http_state = GPRS_HTTP_CMD_POST;
				
					if (http_retry_cnt++ >= HTTP_RETRY_CNT)
					{
						http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_POST;
						sts = HTTP_FAIL;
						
						http_state = GPRS_HTTP_CMD_URL;			//
						http_retry_cnt = 0;
					}
					flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
				break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG_URL
					UWriteString("POST:k", UART_PC);
#endif					
					http_state = GPRS_HTTP_RSP_1_POST;
					flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
					
					http_retry_cnt = 0;
					timeout = 0;
				break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG_URL
					UWriteString("POST:w", UART_PC);
#endif					
					if(timeout++ >= HTTPPOST_TIMEOUT)
					{
						timeout = 0;
						http_state = GPRS_HTTP_CMD_POST;
					
						if (http_retry_cnt++ >= HTTP_RETRY_CNT)
						{							
							http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_POST;
							sts = HTTP_FAIL;
							
							http_state = GPRS_HTTP_CMD_URL;			//
							http_retry_cnt = 0;
						}
						flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
					}
				break;
				default:
				break;
			}
		break;
		case GPRS_HTTP_RSP_1_POST:
		{
			char tmpstr[GPRS_RX_BUFFER_MAX];
			
			// NOTE : CAUTION WHY IT WAS WORKING WITH SIM900 WITHOUT SPADE AFTER COLON ????
			char resp = check_string("+HTTPACTION: 0,", tmpstr, &num_byte);		// For Sim800
			int http_resp = 0;
			
			switch (resp)
			{
				case (GPRS_MATCH_FAIL): 
#ifdef GPRS_DEBUG
					UWriteString("ACT:f", UART_PC);
#endif					
					http_state = GPRS_HTTP_CMD_POST;
					http_retry_cnt++;
#ifdef ONLINE_CONFIG_EN
					if((online_cfg.cfg_pending)	&&	(http_retry_cnt >= HTTP_RETRY_CNT)	)
					{
						online_cfg.cfg_pending = FALSE;
					}
#endif
					if (http_retry_cnt >= HTTP_RETRY_CNT) 
					{
						http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_1_POST;
						sts = HTTP_FAIL;
						
						http_state = GPRS_HTTP_CMD_URL;
						http_retry_cnt = 0;
					}
					flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
					break;
				case (GPRS_MATCH_OK):
					{
#ifdef GPRS_DEBUG_URL
						UWriteString("ACT:k", UART_PC);
						UWriteString(tmpstr, UART_PC);
#endif						

						if((http_resp == HTTP_RESP_SUCCESS)	|| (http_resp == 201))	//29-1-19 VC: need to check for state
						{
#ifdef GPRS_DEBUG_URL
							UWriteString("URL:S", UART_PC);
#endif
							if((getRtuState() == RTU_GPRS_CONFIG) || (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)) 
							{
								// 15-2-19 VC: need to check for retry process
								sts = HTTP_IN_PRG;
								http_state = GPRS_HTTP_CMD_READ;									
							}
							else		// 15-2-19 VC: case of dispense and refuel data uploading to server 							
							{
#ifdef ONLINE_CONFIG_EN
								
								{
									sts = HTTP_IN_PRG;
									http_state = GPRS_HTTP_CMD_READ;
								}
								
#else
								{
									sts = HTTP_PASS;
									http_state = GPRS_HTTP_CMD_URL;
								}
#endif							
								

							}
						}
						else
						{
							if(((getRtuState() == RTU_GPRS_CONFIG)|| (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)) && (gprs_sync_retry_cnt++ >= GPRS_SYNC_RETRY_CNT))		// 15-2-19 VC: need to check for network
							{
								gprs_sync_retry_cnt = 0;
								
							}
#ifdef ONLINE_CONFIG_EN
							else if((online_cfg.cfg_pending)	&&	(gprs_cfg_retry_cnt++ >= GPRS_CFG_RETRY_CNT)	)
							{
								online_cfg.cfg_pending = FALSE;
								gprs_cfg_retry_cnt = 0;
							}
#endif
#ifdef GPRS_DEBUG_URL
							UWriteString("URL:F", UART_PC);
#endif
							sts = HTTP_FAIL;
							http_state = GPRS_HTTP_CMD_URL;	
						}

						flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
						http_retry_cnt = 0;
						timeout = 0;		//Anand -28-04-16
					}
					break;
				case (GPRS_NO_NEW_MSG): 
#ifdef GPRS_DEBUG
					
#endif					
					if(timeout++ >= HTTPPOST_1_TIMEOUT)
					{
						timeout = 0;
						
						http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_1_POST;
						sts = HTTP_FAIL;
						
						http_state = GPRS_HTTP_CMD_URL;			//
						http_retry_cnt = 0;
					
						flushRxBuffer(GPRS_UART);	
					}
					break;
				default:
					break;
			}
		}
		break;
		case GPRS_HTTP_CMD_READ:
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: READ", UART_PC);
#endif			
			UWriteString((char *)"AT+HTTPREAD\r", GPRS_UART);
			http_state = GPRS_HTTP_RSP_READ;
		break;
		case GPRS_HTTP_RSP_READ:
		{
			
			char tmpstr[GPRS_RX_READ_BUFFER_MAX];
			char resp = check_string("+HTTPREAD: ", tmpstr, &num_byte);	
			char rsp = TRUE;

			switch (resp)
			{
				case (GPRS_MATCH_FAIL): 
					
					http_retry_cnt++;
#ifdef ONLINE_CONFIG_EN
					if((online_cfg.cfg_pending)	&&	(http_retry_cnt >= HTTP_RETRY_CNT)	)
					{
						online_cfg.cfg_pending = FALSE;
					}
#endif
					if (http_retry_cnt >= HTTP_RETRY_CNT) 
					{
						http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_READ;
						sts = HTTP_FAIL;
						
						http_state = GPRS_HTTP_CMD_URL;
						http_retry_cnt = 0;
					}
					flushRxBuffer(GPRS_UART);				// HJ 15-07-2016  
					break;
				case (GPRS_MATCH_OK):
					{
#ifdef GPRS_DEBUG_URL
						UWriteString("READ:k", UART_PC);
						
#endif
			
					//Lets Decode recieve data from server
						if(getRtuState() == RTU_GPRS_CONFIG)
						{

							releaseCnfgState();
							flushRxBuffer(GPRS_UART);				// 12-2-19 VC: when operation is done then delet the current buff
							sts = HTTP_PASS;						// gives ok for response to server		
						
							http_state = GPRS_HTTP_CMD_URL;		//Anand 29-03-16 Need Something to do here
							http_retry_cnt = 0;
							timeout = 0;		//Anand -28-04-16
						}
						else if (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)
						{
							rsp = update_flash_hexfile(tmpstr);
							if(rsp == TRUE)								// 4-2-19 VC: think thats required or not
							{
								flushRxBuffer(GPRS_UART);				// 12-2-19 VC: when operation is done then delet the current buff
								sts = HTTP_PASS;						// gives ok for response to server
							}
							else
							{
								
								flushRxBuffer(GPRS_UART);
								sts = HTTP_FAIL;
							
								#ifdef GPRS_DEBUG_URL
								UWriteString((char *)"READ:f", UART_PC);
								
								#endif
							}
							http_state = GPRS_HTTP_CMD_URL;		// Need Something to do here
							http_retry_cnt = 0;
							timeout = 0;
						}
						else
						{
#ifdef ONLINE_CONFIG_EN
							decode_online_resp(tmpstr);
							sts = HTTP_PASS;
							http_state = GPRS_HTTP_CMD_URL;
#else
#endif						
							if(rsp == TRUE)
							{
								http_retry_cnt = 0;
								flushRxBuffer(GPRS_UART);
								sts = HTTP_PASS;
							}
							else
							{
								
#ifdef ONLINE_CONFIG_EN
								if((online_cfg.cfg_pending)	&&	(http_retry_cnt++ >= HTTP_RETRY_CNT)	)
								{
									online_cfg.cfg_pending = FALSE;
								}
#endif
								
								flushRxBuffer(GPRS_UART);
								sts = HTTP_FAIL;
								
#ifdef GPRS_DEBUG_URL
								UWriteString((char *)"READ:f", UART_PC);
								
#endif
							}
							
							http_state = GPRS_HTTP_CMD_URL;
							http_retry_cnt = 0;
							timeout = 0;

						}
					}
				break;
				case (GPRS_NO_NEW_MSG): 
#ifdef GPRS_DEBUG
					UWriteString("READ:w", UART_PC);
#endif					
					if(timeout++ >= HTTPREAD_TIMEOUT)
					{
						timeout = 0;
					
					http_retry_cnt++;
#ifdef ONLINE_CONFIG_EN
						if((online_cfg.cfg_pending)	&&	(http_retry_cnt >= HTTP_RETRY_CNT)	)
						{
							online_cfg.cfg_pending = FALSE;
						}
#endif
						if (http_retry_cnt >= HTTP_RETRY_CNT)
						{							
							http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_READ;
							sts = HTTP_FAIL;
							
							http_state = GPRS_HTTP_CMD_URL;			//
							http_retry_cnt = 0;
						}
						flushRxBuffer(GPRS_UART);	
					}
					break;
				default:
					break;
			}
		}
		break;
		default:
		break;
	}
	return sts;
}
http_status_t http_close(void)
{
	static gprs_http_state_t http_state = GPRS_HTTP_CMD_CLOSE;
	static char http_retry_cnt = 0;
	
	http_status_t sts = HTTP_IN_PRG;
	static unsigned int timeout = 0;
	
	switch(http_state)
	{
		case GPRS_HTTP_CMD_CLOSE:
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CLOSE", UART_PC);
#endif			
			UWriteString((char *)"AT+HTTPTERM\r", GPRS_UART);
			http_state = GPRS_HTTP_RSP_CLOSE;			
		break;
		case GPRS_HTTP_RSP_CLOSE:
			switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
#ifdef GPRS_DEBUG
					UWriteString("CLOSE:f", UART_PC);
#endif				
					http_state = GPRS_HTTP_CMD_CLOSE;
				
					if (http_retry_cnt++ >= HTTP_RETRY_CNT)
					{
						http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_CLOSE;
						sts = HTTP_FAIL;
						
						http_state = GPRS_HTTP_CMD_CLOSE;			//
						http_retry_cnt = 0;
					}
					flushRxBuffer(GPRS_UART);	
				break;
				case (GPRS_MATCH_OK):
#ifdef GPRS_DEBUG
					UWriteString("CLOSE:k", UART_PC);
#endif					
					http_state = GPRS_HTTP_CMD_CLOSE;		//Anand
					sts = HTTP_PASS;
					flushRxBuffer(GPRS_UART);	
					http_retry_cnt = 0;
					timeout = 0;
				break;
				case (GPRS_NO_NEW_MSG):
#ifdef GPRS_DEBUG
					UWriteString("CLOSE:w", UART_PC);
#endif					
					if(timeout++ >= HTTPINIT_TIMEOUT)
					{
						timeout = 0;
						http_state = GPRS_HTTP_CMD_CLOSE;
					
						if (http_retry_cnt++ >= HTTP_RETRY_CNT)
						{							
							http.errcode = HTTP_ERR_OFFSET + GPRS_HTTP_RSP_CLOSE;
							sts = HTTP_FAIL;
							
							http_state = GPRS_HTTP_CMD_CLOSE;			//
							http_retry_cnt = 0;
						}
						flushRxBuffer(GPRS_UART);	
					}
				break;
				default:
				break;
			}
			default:
			break;
	}
	return sts;
}

void http_handler(void)
{

	char status = 0;
	
	char fms_server_resp_buff[2];	// use #define for size of buffer
	static char http_buff[GPRS_TX_BUFFER_MAX];
	static gprs_upload_data_pkt_id_t upload_code = GPRS_NO_DATA;
	static unsigned char upload_retry_cnt = 0;
	static unsigned char http_conn_sts = FALSE;
	switch(gprs.state)
	{
		case GPRS_IDLE:
			if(getGPRSConfigErrSts())			// 22-2-19 VC: after reading all config parameter
			{
				gprs.state = GPRS_CONNECT;		
#ifdef GPRS_DEBUG
				
#endif
			}
			else
			{
				
#ifdef GPRS_DEBUG
				UWriteString("CON:F", UART_PC);
#endif
			}
		break;
		
		case GPRS_CONNECT:
#ifdef GPRS_DEBUG
			
#endif
			status = (char) gprs_connect();				//Connect to sim module and service provider
			if(status == CON_OK)
			{
				
				gprs.state = GPRS_CONN_STS;
			}
			else if(status == CON_FAIL)
			{
				gprs.state = GPRS_IDLE;		//Lets go to idle and wait for trigger to re do
				
				setGPRSConnSts(NOT_AVBL);
			}
			else if (status == CON_IN_PRG)
			{
				
			}
		break;
		case GPRS_CONN_STS:
			status = (char)gprs_connect_status();
			if(status == CON_OK)
			{
				setGPRSConnSts(AVBL);
				if (check_unsent_log() || (getRtuState() == RTU_GPRS_CONFIG)  || (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)) // 29-1-2019 VC: added for gprs sync
				{
					gprs.state = GPRS_HTTP_INIT;
				}
				else
				{
					gprs.state = GPRS_SESSION_IDLE;					
				}
			}
			else if(status == CON_FAIL)
			{
				setGPRSConnSts(NOT_AVBL);
				gprs.state = GPRS_IDLE;		//Lets go to idle and wait for trigger to re do
			}
			else if (status == CON_IN_PRG)
			{
				
			}
		break;
		case GPRS_HTTP_INIT:
			status = (char)http_connect();				// Establish HTTP Connection
			if(status == HTTP_PASS)
			{
				http_conn_sts = TRUE;
				setGPRSSrvrSts(AVBL);
				gprs.state = GPRS_CONNECT_FMS_SERVER;			
			}
			else if(status == HTTP_FAIL)
			{
				//Lets go to idle and wait for trigger to re do
				
				gprs.state = GPRS_HTTP_CLOSE;
			}
			else if (status == HTTP_IN_PRG)
			{
				
			}
		break;
		case GPRS_CONNECT_FMS_SERVER:

		break;
		case GPRS_PREPARE_LOG:
			if((!upload_code) || (getRtuState() == RTU_GPRS_CONFIG) || (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)) // 2-2-19 VC: adding for testing and need to think
			{
				upload_code=controlRTUGprsUpload();
				memset(http_buff, 0, GPRS_TX_BUFFER_MAX);
				rtuPrepareGPRSData(http_buff, upload_code);
#ifdef GPRS_DEBUG
				UWriteString((char *)"uploadcode=", UART_PC);
				UWriteInt(upload_code, UART_PC);
#endif

			}
			if (upload_code)
			{
				gprs.state = GPRS_LOGS_UPLOAD;
				
			}
			else
			{
				// close the http connection
				
				gprs.state = GPRS_SESSION_IDLE;
			}
		break;
		case GPRS_LOGS_UPLOAD:
			if (upload_code)
			{

				status = (char)post_request(http_buff, fms_server_resp_buff);
				
				if (status == HTTP_PASS)
				{
					   if (upload_code == GPRS_RTU_DATA_FLASH)
						decrement_unsent_log_cnt(upload_code);	
					
					upload_retry_cnt = 0;
					gprs.state = GPRS_PREPARE_LOG;
					if(upload_code == GPRS_RTU_DATA_RAM	||	upload_code == GPRS_RTU_DATA_FLASH)
					{
						
						if(db_save_status)
						{
							db_save_status = FALSE;
							
							if(ram_data.status&(1 << POWER_ON_BIT))
							{
								ram_data.status &= ~(1 << POWER_ON_BIT);
							}
							
						}
						if(upload_code == GPRS_RTU_DATA_FLASH)
						{

							if(freq_updated_data.unsent_rtu_logs_GPRS != 0)
							{
								{
									db_save_status = TRUE;
								}
							}
						}
					}

					upload_code = GPRS_NO_DATA;
					set_live_pckt_ready(FALSE);
#ifdef GPRS_DEBUG
					
#endif			
				}
				else if (status == HTTP_FAIL)
				{
					gprs.state = GPRS_PREPARE_LOG;		//need to think about this  HJ 15-7-2016
					upload_code = GPRS_NO_DATA;
					upload_retry_cnt++;
					if (upload_retry_cnt >= UPLOAD_RETRY_CNT)
					{
						upload_retry_cnt = 0;
						
						setGPRSSrvrSts(NOT_AVBL);
						gprs.state = GPRS_HTTP_CLOSE;
						set_live_pckt_ready(FALSE);
						if(get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)	//01-08-2020 SK:
						{
							set_hex_file_updt_state(CONN_ERROR);
							set_flash_file_rqst_type(HEX_NO_RQST);
							
							
						}
					}
				}
				else                     //to handle in HTTP_IN_PROGRSS
				{
#ifdef GPRS_DEBUG
					
#endif
				}
			}
			else
			{
				upload_retry_cnt = 0;
				gprs.state = GPRS_SESSION_IDLE;
			}
		break;

		case GPRS_HTTP_CLOSE:
			status = http_close();
			if (status == HTTP_PASS)
			{
				http_conn_sts = FALSE;
				
				gprs.state = GPRS_CONNECT;
				
#ifdef GPRS_DEBUG
				UWriteString("CON:CLOSE", UART_PC);
#endif
			}
			else if (status == HTTP_FAIL)
			{
				http_conn_sts = FALSE;
				
				gprs.state = GPRS_CONNECT;
			}
			else                     //to handle in HTTP_IN_PROGRSS
			{
			}
		break;
		case GPRS_SESSION_IDLE:
		static unsigned char gprs_conn_retry_time = 0;
#ifdef GPRS_DEBUG

#endif
			
			if(gprs_conn_retry_time++ > GPRS_CONN_RETRY_TIME)
			{
				gprs_conn_retry_time = 0;
				gprs.state = GPRS_CONN_STS;	
#ifdef GPRS_DEBUG

#endif
			}
			// 13-2-19 VC: need to check for gprs_config mode
			if(check_unsent_log() || (getRtuState() == RTU_GPRS_CONFIG) || (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG))		// Check for new dispense or refuel logs
			{
				
				if (getGPRSConnSts() == AVBL)				// HJ 06-09-2017
				{
					
					if (http_conn_sts == TRUE)
					{
						gprs.state = GPRS_PREPARE_LOG;			// HJ 06-09-2017
					}
					else
					{
						
						setGPRSSrvrSts(NOT_AVBL);
						gprs.state = GPRS_CONN_STS;
					}
				}
				else
				{
					gprs.state = GPRS_CONNECT;				// HJ 06-09-2017
					setGPRSSrvrSts(NOT_AVBL);//AM
				}
			}	
		break;
		case GPRS_CONFIG:
			
		break;
		default:
			gprs.state = GPRS_IDLE;
		break;
	}	
}
#endif	//HTTP_ENABLE
int get_rx_data(char *copy_here)
{
	int retval = 0;
	
	if (copy_here)
	{


		{
			memcpy (copy_here, (const void *)gprs_rx_buff.buffer, gprs_rx_buff.index);
			copy_here[gprs_rx_buff.index] = '\0';
					
			retval = gprs_rx_buff.index;
			gprs_rx_buff.index = 0;			
		}
					
	}
	

	return retval;
}
//HJ 29-08-2016
char getGPRSConfigErrSts(void)		//Anand 25.04.2014
{
	return gprs.gprs_config_sts;
}
void setGPRSConfigErrSts(char sts)		//Anand 25.04.2014
{
	gprs.gprs_config_sts = sts;
}
// HJ 31-08-2016
gprs_status_t getGPRSConnSts(void)		//Anand 25.04.2014
{
	return gprs.connect_sts;
}
void setGPRSConnSts(gprs_status_t sts)		//Anand 25.04.2014
{
	gprs.connect_sts = sts;
}
//HJ 17-11-2018
gprs_status_t getGPRSSrvrSts(void)		//Anand 25.04.2014
{
	return gprs.server_status;
}
void setGPRSSrvrSts(gprs_status_t sts)		//Anand 25.04.2014
{
	gprs.server_status = sts;
}

char trylock(char* num)
{
		if (*num != UNLOCKED) {
				return IS_FAILURE;
		}
		*num = LOCKED;
		return IS_SUCCESS;
}
char decode_gprs_response(char *str)
{
	char tmpstr[GPRS_RX_BUFFER_MAX];
	char resp = GPRS_MATCH_FAIL;
	
	resp = check_string_from_data("{\"response\":{", str, tmpstr);
	
	if (resp == GPRS_MATCH_OK)
	{
	}
	else
	{
	}
	return resp;
}
char check_string_from_data(char *cons_str, char *str, char *tmpstr)
{
	
	char retval = GPRS_NO_NEW_MSG;
	unsigned char i, numbytes=0, j, length;
	length = strlen(cons_str);
	if (!tmpstr) {
#ifdef GPRS_DEBUG		//Anand 6-4-16
		
#endif
		retval = IS_FAILURE;
		return retval;
	}
		strcpy(tmpstr, str);
		numbytes = strlen(tmpstr);
#ifdef GPRS_DEBUG		//Anand 6-4-16
		
		if(numbytes)
		{
			UWriteInt(numbytes, UART_PC);
		}
#endif

		if(numbytes)
		{

#ifdef GPRS_DEBUG_URL
			for (i=0; i < numbytes; i++) {
				
				UWriteData(tmpstr[i], UART_PC);		//Anand 06-04-16
			}			
#endif
			for (i=0; i < numbytes; i++) {
				if (!memcmp(&tmpstr[i], cons_str, length)) break;
			}
			

			if(i >= numbytes) {
				return retval = GPRS_MATCH_FAIL;
			}
			
			retval = GPRS_MATCH_OK;
			for(j = 0; j < (numbytes - i - length); j++){
				tmpstr[j] = tmpstr[j + i + length];
			}
			tmpstr[j] = '\0';
		}
	return (retval);
}


void set_imei(char *tmpstr)
{
	memset(&gprs.imei,0,IMEI_NUM_LEN);
	memcpy(&gprs.imei,tmpstr,IMEI_NUM_LEN);
}

char *readImei(void)
{
	return gprs.imei;
}

#ifdef ONLINE_CONFIG_EN
unsigned char decode_online_resp(char *tmpstr)
{
	unsigned char resp = FALSE;

	char temp_buff[100]={0},temp_buff2[100]={0};
	unsigned int index = 0,temp_index = 0,i=0;
	memset(temp_buff, 0, sizeof(temp_buff));
	memset(temp_buff2, 0, sizeof(temp_buff2));
	switch(online_cfg.online_cfg_state)
	{	
		case CFG_VER_CNT:		
#ifdef DEBUG_ONLINE_CFG
			UWriteString((char *)"VER_CNT", UART_PC);
#endif

		while((tmpstr[index] != '"') && (index < GPRS_RX_BUFFER_MAX))
		index++;
		if(index < (GPRS_RX_BUFFER_MAX - 1))
		index++;	//skipping "
		if (tmpstr[index] == '1')
		{
			index++;
			if(index < (GPRS_TEMP_RX_BUFFER_MAX - 1))
			index++;	//skipping ,
			
			memset(&temp_buff,0,sizeof(temp_buff));
			
			temp_index = index;
			for(i=0;tmpstr[temp_index] != '"';i++,temp_index++)
			{
			}
			memcpy(temp_buff, &tmpstr[index], i);
	

			online_cfg.ver_cnt = atoi(temp_buff);
			
			if(online_cfg.ver_cnt>=1	&&	online_cfg.ver_cnt<=99)//valid response
			{
#ifdef DEBUG_ONLINE_CFG
				UWriteString((char *)"\r\ntmpstr:", UART_PC);
				UWriteString(tmpstr, UART_PC);
				UWriteString((char *)"\r\ntemp_buff:", UART_PC);
				UWriteString(temp_buff, UART_PC);
				UWriteString((char *)"\r\nver_cnt_from_srvr:", UART_PC);
				UWriteInt(online_cfg.ver_cnt, UART_PC);
				UWriteString((char *)"\r\ncurr_ver_cnt:", UART_PC);
				UWriteInt(get_online_cfg_ver_cnt(), UART_PC);
#endif
				if(online_cfg.ver_cnt == get_online_cfg_ver_cnt())
				{
					online_cfg.cfg_pending = FALSE;
#ifdef DEBUG_ONLINE_CFG
					UWriteString((char *)"\r\ncnfg_uptodate\r\n", UART_PC);
#endif
				}
				else
				{
					set_online_cfg_ver_cnt(online_cfg.ver_cnt);
					
					online_cfg.cfg_pending = TRUE;
					online_cfg.online_cfg_state = CFG_DW_DATA;
#ifdef DEBUG_ONLINE_CFG
					UWriteString((char *)"ver_not_match", UART_PC);
#endif
				}
				resp= TRUE;
			}
			else
			{
				//No response or invalid response	
#ifdef DEBUG_ONLINE_CFG
				UWriteString((char *)"invalid vercnt", UART_PC);
#endif
				online_cfg.cfg_pending = FALSE;
				resp = FALSE;
			}
			
		}
		else if (tmpstr[index] == '0')
		{
#ifdef DEBUG_ONLINE_CFG
			UWriteString((char *)"Ver_not_avail:", UART_PC);
#endif			
			//Ver cnt not available on server
			online_cfg.cfg_pending = FALSE;
			online_cfg.online_cfg_state = CFG_VER_CNT;
			resp = FALSE;
		}
		else
		{
#ifdef DEBUG_ONLINE_CFG
			UWriteString((char *)"ver_err_und:", UART_PC);
#endif
			
			resp = FALSE;
		}
		
		break;
		
		case CFG_DW_DATA:
		
		memset(temp_buff, 0, sizeof(temp_buff));



#ifdef DEBUG_ONLINE_CFG
		UWriteString((char *)"tmpstr:", UART_PC);
		UWriteString(tmpstr, UART_PC);
#endif
		while((tmpstr[index] != '"') && (index < GPRS_RX_BUFFER_MAX))
		index++;
		if(index < (GPRS_RX_BUFFER_MAX - 1))
		index++;	//skipping "
		if (tmpstr[index] == '1')
		{
			index++;
			if(index < (GPRS_TEMP_RX_BUFFER_MAX - 1))
			index++;	//skipping ,
			memcpy(temp_buff, &tmpstr[index], CONFIG_RESP_LENGTH);
			if(syncOnlineCnfgData(temp_buff))
			{
				resp = TRUE;
				online_cfg.cfg_pending = FALSE;
				online_cfg.online_cfg_state = CFG_VER_CNT;
			}
			else
			{
				resp = FALSE;
			}
			
		}
		else if (tmpstr[index] == '0')
		{
#ifdef DEBUG_ONLINE_CFG
			UWriteString((char *)"config_not_avail:", UART_PC);
#endif
			//No response or invalid response
			online_cfg.cfg_pending = FALSE;
			online_cfg.online_cfg_state = CFG_VER_CNT;
			resp = FALSE;
		}
		else
		{
#ifdef DEBUG_ONLINE_CFG
			UWriteString((char *)"cnfg_data_und:", UART_PC);
#endif
			//unexpected response
			online_cfg.cfg_pending = FALSE;
			online_cfg.online_cfg_state = CFG_VER_CNT;
			resp = FALSE;
		}
			
		break;
		default:
		break;
	}
	return resp;
}
#endif

#endif /* GPRS_ENABLE */
