#include "gprs_config.h"
#include "common.h"
#include "main.h"
#include "language.h"
#include "config.h"
#ifdef _20x4_LCD
#include "lcd.h"
#endif
#include "sys_info.h"

#include "flash.h"
#include "app_eeprom.h"
#include "_debug.h"
#include "app_uart.h"
#include "app_lcd.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gprs.h"
#include <avr\wdt.h>

extern product_info_t product_info;
extern freq_updated_data_t freq_updated_data;
extern FL_rtu_log_t FL_rtu_log;

extern ram_data_t ram_data;
 
#ifdef GPRS_ENABLE

vehicle_sync_t vehicle_sync; // 11-2-19 VC: struct for sync released data

vehicle_sync_time_t vehicle_sync_time; 

flash_hexfile_t flash_hexfile;

#ifdef ONLINE_CONFIG_EN
e2p_online_cnfg_t e2p_online_cnfg;
extern online_cfg_t online_cfg;
extern product_info_config_t product_info_config;
#endif

#ifdef DEBUG_HEX_FILE_STORE
unsigned char dyn_mem_alloc_cnt = 0;
unsigned char dyn_mem_dealloc_cnt = 0;
#endif

void gprsConfigRoutines()
{
	{
		
		{
			
			{										//28-1-19 VC: add for freeze keyboard for not operate other one
				setRtuState(RTU_GPRS_CONFIG);						// change the mode of operation	
				if(getRtuState() == RTU_GPRS_CONFIG)
				{
					config_screen();
				}

			}
		
		}
	}
}

