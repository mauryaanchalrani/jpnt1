#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "config.h"
#include "app_eeprom.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>		
#include "main.h"
#include "app_uart.h"
#include "language.h"
#include "common.h"
#include "sensor.h"
#include "pins.h"

extern freq_updated_data_t freq_updated_data;		//Anand 22-03-16

#ifdef GPRS_ENABLE
extern vehicle_sync_time_t vehicle_sync_time;	// 14-2-19 VC: 
#endif
extern product_info_t product_info;		//HJ 0-08-2016
#ifdef ONLINE_CONFIG_EN
extern product_info_config_t product_info_config;
#endif
extern fm_e2p_data_t fm_e2p_data; // VC:22/08/2018
extern calbration_data_t calbration_data;
extern em_data_t em_data; // 20-11-2018 VC: taking for save the old kwh and kwah
extern gprs_config_data_t gprs_config_data;
extern ram_data_t ram_data;
e2p_hex_file_hdr_t e2p_hex_file_hdr;
e2p_log_data_t e2p_log_data;

e2p_log_y_tot_t e2p_log_y_tot;			//abdul majeed
extern unsigned long int yest_total;	//abdul majeed



void write_default_GPRS_config(void)
{
	cli();
	memset((void *)&gprs_config_data, 0, sizeof(gprs_config_data_t));
	memcpy(gprs_config_data.websiteUrl, "34.237.21.50/", (strlen("34.237.21.50/")+1));
	memcpy(gprs_config_data.apn,"airtelgprs.com",(strlen("airtelgprs.com")));
	gprs_config_data.port_address = 10000;
	
	gprs_config_data.chksum = getChecksum((unsigned char*)(&gprs_config_data), (sizeof(gprs_config_data_t) - 1));
	eepromWriteGPRSConfig(&gprs_config_data);
	sei();
}


void eepromWriteGPRSConfig(gprs_config_data_t *gprs_cfg)
{
	cli();
	int addr = E2P_GPRS_CFG_DATA;
	gprs_cfg->chksum = getChecksum((unsigned char*)gprs_cfg, (sizeof(gprs_config_data_t) - 1 - GPRS_CNFG_RESERVED));
	eeprom_write_block(gprs_cfg, (void*)addr, sizeof(gprs_config_data_t));
	sei();
}
char eepromreadGPRSConfig(void)
{
	char read_ok = 1;
	int i=0;
	int addr = E2P_GPRS_CFG_DATA;
	cli();
	for(i=0 ; i<3 ; i++)
	{
		eeprom_read_block(&gprs_config_data, (void*)addr, sizeof(gprs_config_data_t));
		if(gprs_config_data.chksum == getChecksum((unsigned char*)&gprs_config_data, sizeof(gprs_config_data_t) - 1 - GPRS_CNFG_RESERVED))
		{
			break;
		}
	}
	sei();
	if(i>=3)
	{
		read_ok = 0;
	}
	return read_ok;
}

char eepromReadCalData(void)
{
	char read_ok = 1;
	int i=0;
	int addr = E2P_ADD_CAL_DATA;
	cli();		//Anand 20.06.2014
	for(i=0 ; i<3 ; i++)
	{
		eeprom_read_block(&calbration_data, (void*)addr, sizeof(calbration_data_t));
		if(calbration_data.chksum == getChecksum((unsigned char*)&calbration_data, sizeof(calbration_data_t) - 1))
		{
			break;
		}
	}
	sei();		//Anand 20.06.2014
	if(i>=3)
	{
		read_ok = 0;

	}
	return read_ok;
}
void eepromWriteCalData(void)
{
	cli();
	int addr = E2P_ADD_CAL_DATA;
	calbration_data.chksum = getChecksum((unsigned char*)(&calbration_data), (sizeof(calbration_data_t) - 1));		
	eeprom_write_block(&calbration_data, (void*)addr, sizeof(calbration_data_t));
	sei();
	
} 
void eepromWriteProductInfo(void)
{
	cli();
	int addr = E2P_ADD_PRODUCT_INFO;
	product_info.chksum = getChecksum((unsigned char*)(&product_info), (sizeof(product_info_t) - 1));		//22.05.2014
	eeprom_write_block(&product_info, (void*)addr, sizeof(product_info_t));		//R
	sei();
}

char eepromReadProductInfo(void)
{
	char read_ok = 1;
	int i=0;
	int addr = E2P_ADD_PRODUCT_INFO;
	cli();		//Anand 20.06.2014
	for(i=0 ; i<3 ; i++)
	{
		eeprom_read_block(&product_info,(void*)addr,sizeof(product_info_t));		//R
		if(product_info.chksum == getChecksum((unsigned char*)&product_info,sizeof(product_info_t)-1))
		{
			break;
		}
	}
	sei();		//Anand 20.06.2014
	if(i>=3)
	{
		read_ok = 0;
		
		//Log Sys error and go to config mode
		SYS_ERR_LED_PORT|=(1<<SYS_ERR_LED);
	}
	return read_ok;
}
#ifdef ONLINE_CONFIG_EN

