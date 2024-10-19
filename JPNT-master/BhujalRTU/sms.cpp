/*
 * sms.cpp
 *
 * Created: 09-02-2018 12:43:09 PM
 *  Author: OWNER
 */ 
#include "sms.h"
#include "uart.h"
#include "app_uart.h"
#include "common.h"
#include "gprs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "_debug.h"
#include "always.h"
#include "app_eeprom.h"
#include "jpnt_rtu.h"
#include "flash.h"
//#include "gps.h"
#include "system_sts.h"
#include "sensor.h"
#include "sys_info.h"

sms_data_t sms_data;

sms_data_t sms_data_empty = {0};

gsm_cfg_data_t gsm_cfg_data;

extern ram_data_t ram_data;
extern gprs_config_data_t gprs_config_data;

//extern sim_info_t sim_info;

//extern GPRSCfgData_t GPRSCfgData;
extern calbration_data_t calbration_data;
extern unsigned int tcp_data_send_status ;
extern product_info_t product_info;
unsigned char rcv_msg[NUM_OF_CMDS][CMD_LEN] =
{
	"AAD",
	"EAD",
	"CPS",
	"CIP",
	"CPO",
	"CAP",
	"SSR",
	//"SMR",
	"GGL",
	"GCF",
	"FDE",
	"CSN",
	"IMV",
	"CFM",
	"CPC",
	"RCC",
	"PWR",
	"PID",
};