void syncCnfgData(char *buff)
{
	unsigned int index = 0;
	
	char tempstr[25] = {0};
		
	clearFlashData();	//18-01-2021 SK:why??
	
	memset(&product_info,0,sizeof(product_info_t));
	//memset(&ram_data,0,sizeof(ram_data_t));
	//product_info.industry_id = 0;
	//memset(&product_info.ind_name,0,sizeof(INDUS_NAME_LEN));
	//ram_data.cuml_reading = 0;
	//ram_data.todays_total = 0;
	//product_info.daily_limit = 0;
	//ram_data.months_total = 0;
		
	for (;buff[index] != '\n';index++);				//to ignore unwanted data
	index++;
	memset(&tempstr,0,sizeof(tempstr));						//industry_id
	memcpy(tempstr,&buff[index],4);
	index += 4;

	product_info.industry_id = atoi(tempstr);
	
// 	memset(&tempstr,0,sizeof(tempstr));						//industry_name
// 	memcpy(tempstr,&buff[index],INDUS_NAME_LEN);
	
	memcpy(product_info.ind_name,&buff[index],INDUS_NAME_LEN);
	index += (INDUS_NAME_LEN);
	

	/*for(i = 0 ; tempstr[i] != '*' ; i++)					//industry name whitespaced with '*'
	{
		product_info.ind_name[i] = tempstr[i];
	}*/
	
	memset(&tempstr,0,sizeof(tempstr));						//daily_limit
	memcpy(tempstr,&buff[index],3);
	index += 3;

	product_info.daily_limit = atoi(tempstr);
	
	eepromWriteProductInfo();
	
	/*memset(&tempstr,0,sizeof(tempstr));						//today_total
	memcpy(tempstr,&buff[index],7);
	index += 7;
	index++;		//for decimal point*/

	
	ram_data.todays_total = my_atoul(&buff[index],10,8);
	index += 10;
	ram_data.months_total = my_atoul(&buff[index],10,8);
	index += 10;
	ram_data.cuml_reading = my_atoul(&buff[index],10,8);

	
	/*memset(&tempstr,0,sizeof(tempstr));						//today_total_dp
	memcpy(tempstr,&buff[index],2);
	index += 2;
#ifdef DEBUG_GPRS_SERVER_SYNC
UWriteString(tempstr,UART_PC);
#endif
	ram_data.todays_total += atol(tempstr);
	
	memset(&tempstr,0,sizeof(tempstr));						//months_total
	memcpy(tempstr,&buff[index],7);
	index += 7;
	index++;		//for decimal point
#ifdef DEBUG_GPRS_SERVER_SYNC
UWriteString(tempstr,UART_PC);
#endif
	ram_data.months_total = atol(tempstr) * 100;
	
	memset(&tempstr,0,sizeof(tempstr));						//months_total_dp
	memcpy(tempstr,&buff[index],2);
	index += 2;
#ifdef DEBUG_GPRS_SERVER_SYNC
UWriteString(tempstr,UART_PC);
#endif
	ram_data.months_total += atol(tempstr);
	
	memset(&tempstr,0,sizeof(tempstr));						//cumm_total
	memcpy(tempstr,&buff[index],7);
	index += 7;
	index++;		//for decimal point
#ifdef DEBUG_GPRS_SERVER_SYNC
UWriteString(tempstr,UART_PC);
#endif
	ram_data.cuml_reading = atol(tempstr) * 100;
	
	memset(&tempstr,0,sizeof(tempstr));						//cumm_total_dp
	memcpy(tempstr,&buff[index],2);
	index += 2;
#ifdef DEBUG_GPRS_SERVER_SYNC
UWriteString(tempstr,UART_PC);
#endif
	ram_data.cuml_reading += atol(tempstr);*/
	
#ifdef FLASH_LOG_DATA
	prepare_FL_LogData();
	save_FL_rtu_data();
#else
	prepare_e2p_logdata();
	e2p_write_log_data();
#endif

#ifdef DEBUG_GPRS_SERVER_SYNC
	UWriteString("\nid:",UART_PC);
	UWriteInt(product_info.industry_id,UART_PC);
	UWriteString("\nname:",UART_PC);
	UWriteString((char*)product_info.ind_name,UART_PC);
	UWriteString("\nd_l:",UART_PC);
	UWriteInt(product_info.daily_limit,UART_PC);
	UWriteString("\nt_t:",UART_PC);
	UWriteInt(ram_data.todays_total,UART_PC);
	UWriteString("\nm_t:",UART_PC);
	UWriteInt(ram_data.months_total,UART_PC);
	UWriteString("\nc_t:",UART_PC);
	UWriteInt(ram_data.cuml_reading,UART_PC);
	
#endif

}
#ifdef ONLINE_CONFIG_EN
unsigned char syncOnlineCnfgData(char *buff)
{
	unsigned int index = 0,pid = 0, i = 0;
	unsigned char resp = FALSE;
	char tempstr[25] = {'\0'};
	online_cfg_t online_cfg_data;
	unsigned long temp_daily_lim = 0;
	unsigned char temp_daily_lim_dp = 0;
	//clearFlashData();	//18-01-2021 SK:why??
	
#ifdef DEBUG_ONLINE_CFG
	UWriteString((char *)"buff:", UART_PC);
	UWriteString(buff, UART_PC);
#endif


	//if(strlen(buff)>=CONFIG_RESP_LENGTH)
	{

		//memset(&product_info,0,sizeof(product_info_t));
		//memset(&product_info_config,0,sizeof(product_info_config_t));
		memset(&online_cfg_data,0,sizeof(online_cfg_t));

		memset(&tempstr,'\0',sizeof(char)*strlen(tempstr));
		//memcpy(tempstr,&buff[index],PID_LEN);				//industry_id
		//if(buff[index]>='0' && buff[index]<='9' && buff[index+1]>='0' && buff[index+1]<='9' && buff[index+2]>='0' && buff[index+2]<='9')
		//pid = atoi(tempstr);
		//if(pid>=1	&&	pid<=9999)
		{
			index = i = 0;
			
			while((buff[i] != '.'))
			{
				if(buff[i] == '"')
				{
					break;
				}
				temp_daily_lim *= 10;
				temp_daily_lim += buff[i] - '0';
				i++;
			}
			if(buff[i] != '"')
			{
				i++;
			}
			while(buff[i] != '"')
			{
				temp_daily_lim_dp *= 10;
				temp_daily_lim_dp += buff[i] - '0';
				
				i++;
			}
			online_cfg_data.daily_limit = temp_daily_lim;
			online_cfg_data.daily_limit_dp = temp_daily_lim_dp;
			
			
#ifdef DEBUG_ONLINE_CFG
UWriteString("online_lim",UART_PC);
UWriteInt(product_info.daily_limit,UART_PC);
#endif
			
			/*index += PID_LEN;

			online_cfg_data.industry_id = atoi(tempstr);

 			memset(&tempstr,0,sizeof(tempstr));//industry_name
 			memcpy(tempstr,&buff[index],INDUS_NAME_LEN);
			
			for(i = 0 ; i<INDUS_NAME_LEN && tempstr[i] != '*' ; i++)//industry name whitespaced with '*'
			{
				online_cfg_data.ind_name[i] = tempstr[i];
			}
			if(tempstr[i] == '*')
				online_cfg_data.ind_name[i] = '\0';
			
			index += (INDUS_NAME_LEN);
	
	
			memset(&tempstr,0,sizeof(tempstr));						//daily_limit
			memcpy(tempstr,&buff[index],DAILY_LIMIT_LEN);
			online_cfg_data.daily_limit = atoi(tempstr);
			index += DAILY_LIMIT_LEN;
	
			online_cfg_data.today_total = my_atoul(&buff[index],TOTALIZER_LEN,TOTALIZER_DP);
			index += TOTALIZER_LEN;//with dp
			online_cfg_data.month_total = my_atoul(&buff[index],TOTALIZER_LEN,TOTALIZER_DP);
			index += TOTALIZER_LEN;
			online_cfg_data.cum_total = my_atoul(&buff[index],TOTALIZER_LEN,TOTALIZER_DP);
			index += TOTALIZER_LEN;
	
			memset(&tempstr,0,sizeof(tempstr));
			memcpy(tempstr,&buff[index],TIME_SLOT_LEN);
			online_cfg_data.time_slot = atoi(tempstr);
			index += TIME_SLOT_LEN;
	
			memset(&tempstr,0,sizeof(tempstr));
			memcpy(tempstr,&buff[index],CAL_FAC_LEN_ON_CNFG);
			online_cfg_data.calib_factor = atoi(tempstr);
			index += CAL_FAC_LEN_ON_CNFG;
	
			memset(&tempstr,0,sizeof(tempstr));
			memcpy(tempstr,&buff[index],ADC_OFFSET_LEN);
			online_cfg_data.ADC_offset = atoi(tempstr);
			index += ADC_OFFSET_LEN;
		
			memset(&tempstr,0,sizeof(tempstr));
			memcpy(tempstr,&buff[index],FM_MAKE_LEN);
			online_cfg_data.flow_meter_make = (flow_meter_make_t)atoi(tempstr);
			index += FM_MAKE_LEN;

			memset(&tempstr,0,sizeof(tempstr));
			memcpy(tempstr,&buff[index],CON_TYPE_LEN);
			online_cfg_data.connection_type = (connection_type_t)atoi(tempstr);*/

			//eepromWriteProductInfo();
			//eepromWriteProductInfoConfig();
			//set_online_cfg_ver_cnt(online_cfg.ver_cnt);
			//eeprom_write_online_cfg_cnt();
		
			/*
			online_cfg_data.industry_id    = product_info.industry_id;
			memcpy(online_cfg_data.ind_name,&product_info.ind_name,INDUS_NAME_LEN);
			online_cfg_data.daily_limit    = product_info.daily_limit;
			online_cfg_data.today_total    = ram_data.todays_total;
			online_cfg_data.month_total    = ram_data.months_total;
			online_cfg_data.cum_total      = ram_data.cuml_reading;
			online_cfg_data.calib_factor   = product_info.calib_factor;
			online_cfg_data.ADC_offset     = product_info.ADC_offset;
			online_cfg_data.time_slot      = product_info_config.time_slot;
			online_cfg_data.flow_meter_make= product_info_config.flow_meter_make;
			online_cfg_data.connection_type= product_info_config.connection_type;
			*/
			update_rtu_config(&online_cfg_data,online_cfg.ver_cnt);	
	
#ifdef FLASH_LOG_DATA
			prepare_FL_LogData();
			save_FL_rtu_data();
#else
			prepare_e2p_logdata();
			e2p_write_log_data();
#endif

#ifdef DEBUG_GPRS_SERVER_SYNC
			UWriteString("\r\nid:",UART_PC);
			UWriteInt(product_info.industry_id,UART_PC);
			UWriteString("\r\nname:",UART_PC);
			UWriteString((char*)product_info.ind_name,UART_PC);
			UWriteString("\r\nd_l:",UART_PC);
			UWriteInt(product_info.daily_limit,UART_PC);
			UWriteString("\r\nt_t:",UART_PC);
			UWriteInt(ram_data.todays_total,UART_PC);
			UWriteString("\r\nm_t:",UART_PC);
			UWriteInt(ram_data.months_total,UART_PC);
			UWriteString("\r\nc_t:",UART_PC);
			UWriteInt(ram_data.cuml_reading,UART_PC);
			UWriteString("\r\ntime_slot:",UART_PC);
			UWriteInt(product_info_config.time_slot,UART_PC);
			UWriteString("\r\nCal_Fac:",UART_PC);
			UWriteInt(product_info.calib_factor,UART_PC);
			UWriteString("\r\nADC_ofst:",UART_PC);
			UWriteInt(product_info.ADC_offset,UART_PC);
			UWriteString("\r\nFm_make:",UART_PC);
			UWriteInt(product_info_config.flow_meter_make,UART_PC);
			UWriteString("\r\nContype:",UART_PC);
			UWriteInt(product_info_config.connection_type,UART_PC);
#endif
			resp= TRUE;
		}
		/*else
		{
#ifdef DEBUG_ONLINE_CFG
			UWriteString((char *)"Inv Data:", UART_PC);
#endif
			resp = FALSE;
		}*/
	}
	/*else
	{
#ifdef DEBUG_ONLINE_CFG
		UWriteString((char *)"Insuf len:", UART_PC);
#endif
		resp = FALSE;
	}*/
	return resp;
}
#endif