void eepromWriteProductInfoConfig(void)
{
	cli();
	int addr = E2P_ADD_PRODUCT_INFO_CONFIG;
	product_info_config.chksum = getChecksum((unsigned char*)(&product_info_config), (sizeof(product_info_config_t) - 1));		//22.05.2014
	eeprom_write_block(&product_info_config, (void*)addr, sizeof(product_info_config_t));		//R
	sei();
}

char eepromReadProductInfoConfig(void)
{
	char read_ok = 1;
	int i=0;
	int addr = E2P_ADD_PRODUCT_INFO_CONFIG;
	cli();		//Anand 20.06.2014
	for(i=0 ; i<3 ; i++)
	{
		eeprom_read_block(&product_info_config,(void*)addr,sizeof(product_info_config_t));		//R
		if(product_info_config.chksum == getChecksum((unsigned char*)&product_info_config,sizeof(product_info_config_t)-1))
		{
			break;
		}
	}
	sei();		//Anand 20.06.2014
	if(i>=3)
	{
		read_ok = 0;
		
		SYS_ERR_LED_PORT|=(1<<SYS_ERR_LED); //Log Sys error and go to config mode
	}
	
	return read_ok;
}
#endif

void write_default_product_info(void)
{
	cli();
	unsigned int temp_id = product_info.industry_id;
	memset((void *)&product_info, 0, sizeof(product_info_t));
	product_info.industry_id = temp_id;
	product_info.daily_limit = 100;
	product_info.daily_limit_dp = 0;
	memcpy((void*)&product_info.ind_name,"E4ENGINEER",strlen("E4ENGINEER")/*INDUS_NAME_LEN*/);
	product_info.ADC_offset = 200;
	product_info.calib_factor = 1500;
	
	eepromWriteProductInfo();
#ifdef ONLINE_CONFIG_EN	
	product_info_config.time_slot = 1400;
	product_info_config.flow_meter_make = KROHNE_MAR_50;
	product_info_config.connection_type = FM_CONN_ADC420;
	eepromWriteProductInfoConfig();
#endif	
	sei();
}

// this functions are used for mbtcp command handling 
void writeCalDataFromMbtoEeprom(unsigned int data, unsigned int parameter)
{
	if(parameter == CAL_LVL)
	{
		calbration_data.cal_data_lvl1 = data;	
	}
	else if(parameter == CAL_PRESSURE)
	{
		calbration_data.cal_data_pressure1 = data;
	}
	else if(parameter == CAL_FLOW)
	{
		calbration_data.cal_data_flow1 = data;
	}
	
	eepromWriteCalData();
}
void writeEmInfoFromMbtoEeprom(long data, unsigned int parameter)
{
	
}

void eepromWriteRTUConfigFromMbtcp(unsigned char data)
{

}

void writeProductInfoFromMbtoEeprom(long  data)
{
	
}
void eepromWriteVehicleSyncTime()
{
	cli();
	int addr = E2P_VEHICLE_LIST_UPDATE_TIME;
	vehicle_sync_time.checksum = getChecksum((unsigned char*)(&vehicle_sync_time), (sizeof(vehicle_sync_time_t) - 1 - VEHICLE_SYNC_UNUSED));		//22.05.2014
	eeprom_write_block(&vehicle_sync_time, (void*)addr, sizeof(vehicle_sync_time_t));	
	sei();
}

char eepromReadVehicleSyncTime(void)
{
	char read_ok = 1;
	int i=0;
	int addr = E2P_VEHICLE_LIST_UPDATE_TIME;
	cli();	
	for(i=0 ; i<3 ; i++)
	{
		eeprom_read_block(&vehicle_sync_time, (void*)addr, sizeof(vehicle_sync_time_t));
		if(vehicle_sync_time.checksum == getChecksum((unsigned char*)&vehicle_sync_time, sizeof(vehicle_sync_time_t) - VEHICLE_SYNC_UNUSED - 1))
		{
			break;
		}
	}
	sei();	
	if(i>=3)
	{
		read_ok = 0;
	}
	return read_ok;
}