gsm_status_t sms_handler(void)
{
	static gsm_state_t gsm_state = GSM_CMD_TEXT_MODE;
	static char gsm_retry_cnt = 0;
	gsm_status_t sts = GSM_IN_PRG;
	//static rx_sms_state_t rx_sms_state = MSG_AUTH;
	static unsigned int timeout = 0;
	static int num_of_msg = 0;
	static int msg_number = 0;
	int num_byte = 0;
	char cmd[34], len = 0;
	static int sms_handler_timeout = 0;
	static char tmpstr[GSM_RX_BUFFER_MAX];

	//UWriteString("In sms handler",UART_PC);
	switch (gsm_state)
	{
		case GSM_CMD_TEXT_MODE:
		{
#ifdef GSM_DEBUG
			UWriteString("Tmode",UART_PC);
#endif			
			//UWriteString("AT+CMGF=1\r", GPRS_UART);
			sprintf(cmd,"AT+CMGF=1\r"); // VC:20/08/2018
			len = strlen(cmd);          // VC:20/08/2018
			UWriteBytes(cmd,len,GPRS_UART); // VC:20/08/2018
			gsm_state = GSM_RSP_TEXT_MODE;
		}
		break;
		case GSM_RSP_TEXT_MODE:
		{
			if(check_string_nobuf("OK"))
			{
#ifdef GSM_DEBUG
	UWriteString("Tmode:K", UART_PC);
#endif				
				flushRxBuffer(GPRS_UART);
				gsm_state = GSM_CMD_CNMI;
			}
			else
			{
#ifdef GSM_DEBUG
	UWriteString("Tmode:NK", UART_PC);
#endif
				flushRxBuffer(GPRS_UART);
				gsm_state = GSM_CMD_TEXT_MODE;
			}
		}
		break;
		case GSM_CMD_CNMI: // geting the index of the msg
		{
#ifdef GSM_DEBUG
	UWriteString("cnmi", UART_PC);
#endif
			//UWriteString("AT+CNMI=0,0,0,0,0\r", GPRS_UART);
			sprintf(cmd,"AT+CNMI=0,0,0,0,0\r"); // VC:20/08/2018
			len =  strlen(cmd);					// VC:20/08/2018	 
			UWriteBytes(cmd,len,GPRS_UART);		// VC:20/08/2018
			//to do: add something for fail
			gsm_state = GSM_RSP_CNMI;
		}
		break;
		case GSM_RSP_CNMI:
		{
			if(check_string_nobuf("OK"))
			{
#ifdef GSM_DEBUG
	UWriteString("cnmi:K", UART_PC);
#endif
				flushRxBuffer(GPRS_UART);
				gsm_state = GSM_CMD_CMGD;//GSM_CMD_CPMS; // VC:25/08/2018 first we go for delet all the pending msg
			}
			else
			{
#ifdef GSM_DEBUG_SHOW_FAILS
	UWriteString("cnmi:NK", UART_PC);
#endif
				flushRxBuffer(GPRS_UART);
				gsm_state = GSM_CMD_CNMI;
			}
		}
		break;
		case GSM_CMD_CPMS:
#ifdef GSM_DEBUG
	UWriteString("cpms", UART_PC);
#endif     
				sms_handler_timeout++;
				flushRxBuffer(GPRS_UART);
				//UWriteString("AT+CPMS=\"SM\"\r", GPRS_UART);
				sprintf(cmd,"AT+CPMS=\"SM\"\r"); //VC:20/08/2018
				len = strlen(cmd);				//VC:20/08/2018	
				UWriteBytes(cmd,len,GPRS_UART);	//VC:20/08/2018
				msg_number = 0;
				num_of_msg = 0;
				gsm_state = GSM_RSP_CPMS;
			
		break;
		case GSM_RSP_CPMS:
		{
		//char tmpstr[GSM_RX_BUFFER_MAX];
			char resp = check_string("+CPMS: ", tmpstr, &num_byte);
			//char http_resp = 0;
			sms_handler_timeout++;
			
			switch (resp)
			{
				case GSM_MATCH_FAIL:
				gsm_state = GSM_CMD_CPMS;
				sts = GSM_PASS;
				gsm_retry_cnt++;
				if (gsm_retry_cnt >= GSM_RETRY_CNT)
				{
			//gps.errcode = GPS_ERR_OFFSET + GPS_RSP_STATUS;
					sts = GSM_FAIL;
					gsm_state = GSM_CMD_CPMS;
					//gps_state = GPS_CMD_POWER;
					gsm_retry_cnt = 0;
					
				}
				//flushRxBuffer(GPRS_UART);				// HJ 15-07-2016
				break;
				case GSM_MATCH_OK:
				{
#ifdef GSM_DEBUG
	UWriteString("=cpms:k\n", UART_PC);
	UWriteInt(sms_handler_timeout, UART_PC);
#endif
					char temp[3];
					int i;
					for(i = 0; tmpstr[i]!=','; i++)
					{
						temp[i] = tmpstr[i];	
					}
					temp[i] = '\0';
					num_of_msg = atoi(temp);
					msg_number = 0;
#ifdef GSM_DEBUG
	UWriteString("num_msg=", UART_PC);
	UWriteData((num_of_msg+'0'), UART_PC);					
#endif					
					if(num_of_msg != 0)
					{
						gsm_state = GSM_CMD_CMGR;
					}
					else
					{
						sts = GSM_PASS;
						gsm_state = GSM_CMD_CPMS;
						if(sms_handler_timeout > GSM_TIMEOUT)
						{
#ifdef GSM_DEBUG 
	UWriteString("GO:",UART_PC);
#endif							
							sms_handler_timeout = 0;
							gsm_state = GSM_CMD_CMGD;
							setRtuState(RTU_NORMAL);
						}
					}
					
					flushRxBuffer(GPRS_UART);				// HJ 15-07-2016		
				}
				break;
				
				default:
				break;
			}
		}
		break;
		case GSM_CMD_CMGR:
#ifdef GSM_DEBUG
	UWriteString("cmgr", UART_PC);
#endif
			char ustr[16];
			msg_number++;
			if(num_of_msg >= msg_number)
			{
				sprintf(ustr,"AT+CMGR=%d\r",msg_number);
				len = strlen(ustr);
				flushRxBuffer(GPRS_UART);
				//UWriteString(ustr, GPRS_UART);
				UWriteBytes(ustr,len,GPRS_UART); // VC: 20/08/2018
#ifdef GSM_DEBUG
	UWriteString("index:",UART_PC);
	UWriteInt(msg_number,UART_PC);
#endif				
				gsm_state = GSM_RSP_CMGR_NUM;
			}
			else
			{
				sts = GSM_PASS;
				gsm_state = GSM_CMD_CPMS;
			}
		break;
		case GSM_RSP_CMGR_NUM:
		{
			//char tmpstr[GSM_RX_BUFFER_MAX];
			memset(tmpstr,0xFF,GSM_RX_BUFFER_MAX);
			char resp = check_string("\"+91", tmpstr, &num_byte);
			if (resp) 
			{
				//UWriteString( tmpstr,UART_PC);
			}
			switch (resp)
			{
				case (GSM_MATCH_FAIL):
#ifdef GSM_DEBUG_SHOW_FAILS
	UWriteString("GSM_cmgr_num:f", UART_PC);
#endif
					gsm_state = GSM_CMD_CPMS;
					//msg_number--;
					gsm_retry_cnt++;
					if (gsm_retry_cnt >= GSM_RETRY_CNT)
					{
						//gps.errcode = GPS_ERR_OFFSET + GPS_RSP_STATUS;
						sts = GSM_FAIL;
						gsm_state = GSM_CMD_CMGD;		// If we got service message, we must delete it
						//gps_state = GPS_CMD_POWER;
					}
					flushRxBuffer(GPRS_UART);
						gsm_retry_cnt = 0;
				break;
				case (GSM_MATCH_OK):
				{
					eepromreadGSMConfig();
#ifdef GSM_DEBUG
	UWriteString("cmgr:k\n",UART_PC);
	//UWriteString(tmpstr, UART_PC);
#endif					
					strcpy(sms_data.sender_number, "+91");
					//strcpy(sms_data.new_number, "+91");
					memcpy (&sms_data.sender_number[3], tmpstr, MOB_NUM_LEN-COUNTRY_CODE_LEN);
					sms_data.sender_number[MOB_NUM_LEN-1]='\0';

#ifdef GSM_DEBUG
	UWriteString("mob \n", UART_PC);
	UWriteString(sms_data.sender_number, UART_PC);
#endif					
					int i;
					for (i=0;i<NUM_OF_ADMIN;i++)
					{
						if (!strncmp((const char*)sms_data.sender_number,(const char *)&gsm_cfg_data.mob_num[i],MOB_NUM_LEN-1))
						{							
							break;
						}
					}
					if(i==NUM_OF_ADMIN)
					{
						gsm_state = GSM_CMD_CMGD; // going for delet the rcv msg
					}
					else if(i==0)
					{
						gsm_state = GSM_MSTR_RSP; // going for master response
					}
					else
					{
						
						if(validatePass(&tmpstr[PASS_INDEX])) // comes for pass 
						{
							gsm_state = GSM_RSP_CMGR_MSG;
					
						}
						else
						{
							gsm_state = GSM_CMD_CMGD; // going for delet the rcv msg
						}						
					}
				}
				break;
				case (GSM_NO_NEW_MSG):
#ifdef GSM_DEBUG_SHOW_FAILS
	//UWriteString("URL:w", UART_PC);
#endif
				if(timeout++ >= GSM_TIMEOUT)
				{
					timeout = 0;
					gsm_state = GSM_CMD_CPMS;
					flushRxBuffer(GPRS_UART);
					sts = GSM_PASS;
				}
				break;
				default:
				break;
			}			
		}
		break;
		case GSM_RSP_CMGR_MSG:
		{
#ifdef GSM_DEBUG
	UWriteString("CMGR:",UART_PC);
#endif			
					memcpy(sms_data.recv_msg_data,&tmpstr[ADMIN_CMD_INDEX],sizeof(tmpstr));	
				
				
#ifdef GSM_DEBUG
	UWriteString("\nmsg rcv:", UART_PC);
	UWriteString(sms_data.recv_msg_data, UART_PC);
#endif
				//gsm_state = GSM_CMD_CMGD;
				if(decode_msg(sms_data.recv_msg_data))
				{					
					timeout = 0;
				}
				else
				{
					sprintf(sms_data.send_msg_data,"Command Error%c",26);
				}
				gsm_state = GSM_RX_MSG_RSP;
		}
		break;
		case GSM_RX_MSG_RSP:
		{
			if (tx_msg() == GSM_PASS)
			{
				gsm_state = GSM_CMD_CMGD;
			}
			sms_handler_timeout = 0; //VC: 25/08/2018 need to clear for one transaction			
		}
		break;
		case GSM_CMD_CMGD:
		{
			//UWriteString("AT+CMGD=1,4\r", GPRS_UART);
			sprintf(cmd,"AT+CMGDA=\"DEL ALL\"\r");
			len = strlen(cmd);
			UWriteBytes(cmd,len,GPRS_UART);
#ifdef GSM_DEBUG
	UWriteString("\nDelete MSG", UART_PC);
	//UWriteString(tmpstr,UART_PC);
#endif			
			gsm_state = GSM_RSP_CMGD;
		}
		break;
		case GSM_RSP_CMGD:
		{
/*
#ifdef GSM_DEBUG
UWriteString("DT:",UART_PC);
#endif*/			
			if(check_string_nobuf("OK"))
			{
#ifdef GSM_DEBUG
	UWriteString("cmgd:K", UART_PC);
#endif
				timeout = 0;
				flushRxBuffer(GPRS_UART);
				gsm_state = GSM_CMD_CPMS;
				sts = GSM_PASS;
				sms_data = sms_data_empty;	//Empty the structure
				//memset(tmpstr,0,sizeof(tmpstr));  // VC: 21/08/2018
				//memset((void *)&sms_data,0,sizeof(sms_data_t));
				// here need to be set RTU in normal Mode.
				//setRtuState(RTU_NORMAL); // VC: 21/08/2018
				
			}
			else
			{
				if(timeout++ >= GSM_TIMEOUT)
				{
					timeout = 0;
					sts = GSM_PASS;
					flushRxBuffer(GPRS_UART);
					gsm_state = GSM_CMD_CPMS;
					//memset(tmpstr,0,sizeof(tmpstr));  // VC: 21/08/2018
					//setRtuState(RTU_NORMAL); // VC: 21/08/2018
					
				}
			}
		}
		break;	

	case  GSM_MSTR_RSP:
	{
		//memcpy((void *)sms_data.recv_msg_data,(const void*)&tmpstr[5],sizeof(tmpstr));
		strncpy(sms_data.recv_msg_data,(const char *)&tmpstr[MASTER_CMD_INDEX],RCV_MSG_BUF); 
#ifdef GSM_DEBUG
	UWriteString("\nmsg is", UART_PC);
	UWriteString(sms_data.recv_msg_data, UART_PC);
#endif
		//gsm_state = GSM_CMD_CMGD;
		if(decode_msg(sms_data.recv_msg_data))
		{
			timeout = 0;
		}
		else
		{
			sprintf(sms_data.send_msg_data,"Command Error%c",26);
		}
		gsm_state = GSM_RX_MSG_RSP;
				
		//flushRxBuffer(GPRS_UART);
				
	}
		break;		
		default: gsm_state = GSM_CMD_CMGD;
		break;
	}
	return sts;
}