void clearFlashData()					//02-04-2019 SK:
{
	//clear_logs();
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
	freq_updated_data.unsent_rtu_logs_GPRS = 0;
	
	//freq_updated_data.cnt = 1;
	//flashWriteFreqUpdatedConfig();
	flashClearFreqUpdatedConfig();

	FL_rtu_log.transaction_id_rtu_log = 0;	
	FL_rtu_log.transaction_id_MSB_rtu_log = 0;	
	
	flashClrMRBR();
	updateFlashCurrAddr();
}

unsigned char syncVehicleList(char *buff)
{
	//unsigned int i = 0, j = 0;
	//char temp[MAX_DATA_SIZE + 1];
	unsigned char ret_val = FALSE;
			
	switch(vehicle_sync.sync_state)
	{
		case GPRS_VEHICLE_NO_UPDATE:
		{										//28-1-19 VC: add for freeze keyboard for not operate other one
			ret_val = TRUE;
#ifdef DEBUG_GPRS_SERVER_SYNC
	UWriteString((char*)"done_0..",UART_PC);
#endif	
		}
		break;
		case GPRS_VEHICLE_ERASE:
		{	// need to erase all the truck data
#ifdef DEBUG_GPRS_SERVER_SYNC
	UWriteString((char*)"erase_done..",UART_PC);
#endif

		}
		break;
		case GPRS_VEHICLE_UPDATE:
		{
		
		}
		break;
		default:
		break;
	}// end of switch
	return ret_val;
}
void prepareServerResponse(char *buff)
{
	//static unsigned int last_req_num = 0, transaction_req = 0;
	
	switch(vehicle_sync.gprs_config_req)
	{
		case GPRS_CHECK_CFG_UPDATE:
		{
			
		}
		break;
		case GPRS_SAVE_CFG_TIME:
		{
			get_present_time(&vehicle_sync_time.last_update_time);				// 15-2-19 VC:					
		//	sprintf(buff, "api/setbowsersyncedtimestamp?data=%06ld%02d%02d%02d%02d%02d%02d%02d\"\r",  product_info.industry_id, DEFAULT_YEAR_OFFSET, vehicle_sync_time.last_update_time.yr, vehicle_sync_time.last_update_time.mm,
		//	vehicle_sync_time.last_update_time.dd, vehicle_sync_time.last_update_time.hr, vehicle_sync_time.last_update_time.min, DUMMY_TIME_SEC);
			
			//last_req_num = 0;			 // 16-2-19 VC: need to clear this
#ifdef DEBUG_GPRS_SERVER_SYNC
UWriteString((char*)"time_rsp",UART_PC);
#endif			
		}				
		break;
		default:
		break;
	}
}