void eepromUpdateHexFileHdr(void)
{
	cli();
	int addr = E2P_FLASH_HEX_FILE_UPDATE;
	e2p_hex_file_hdr.checksum = getChecksum((unsigned char*)(&e2p_hex_file_hdr), (sizeof(e2p_hex_file_hdr_t) - 1));
	eeprom_update_block(&e2p_hex_file_hdr, (void*)addr, sizeof(e2p_hex_file_hdr_t));
	sei();
}
char eepromReadHexFileHdr(void)
{
	char read_ok = 1;
	int i=0;
	int addr = E2P_FLASH_HEX_FILE_UPDATE;
	cli();
	for(i=0 ; i<3 ; i++)
	{
		eeprom_read_block(&e2p_hex_file_hdr, (void*)addr, sizeof(e2p_hex_file_hdr_t));
		if(e2p_hex_file_hdr.checksum == getChecksum((unsigned char *)&e2p_hex_file_hdr, (sizeof(e2p_hex_file_hdr_t) - 1)))
		{
			break;
		}
	}
	sei();
	if(i>=3)
	{
		read_ok = 0;
	}
	return read_ok;
}
void hex_file_ready(unsigned char state)
{
	e2p_hex_file_hdr.hex_file_update_flag = state;		//24-08-2020 SK
	eepromUpdateHexFileHdr();
}
void update_e2p_frm_ver(char *str)
{
	memcpy((void *)&e2p_hex_file_hdr.project_version, (void *)str, FIRMWARE_VER_LEN);	//24-08-2020 SK
	eepromUpdateHexFileHdr();
}
char *get_e2p_frm_ver(void)
{
	return e2p_hex_file_hdr.project_version;
}

char *get_gprs_config (char *data)
{
	if(data[0] == 0x01)	//Header 1 ID
	{
		
		return (char *)&gprs_config_data.apn;
	}
	else if(data[0] == 0x02)	//Header 2 ID
	{
		
		return (char *)&gprs_config_data.websiteUrl;
	}
	else
	{
		return (char *)&gprs_config_data.apn;
	}
}
void update_gprs_config(char *data)
{
	int i=0;
	eepromreadGPRSConfig();
	
	if(data[0] == 0x01)
	{
		memset(&gprs_config_data.apn, 0, sizeof(gprs_config_data.apn));
		
		for (i=0; data[i + 1] != '\0'; i++)
		{
			gprs_config_data.apn[i] = data[i + 1];
		}
	}
	else if(data[0] == 0x02)
	{
		memset(&gprs_config_data.websiteUrl, 0, sizeof(gprs_config_data.websiteUrl));
		
		for (i=0; data[i + 1] != '\0'; i++)
		{
			gprs_config_data.websiteUrl[i] = data[i + 1];
		}
	}
	eepromWriteGPRSConfig(&gprs_config_data);
}

char read_e2p_log_data()
{
	char read_ok = 1;
	int i = 0;
	unsigned long addr;
	e2p_log_data_t e2p_log_data_;
	unsigned char j=1;
	j = 1;
	cli();
	for(i=0 ; i<3 ; i++)
	{
		do{
			addr = (E2P_LOG_DATA_START_ADDRESS + (j++ * E2P_LOG_DATA_LEN));
			if(addr < E2P_LOG_DATA_MAX_ADDRESS)
			{
				eeprom_read_block(&e2p_log_data,(void *)(addr - E2P_LOG_DATA_LEN),E2P_LOG_DATA_LEN);
				eeprom_read_block(&e2p_log_data_,(void *)addr,E2P_LOG_DATA_LEN);
				if(e2p_log_data_.cnt == 0xFF)
				{
					break;
				}
			}
			else
			{
				
				memcpy(&e2p_log_data, &e2p_log_data_, E2P_LOG_DATA_LEN);		//At the end the second one is holding the Gold
				e2p_clear_log_sector();
				break;
			}
		}while((e2p_log_data.cnt + 1) == e2p_log_data_.cnt);
		
		if(e2p_log_data.checksum == getChecksum((unsigned char*)(&e2p_log_data), (sizeof(e2p_log_data_t) - 1/* - BD_FREQ_UNUSED_LEN*/)))	// check how your wrote in writecfg
		{
			read_ok = 1;
			break;
		}
		else
		{
			j = 1;
			read_ok = 0;
		}
	}
	sei();
	if(i>=3)
	{
		read_ok = 0;		
	}
	if (!read_ok)
	{
		e2p_log_data.t_total = 0;
		e2p_log_data.m_total = 0;
		e2p_log_data.c_total = 0;
	}


	return read_ok;
}