gsm_status_t tx_msg(void)
{
	static char tx_state = 	TX_MSG_NUM;
	static int timeout = 0;
	gsm_status_t sts = GSM_IN_PRG;
	char len;
	switch(tx_state)
	{
		case TX_MSG_NUM:
		{
			char tmpstr[GSM_RX_BUFFER_MAX];
			flushRxBuffer(GPRS_UART);
			sprintf(tmpstr,"AT+CMGS=\"%s\"\r",sms_data.sender_number);
			len = strlen(tmpstr);
			//UWriteString(tmpstr,GPRS_UART);
			UWriteBytes(tmpstr,len,GPRS_UART);
			//UWriteString("AT+CMGS=",GPRS_UART);
#ifdef GSM_DEBUG
			UWriteString("tx1:",UART_PC);
			UWriteString(tmpstr,UART_PC);
#endif			
			timeout = 0;
			tx_state = TX_MSG_NUM_RSP;
		}
		break;
		case TX_MSG_NUM_RSP:
		{
			if(check_string_nobuf(">"))
			{
#ifdef GSM_DEBUG
				UWriteString("cmgs:K", UART_PC);
#endif
				tx_state = TX_MSG_DATA;
			}
			else
			{
#ifdef GSM_DEBUG
				UWriteString("cmgs:NK", UART_PC);
#endif
				//tx_state = TX_MSG_NUM;
				if(timeout++ >= GSM_TIMEOUT)
				{
					timeout = 0;
					sts = GSM_PASS;
					tx_state = TX_MSG_NUM;
				}				
			}
		}
		break;
		case TX_MSG_DATA:
		{
			//char tmpstr[GSM_RX_BUFFER_MAX];
			flushRxBuffer(GPRS_UART);
			//sprintf(tmpstr,"login successful%c",26);
			//UWriteString(sms_data.send_msg_data,GPRS_UART); // VC:21/08/2018
			len = strlen(sms_data.send_msg_data);
			UWriteBytes(sms_data.send_msg_data,len,GPRS_UART);
			//UWriteData(0x1A,GPRS_UART);	//26
#ifdef GSM_DEBUG
			UWriteString("send_data:",UART_PC);
			UWriteString(sms_data.send_msg_data,UART_PC);
#endif			
			timeout = 0;
			tx_state = TX_MSG_DATA_RSP;
		}
		break;
		case TX_MSG_DATA_RSP:
		{
			if(check_string_nobuf("+CMGS"))
			{
#ifdef GSM_DEBUG
				UWriteString("msg sent:K", UART_PC);
#endif
				tx_state = TX_MSG_NUM;
				sts = GSM_PASS;
			}
			else
			{
#ifdef GSM_DEBUG
				UWriteString("msg sent:NK", UART_PC);
#endif
				if(timeout++ >= GSM_MSG_ACK_TIMEOUT)
				{
					timeout = 0;
					sts = GSM_PASS;
					tx_state = TX_MSG_NUM;
				}
				//tx_state = TX_MSG_NUM;
			}
		}
		break;
	}

	return sts;
}