void setServerResponseState(gprs_config_req_t gprs_config_req_)
{
	vehicle_sync.gprs_config_req = gprs_config_req_;
}
gprs_config_req_t getServerResponseState(void )
{
	return vehicle_sync.gprs_config_req;
}
void releaseCnfgState()
{
	//vehicle_sync.gprs_sync_state = GPRS_WAIT_FOR_CMD;						// 13-2-19 VC: state for taking cmd
	//setServerResponseState(GPRS_CHECK_CFG_UPDATE);				// 13-2-19 VC: need to go for init server packet
	setRtuState(RTU_NORMAL);									// 15-2-19 VC: reset time for 	
#ifdef DEBUG_GPRS_SERVER_SYNC
UWriteString("rtu_n:",UART_PC);
#endif
// 	free(gprs_temp_rx_buff.buff);
// 	gprs_temp_rx_buff.buff = NULL;
}
void resetVehicleSyncTime(void)
{
	memset((void*)&vehicle_sync_time,0,sizeof(vehicle_sync_time_t));
	vehicle_sync_time.last_update_time.yr = 0;
	vehicle_sync_time.last_update_time.mm = 1;
	vehicle_sync_time.last_update_time.dd = 1;
	eepromWriteVehicleSyncTime();
}

void gprs_cfg_updt_disp(void)
{
	static unsigned char display_toggle = 1, toggle_time = 0;
	//static unsigned int system_fail_time_out = 0;
	
	switch(vehicle_sync.gprs_cfg_disp_state)
	{
		case FDM_GPRS_CONFIG_INIT:
		if(vehicle_sync.gprs_sync_state == GPRS_WAIT_FOR_CMD)
		{
			
		}
		++toggle_time;
		if((toggle_time % 5) == 0)
		{
			display_toggle  ^= 1;
		}
		
		break;
		case FDM_GPRS_CONFIG_CONNECTION_ERROR:
			
		break;
		case FDM_GPRS_CONFIG_PROCESS_DONE:						// upload successfully
		
		break;
		case FDM_GPRS_CONFIG_NO_UPDATE:		// no update available
		
		break;
		case FDM_GPRS_CONFIG_UPLOAD_FAIL:			// error in upload
		
		break;
		default:
		break;
	}
}
void setGprsCfgDispState(gprs_cfg_disp_state_t _gprs_config_state)  // 15-2-19 VC:
{
	vehicle_sync.gprs_cfg_disp_state = _gprs_config_state;
}