void e2p_clear_log_sector(void)
{
	cli();
	
	int addr = E2P_LOG_DATA_START_ADDRESS;
	// Lets clear the first Row of circular buffer
	e2p_log_data_t tmp;
	memset(&tmp, 0, sizeof(e2p_log_data_t));
	eeprom_write_block(&tmp, (void*)addr, sizeof(e2p_log_data_t));
	
	//Lets copy latest data on first row
	e2p_log_data.checksum = getChecksum((unsigned char*)(&e2p_log_data), (sizeof(e2p_log_data_t) - 1));		//22.05.2014
	eeprom_write_block(&e2p_log_data, (void*)addr, sizeof(e2p_log_data_t));		//R
	
	addr += sizeof(e2p_log_data_t);
	//Lets clear all remaining rows
	while(addr < E2P_LOG_DATA_MAX_ADDRESS)
	{
		eeprom_write_block(&tmp, (void*)addr, sizeof(e2p_log_data_t));
		addr += sizeof(e2p_log_data_t);
	}
	
	sei();

}

void e2p_write_log_data(void)
{
	cli();
	++e2p_log_data.cnt;
	
	int addr = E2P_LOG_DATA_START_ADDRESS + ((e2p_log_data.cnt - 1) * sizeof(e2p_log_data_t));
	if((addr < E2P_LOG_DATA_MAX_ADDRESS) && e2p_log_data.cnt)
	{
		e2p_log_data.checksum = getChecksum((unsigned char*)(&e2p_log_data), (sizeof(e2p_log_data_t) - 1/* - BD_FREQ_UNUSED_LEN*/));
		eeprom_write_block(&e2p_log_data, (void*)addr, sizeof(e2p_log_data_t));		//R
		
	}
	else
	{
		e2p_log_data.cnt = 1;			//Lets keep cnt within 1 to 10, otherwise it will create problem after 255 transactions.
		
		e2p_clear_log_sector();
		
	}
	sei();

}

char e2p_read_y_total(void)	//abdul majeed
{
	char read_ok = 1;
	int addr = E2P_Y_TOTAL_START_ADDRESS;
	cli();		//Anand 20.06.2014
	eeprom_read_block(&e2p_log_y_tot,(void*)addr,sizeof(e2p_log_y_tot_t));		//R
	
	sei();		//Anand 20.06.2014

	return read_ok;
	
}

void e2p_write_y_total(void)	//abdul majeed
{
	cli();
	int addr = E2P_Y_TOTAL_START_ADDRESS ;
	eeprom_write_block(&e2p_log_y_tot, (void*)addr, sizeof(e2p_log_y_tot_t));		//R
	sei();
	
}

void e2p_clear_y_total_sector(void)	//abdul majeed
{

}


void clear_e2p_log()
{
	cli();
	
	int addr = E2P_LOG_DATA_START_ADDRESS;
	// Lets clear the first Row of circular buffer
	e2p_log_data_t tmp;
	memset(&tmp, 0, sizeof(e2p_log_data_t));
	eeprom_write_block(&tmp, (void*)addr, sizeof(e2p_log_data_t));
		
	addr += sizeof(e2p_log_data_t);
	//Lets clear all remaining rows
	while(addr < E2P_LOG_DATA_MAX_ADDRESS)
	{
		eeprom_write_block(&tmp, (void*)addr, sizeof(e2p_log_data_t));
		addr += sizeof(e2p_log_data_t);
	}
	
	sei();
}

#ifdef ONLINE_CONFIG_EN
void update_rtu_config(online_cfg_t *data, unsigned char vercount)
{
	product_info.daily_limit	=	data->daily_limit;
	product_info.daily_limit_dp	=	data->daily_limit_dp;


	eepromWriteProductInfo();
	
	set_online_cfg_ver_cnt(vercount);
	eeprom_write_online_cfg_cnt();
}

void eeprom_write_online_cfg_cnt()
{
	cli();
	int addr = E2P_ONLINE_CFG_CNT;
	e2p_online_cnfg_t temp_ver_cnt;
	temp_ver_cnt.online_cfg_version_count = get_online_cfg_ver_cnt();
	eeprom_write_block((const void *)&temp_ver_cnt, (void*)addr, sizeof(e2p_online_cnfg_t));
	sei();
}

char eeprom_read_online_cfg_cnt()
{
	char read_ok = 1;
	int i=0;
	e2p_online_cnfg_t temp_ver_cnt;
	int addr = E2P_ONLINE_CFG_CNT;
	cli();
	for(i=0 ; i<3 ; i++)
	{
		eeprom_read_block(&temp_ver_cnt,(void*)addr,sizeof(e2p_online_cnfg_t));
		if(temp_ver_cnt.online_cfg_version_count <= 100)
		{
			set_online_cfg_ver_cnt(temp_ver_cnt.online_cfg_version_count);
			break;
		}
	}
	sei();
	if(i>=3)
	{
		read_ok = 0;
	}
	return read_ok;
}
#endif