int decode_msg(char *msg)
{
	int cmd, ret_val = 0;
	unsigned int cnt = 0, loop_cnt;
	
	char cal_flow[5], cal_pres[5], product_id[4];
	memset(sms_data.send_msg_data,0,GSM_RX_BUFFER_MAX);
	
	for (cmd = AAD; cmd < NUM_OF_CMDS ; cmd++)
	{
		if (!(strncmp((const char *)msg,(const char *)&rcv_msg[cmd],(CMD_LEN - 1))))
		{
			ret_val = 1;

#ifdef GSM_DEBUG
	UWriteString("cmd",UART_PC);
	UWriteInt(cmd,UART_PC);
	UWriteString("msg:",UART_PC);
	UWriteString(msg,UART_PC);
#endif

			break;
		}

	}



	switch(cmd)
	{
		case AAD:
		case EAD:
#ifdef GSM_DEBUG
	UWriteString("Add/Edit admin",UART_PC);
#endif		
			int admin_num;
			admin_num = msg[SMS_DATA_INDEX] - '0';
			if((admin_num > 0) && (admin_num < NUM_OF_ADMIN))
			{
				memcpy(gsm_cfg_data.mob_num[admin_num],&msg[MOB_NUM_INDEX],MOB_NUM_LEN-1);
				/*memcpy(sms_data.old_number,&msg[6],MOB_NUM_LEN-1);
				sms_data.old_number[MOB_NUM_LEN - 1] = '\0';
				memcpy(sms_data.new_number,gsm_cfg_data.mob_num[admin_num],MOB_NUM_LEN-1);
				sms_data.new_number[MOB_NUM_LEN - 1] = '\0';				*/

				eepromWriteGSMConfig(&gsm_cfg_data);
				sprintf(sms_data.send_msg_data,"Add/Edit Admin %d OK%c",admin_num,26);
#ifdef GSM_DEBUG
	//UWriteInt(admin_num , UART_PC);
	UWriteString("1:",UART_PC);
	UWriteString(sms_data.send_msg_data,UART_PC);
#endif
			}
			else if(admin_num == 0)
			{	
				if (!strncmp((const char*)sms_data.sender_number,(const char *)&gsm_cfg_data.mob_num[admin_num],MOB_NUM_LEN-1))	
				{
					memcpy(gsm_cfg_data.mob_num[admin_num],&msg[MOB_NUM_INDEX],MOB_NUM_LEN-1);
	
					eepromWriteGSMConfig(&gsm_cfg_data);
					sprintf(sms_data.send_msg_data,"Add/Edit Admin %d OK%c",admin_num,26);
#ifdef GSM_DEBUG
	//UWriteInt(admin_num , UART_PC);
	UWriteString("2:",UART_PC);
	UWriteString(sms_data.send_msg_data,UART_PC);
#endif					
				}
				else
				{
					sprintf(sms_data.send_msg_data,"Command error%c",26);
				}
			}
		break;
		
		case CPS:
#ifdef GSM_DEBUG
	UWriteString("change Pass",UART_PC);
#endif	
			memcpy(gsm_cfg_data.pswd,&msg[SMS_DATA_INDEX],PASS_LEN-1);	
			eepromWriteGSMConfig(&gsm_cfg_data);
			sprintf(sms_data.send_msg_data,"Change Password OK%c",26);
		break;
		
		case CIP:
			
#ifdef GSM_DEBUG
	UWriteString("Change IP",UART_PC);
#endif		
			memcpy(gprs_config_data.ip_address,&msg[SMS_DATA_INDEX],IP_LENGTH-1);
			for(loop_cnt = 0; gprs_config_data.ip_address[loop_cnt] != '\0'; loop_cnt++)
			{
				if(gprs_config_data.ip_address[loop_cnt] == '.')
				{
					cnt++;
					continue;
				}				
			}
			if(cnt >=3)
			{
				eepromWriteGPRSConfig(&gprs_config_data);	
			}
			
			sprintf(sms_data.send_msg_data,"Change IP OK%c",26);	
			//update_url();				
		break;
		
		case CPO:
#ifdef GSM_DEBUG
	UWriteString("Change Port Address",UART_PC);
#endif		
			char arr[6];
			int port_indx;
			port_indx = SMS_DATA_INDEX;
			int i;
			for(i=0;i<5;i++)	//	Integer number length for atoi
			{
				if (msg[port_indx] == '#')
				{
					break;
				}				
				arr[i] = msg[port_indx++];
			}
			arr[i] = '\0';
			gprs_config_data.port_address = atoi((const char*)arr);
#ifdef GSM_DEBUG			
	UWriteInt(gprs_config_data.port_address,UART_PC);
#endif
			eepromWriteGPRSConfig(&gprs_config_data);
			sprintf(sms_data.send_msg_data,"Change Port OK%c",26);
			//update_url();
		break; 
		
		case CAP:
#ifdef GSM_DEBUG
	UWriteString("Change APN",UART_PC);
#endif		
			port_indx = SMS_DATA_INDEX;
			for(i=0;i<APN_LEN;i++)	//	Integer number length for atoi
			{
				if (msg[port_indx] == '#')
				{
					break;
				}
				gprs_config_data.apn[i] = msg[port_indx++];
			}
			gprs_config_data.apn[i] = '\0';
			//memcpy(gsm_cfg_data.apn,&msg[4],GPRS_APN_LEN);
			eepromWriteGPRSConfig(&gprs_config_data);
			sprintf(sms_data.send_msg_data,"Change APN OK%c",26);
			//memcpy(GPRSCfgData.apn, gsm_cfg_data.apn, APN_LEN);//(strlen("airtelgprs.com")+1));
		break;
		
		case SSR:
#ifdef GSM_DEBUG
	UWriteString("Set sync Rate",UART_PC);
#endif		
#if 0	//TT
			char mr[4],sr[4];
			int indx;
			indx = SMS_DATA_INDEX;
			for(i=0;i<4;i++)	//	Integer number length for atoi
			{
				if (msg[indx] == '#')
				{
					break;
				}
				mr[i] = msg[indx++];
			}
			mr[i] = '\0';
			++indx;
			for(i=0;i<4;i++)	//	Integer number length for atoi
			{
				if (msg[indx] == '#')
				{
					break;
				}
				sr[i] = msg[indx++];
			}
			sr[i] = '\0';			
		//	gsm_cfg_data.movable_rate = atoi((const char*)mr); // :VC -10/07/2018 need to be check
		//	gsm_cfg_data.static_rate = atoi((const char*)sr); // :VC -10/07/2018 need to be check
#endif
			eepromWriteGSMConfig(&gsm_cfg_data);
			sprintf(sms_data.send_msg_data,"Set sync rate OK%c",26);			
		break;
#if 0
		case GGL:
#ifdef GSM_DEBUG
	UWriteString("Get google link",UART_PC);
#endif					
			char lat[LAT_LEN+1];
			char longi[LONG_LEN+1];
			
			//convet_location(lat, longi);
			get_present_time(&ram_data.time);
			sprintf(sms_data.send_msg_data,"http://maps.google.com/maps?&hl=en&q=%sN,%sE\nSPD=%d\nLVL=%d\nBAT=%dIGN=%d\n20%02d-%02d-%02d\n%02d:%02d:%02d%c", lat, longi, /*get_speed(), get_lvl()*/100,200, 50, 1,
			ram_data.time.yr, ram_data.time.mm, ram_data.time.dd, ram_data.time.hr, ram_data.time.min, ram_data.time.sec,26);
		break;
#endif
		case GCF:
#ifdef GSM_DEBUG
	UWriteString("Get Config",UART_PC);
#endif		
			eepromreadGSMConfig();
			sprintf(sms_data.send_msg_data,"DV-%s/%d\nA1-%s\nA2-%s\nA3-%s\nAPN-%s\n@S-%d\n@M-%d%c",gprs_config_data.ip_address,gprs_config_data.port_address,
			gsm_cfg_data.mob_num[1],gsm_cfg_data.mob_num[2],gsm_cfg_data.mob_num[3],gprs_config_data.apn,/*gsm_cfg_data.static_rate,gsm_cfg_data.movable_rate,*/10,'A',26); // :VC -10/07/2018 need to be check
		break;

		case  FDE:
#ifdef GSM_DEBUG
	UWriteString("Factory Default",UART_PC);
#endif		
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
			writeDefaults();
			//write_default_GSM_config();
			sprintf(sms_data.send_msg_data,"Factory Default OK%c",26);
		break;
		
		case CSN:
#ifdef GSM_DEBUG
			UWriteString("Change SIM Number",UART_PC);
#endif		
// need to update as for new struct 11-12-18
			//eepromreadSIMConfig();
			//memset(&sim_info.sim_no, 0, sizeof(sim_info.sim_no));
			//memcpy(sim_info.sim_no,&msg[SMS_DATA_INDEX],sizeof(sim_info.sim_no));
			//eepromWriteSIMConfig();		//Packet Updated, Now Update the E2P Data
			sprintf(sms_data.send_msg_data,"Change SIM Number OK%c",26);
		break;
		//Hj 26-03-2018
		case IMV:
#ifdef GSM_DEBUG
			UWriteString("ImMobilize Vehicle",UART_PC);
#endif		
		break;

		case CFM: // VC: 22/08/2018 
		
#ifdef GSM_DEBUG
	UWriteString("Calibration for flow",UART_PC);
#endif

			memcpy(cal_flow,&msg[SMS_DATA_INDEX],(sizeof(cal_flow)-1));
			cal_flow [NULL_INDEX] = '\0';
			
			calbration_data.cal_data_flow1 = atoi(cal_flow);
			eepromWriteCalData();
			sprintf(sms_data.send_msg_data,"Flow Calibration Write OK%c",26);
		
		break;
		
		case CPC: // VC: 22/08/2018 
		
#ifdef GSM_DEBUG
	UWriteString("Calibration for pressure",UART_PC);
#endif
			memcpy(cal_pres,&msg[SMS_DATA_INDEX],(sizeof(cal_pres)-1));
			cal_pres [NULL_INDEX] = '\0';
			calbration_data.cal_data_pressure1 = atoi(cal_pres);
			eepromWriteCalData();
			sprintf(sms_data.send_msg_data,"Pressure Calibrtion Write OK%c",26);
		
		break;
	case RCC: // VC: 22/08/2018
			
#ifdef GSM_DEBUG
	UWriteString("Read calibration constant",UART_PC);
#endif
		eepromReadCalData();			
		sprintf(sms_data.send_msg_data,"Pressure = %d,Flow = %d calibration OK and level = %d level OK%c",calbration_data.cal_data_pressure1,calbration_data.cal_data_flow1,calbration_data.cal_data_lvl1,26);
			
	break;

		case PWR: // VC: 22/08/2018
		
#ifdef GSM_DEBUG
	UWriteString("Gprs Reset",UART_PC);
#endif
		tcp_data_send_status = GPRS_HARD_RST_TIME ;
		gprsConnHardRst();
		sprintf(sms_data.send_msg_data,"Gprs Reset OK%c",26);
		
		break;
		
		case PID: // VC: 31/10/2018 supported upto 999 product id
				
#ifdef GSM_DEBUG
	UWriteString("change product id",UART_PC);
#endif
			memcpy(product_id,&msg[SMS_DATA_INDEX],(sizeof(product_id)-1));
			product_id[NULL_INDEX - 1] = '\0';
			product_info.product_id = atoi(product_id);
			eepromWriteProductInfo();			
			sprintf(sms_data.send_msg_data,"Change product ID OK%c",26);
				
		break;


		default:	// Invalid
		#ifdef GSM_DEBUG
		UWriteString("Command error",UART_PC);
		#endif
		sprintf(sms_data.send_msg_data,"Command Error%c",26);
		break;
	}
	return ret_val;
}
int validatePass(char *str)
{
	int ret_val = 0;
	//char tmpstr[GSM_RX_BUFFER_MAX];
	char pass_key[PASS_LEN+2];
	pass_key[0] = '#';
	memcpy(&pass_key[1],gsm_cfg_data.pswd,PASS_LEN-1);
	pass_key[5] = '#';
	pass_key[6] = '\0';
	//char resp = check_string(pass_key, tmpstr, &num_byte); 
	if(!(memcmp(pass_key,str,PASS_LEN)))
	{
		ret_val = 1;
	}
#ifdef GSM_DEBUG
	UWriteString("pass:",UART_PC);
	UWriteString(str,UART_PC);
#endif	
	return ret_val; 
}