void init_hex_file_data(void)
{
	memset((void*)&flash_hexfile, 0, sizeof(flash_hexfile_t));
	flash_hexfile.addr = FL_HEX_FILE_START_ADDR;
	set_frmwr_choice(0xFF);
	flash_hexfile.hex_file_updt_mode = HEX_FILE_UPDT_AUTO;
}
unsigned char update_flash_hexfile(char *str)
{
	unsigned int index = 0, j = 0;
	unsigned char hex_data[32] = {0}, resp = FALSE;
	unsigned int dd_cnfg = 0;
	
	while((str[index] != '"') && (index < GPRS_TEMP_RX_BUFFER_MAX))
	index++;
	if(index < (GPRS_TEMP_RX_BUFFER_MAX - 1))
	index++;	//skipping "
	if (str[index] == '1')
	{
		index++;
		if(index < (GPRS_TEMP_RX_BUFFER_MAX - 1))
		index++;	//skipping ,
		switch (flash_hexfile.state)
		{
			case HEX_CHK_FRMWR_UPDT:
			if (!memcmp(&str[index], PRODUCT_CODE, strlen(PRODUCT_CODE)))
			{
				index += strlen(PRODUCT_CODE) + FIRMWARE_VER_LEN;
				dd_cnfg = hex2int(&str[index]);
				index += 2;
				dd_cnfg = (dd_cnfg << 8) | hex2int(&str[index]);
				if((dd_cnfg & 0x03FF) == (get_rtu_cnfg() &0x03FF))
				{
					index -= (FIRMWARE_VER_LEN + 2);
					update_e2p_frm_ver(&str[index]);
					flash_hexfile.state = HEX_RQST_NEW_REC;
					flash_hexfile.pkt_no = 0;
					flash_hexfile.hex_file_updt_state = DOWNLOADING;
					resp = TRUE;
					flashClrHexFile();
					//22-06-21 SK: hex file memory sector should be cleared here
				}
			}
			break;
			case HEX_RQST_NEW_REC:
			do
			{
				if(str[index] == ':')
				{
					#ifdef DEBUG_HEX_FILE_STORE
					UWriteString((char *)"Initial Packet Correct", UART_PC);
					#endif
					hex_data[j++] = str[index++];
					hex_data[j] = ((str[index++] - '0') << 4);
					if ((str[index] >= '0') && (str[index] <= '9'))
					{
						str[index] -= '0';
					}
					else if ((str[index] >= 'A') && (str[index] <= 'F'))
					{
						str[index] = ((str[index] - 'A') + 10);
					}
					else
					{
						break;
					}
					hex_data[j++] |= str[index++];
					//index += 2;
					resp = validate_data(&hex_data[1], &str[index], ((hex_data[1] + 4) * 2));
					if (resp)
					{
						flashWriteHex(hex_data, &flash_hexfile.addr);
						flash_hexfile.pkt_no++;
					}
					else
					{
						break;
					}
					index += ((hex_data[1] + 4) * 2);
					memset(hex_data, 0, 32);
					j = 0;
					resp = FALSE;
				}
				else
				{
					#ifdef DEBUG_HEX_FILE_STORE
					UWriteString((char *)"Packet Initial Fail", UART_PC);
					#endif
					break;
				}
			} while (str[index] != '"');
			break;
			default:
#ifdef DEBUG_HEX_FILE_STORE
			UWriteString((char *)"resp_1_def:",UART_PC);
#endif
			break;
		}
		if (str[index] == '"')
		{
			resp = TRUE;
		}
		else
		{
			
		}
	}
	else if(str[index] == '0')
	{
		index++;
		/*
		#ifdef DEBUG_HEX_FILE_STORE
		UWriteString("resp_0:",UART_PC);
		#endif*/
		switch(flash_hexfile.state)
		{
			case HEX_CHK_FRMWR_UPDT:
			#ifdef DEBUG_HEX_FILE_STORE
			UWriteString((char *)"no_fw:",UART_PC);
			#endif
			//No Firmware Available
			flash_hexfile.hex_file_updt_state = NO_UPDATE;
			break;
			case HEX_RQST_NEW_REC:
			// Firmware update Complete
			#ifdef DEBUG_HEX_FILE_STORE
			UWriteString((char *)"fw_finish:",UART_PC);
			#endif
			//flash_hexfile.hex_file_updt_state = DWNLD_CMPLT;
			flash_hexfile.hex_file_updt_state = VERIFYING;
			set_frmwr_update_state(HEX_FILE_UPDATE_VERIFY);
			break;
			default:
#ifdef DEBUG_HEX_FILE_STORE
			UWriteString((char *)"resp_0_def:",UART_PC);
#endif
			break;
		}
		set_flash_file_rqst_type(HEX_NO_RQST);
		resp = TRUE;
	}
	else if(str[index] == '2')
	{
		index++;
		/*
		#ifdef DEBUG_HEX_FILE_STORE
		UWriteString("resp_2:",UART_PC);
		#endif*/
		switch(flash_hexfile.state)
		{
			case HEX_CHK_FRMWR_UPDT:
#ifdef DEBUG_HEX_FILE_STORE
			UWriteString((char *)"resp_2_chk:",UART_PC);
#endif
			//Nothing to do
			break;
			case HEX_RQST_NEW_REC:
			// Requested file not found
#ifdef DEBUG_HEX_FILE_STORE
UWriteString((char *)"no_file:",UART_PC);
#endif
			flash_hexfile.hex_file_updt_state = FILE_NOT_FOUND;
			flashClrHexFile();			//17-04-2021 SK: If the file which we are downloading is not found then lets remove it from flash 
			break;
			default:
#ifdef DEBUG_HEX_FILE_STORE
			UWriteString((char *)"resp_2_def:",UART_PC);
#endif
			break;
		}
		set_flash_file_rqst_type(HEX_NO_RQST);
		resp = TRUE;
	}
	else
	{
		/*
		#ifdef DEBUG_HEX_FILE_STORE
		UWriteString("resp_unk:",UART_PC);
		UWriteData(str[index],UART_PC);
		UWriteInt(index,UART_PC);
		#endif*/
	}
	return resp;
}
char validate_data(unsigned char *dest, char *src, int len)
{
	unsigned char chksum = 0, j = 0, resp = FALSE;
	unsigned char value = 0;
	#ifdef DEBUG_HEX_FILE_STORE
	UWriteString((char *)"src:",UART_PC);
	UWriteBytes((char *)src,len,UART_PC);
	#endif
	chksum += dest[j++];
	for (int i = 0; i < len; i++)
	{
		if ((src[i] >= '0') && (src[i] <= '9'))
		{
			src[i] -= '0';
		}
		else if ((src[i] >= 'A') && (src[i] <= 'F'))
		{
			src[i] = ((src[i] - 'A') + 10);
		}
		else
		{
			#ifdef DEBUG_HEX_FILE_STORE
			UWriteString((char *)"Invalid Bytes", UART_PC);
			#endif
			break;
		}
		if (i % 2 != 0)
		{
			value = (value << 4) | (src[i]);
			dest[j++] = value;
			//UWriteData(value, UART_PC);
			if (i < (len - 1))
			{
				chksum += value;
			}
			else
			{
				//UWriteData(chksum, UART_PC);
				chksum = ~chksum + 1;
				//UWriteData(chksum, UART_PC);
				if (value == chksum)
				{
					resp = TRUE;
				}
				else
				{
					#ifdef DEBUG_HEX_FILE_STORE
					UWriteString((char *)"Checksum_fail", UART_PC);
					UWriteInt(chksum,UART_PC);
					UWriteData(',',UART_PC);
					UWriteInt(value,UART_PC);
					#endif
					break;
				}
			}
		}
		else
		{
			value = src[i];
		}
	}
	return resp;
}

void release_hex_file_states(void)
{
	//resetSystemIdleTime();
	/*free(gprs_temp_rx_buff.buff);
	gprs_temp_rx_buff.buff = NULL;*/
	
#ifdef DEBUG_HEX_FILE_STORE
	dyn_mem_dealloc_cnt++;
	UWriteString((char*)"dyn_mem:free",UART_PC);
	UWriteInt(dyn_mem_alloc_cnt,UART_PC);
	UWriteData(',',UART_PC);
	UWriteInt(dyn_mem_dealloc_cnt,UART_PC);
#endif
}
hex_file_updt_state_t get_hex_file_updt_state(void)
{
	return flash_hexfile.hex_file_updt_state;
}
void set_hex_file_updt_state(hex_file_updt_state_t state)
{
	flash_hexfile.hex_file_updt_state = state;
}
unsigned char check_pending_hex_data(void)
{
	unsigned long flash_file_address = FL_HEX_FILE_START_ADDR;
	unsigned char ack = 0;
	unsigned char temp_data[32] = {0};
	unsigned int line_count = 0;
	do
	{
		wdt_reset();
		#ifdef DEBUG_BOOT_HEX
		//UWriteString((char *)"\naddr:",UART_PC);
		//UWriteInt(flash_file_address, UART_PC);
		#endif
		read_flash_file_row(flash_file_address, temp_data);
		if (temp_data[0] == ':')
		{
			if (temp_data[temp_data[1] + 5] != getChecksum(&temp_data[1], (temp_data[1] + 4)))
			{
				break;
			}
		}
		else
		{
			break;
		}
		flash_file_address += 32;
		line_count++;
	} while ((temp_data[1] != 0) && (temp_data[1] != 0xFF));
	if (/*(temp_data[1] == 0) || */(temp_data[1] == 0xFF) && line_count)
	{
		ack = 1;
		flash_hexfile.addr = flash_file_address;
	}
	flash_hexfile.pkt_no = line_count;
	return ack;			// returns true if downloaded file is pending
}
void set_flash_file_rqst_type(hex_file_updt_gprs_state_t state)
{
	flash_hexfile.state = state;
}
hex_file_updt_gprs_state_t get_flash_file_rqst_type(void)
{
	return flash_hexfile.state;
}
#ifdef ONLINE_CONFIG_EN
unsigned char get_online_cfg_ver_cnt(void)
{
	return e2p_online_cnfg.online_cfg_version_count;
}

void set_online_cfg_ver_cnt(unsigned char val)
{
	e2p_online_cnfg.online_cfg_version_count = val;
}
#endif
unsigned char allot_extra_mem(void)
{
	
	/*gprs_temp_rx_buff.buff = (char*) calloc(GPRS_TEMP_RX_BUFFER_MAX, sizeof(char));
	if(gprs_temp_rx_buff.buff == NULL)
	{
		
		#ifdef DEBUG_HEX_FILE_STORE
		UWriteString((char*)"dyn_mem:f",UART_PC);
		
		#endif
		return 0;
	}
	else
	{
		
		#ifdef DEBUG_HEX_FILE_STORE
		UWriteString((char*)"dyn_mem:k",UART_PC);
		dyn_mem_alloc_cnt++;
		UWriteInt(dyn_mem_alloc_cnt,UART_PC);
		UWriteData(',',UART_PC);
		UWriteInt(dyn_mem_dealloc_cnt,UART_PC);
		#endif
		return 1;
	}*/
	return 1;
}
void set_hexfile_init_addr(void)
{
	flash_hexfile.addr = FL_HEX_FILE_START_ADDR;
}

hex_file_updt_mode_t get_hex_file_updt_mode(void)
{
	return flash_hexfile.hex_file_updt_mode;
}

void set_hex_file_updt_mode(hex_file_updt_mode_t mode)
{
	flash_hexfile.hex_file_updt_mode = mode;
}
#endif