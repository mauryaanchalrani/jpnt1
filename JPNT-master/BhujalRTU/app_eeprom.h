

#ifndef _APP_EEPROM_H_
#define _APP_EEPROM_H_

#include <avr/io.h>

#include "config.h"
#include "sms.h"
#include "gprs.h"

/************************************************
	MACROS & ENUM DEFINED
*************************************************/

#define E2P_ADD_PRODUCT_INFO		(0)			// 32 byte data packet

#define E2P_GSM_CFG_START_ADDRESS	(32)		// 16 byte data packet	

#define E2P_ADD_CAL_DATA			(80)		// 64 byte data packet

#define E2P_GPRS_CFG_DATA			(160)		// 51 byte data packet
#ifdef ONLINE_CONFIG_EN
#define E2P_ADD_PRODUCT_INFO_CONFIG	(240)			// 32 byte data packet
#endif
#define E2P_ONLINE_CFG_CNT			(256)

#define E2P_VEHICLE_LIST_UPDATE_TIME	(256)		

#define E2P_FLASH_HEX_FILE_UPDATE		(360)

#define E2P_Y_TOTAL_START_ADDRESS		(368)//yesterday total	//abdul majeed

#define E2P_DO_NOT_WRITE (0)
#define E2P_WRITE (1)

#define E2P_LOG_UNUSED	(0)

#define E2P_LOG_DATA_START_ADDRESS		(3072)
#define E2P_LOG_DATA_ALOT_SIZE			(1024)
#define E2P_LOG_DATA_LEN				(16)
#define E2P_LOG_DATA_NUM_OF_LOG			(E2P_LOG_DATA_ALOT_SIZE/E2P_LOG_DATA_LEN)
#define E2P_LOG_DATA_MAX_ADDRESS		(E2P_LOG_DATA_START_ADDRESS + (E2P_LOG_DATA_LEN*E2P_LOG_DATA_NUM_OF_LOG))


typedef struct
{
	unsigned char cnt;			//1
	unsigned long t_total;		//4
	unsigned long m_total;		//4
	unsigned long c_total;		//4
	unsigned char date;			//1
	unsigned char month;		//1
	unsigned char reserved[E2P_LOG_UNUSED];	//0
	unsigned char checksum;		//1
}e2p_log_data_t;

typedef struct
{
	unsigned long y_total;		//4
}e2p_log_y_tot_t;	//abdul majeed


/***************************************************
			F U N C T I O N S
****************************************************/
void epromPoweronRead();
//void eepromWriteBowser();
//void eeprom_power_on_read();
void eepromReset();
void eepromWritePswd();
void eepromReadPswd();
void eepromUpload();
//char get_chksum(char *ptr,int size);
#ifdef GPRS_ENABLE
void eepromWriteGPRSConfig(gprs_config_data_t*);
void eepromWriteVehicleSyncTime(void);
char eepromReadVehicleSyncTime(void);
char eepromreadGPRSConfig(void);
void write_default_GPRS_config(void);
void write_default_product_info(void);
#endif
//void eepromWritetmConfig();
//void eepromClearFreqUpdatedDataBlock(void);		//HJ 24-07-2016

//void eepromWriteRTUConfig(rtu_config_t *);
//char eepromReadRTUCfgData(rtu_config_t *);

#ifdef GPRS_ENABLE
char readGPRSConfig(void);
#ifdef ONLINE_CONFIG_EN
void update_rtu_config(online_cfg_t *,unsigned char);
void eeprom_write_online_cfg_cnt();
char eeprom_read_online_cfg_cnt();
#endif
#endif

//void eepromWriteSIMConfig(void);
//char eepromreadSIMConfig(void);

//HJ 03-08-2016
void eepromWriteProductInfo(void);
char eepromReadProductInfo(void);
#ifdef ONLINE_CONFIG_EN
void eepromWriteProductInfoConfig(void);
char eepromReadProductInfoConfig(void);
#endif
//void eepromWriteCal_Step_Data(cal_data_t*);
//void adj_cal_step_data(cal_data_t *, unsigned int);
//void eepromWriteCalHdr(cal_data_hdr_t *);
//char eepromReadCalHdr(cal_data_hdr_t *);
char eepromReadCalData(void);
void eepromClearCalData(void);
void write_default_cal_table(void);

void write_default_GSM_config(void);
char eepromreadGSMConfig(void);
void eepromWriteCumulativeData(void);
char eepromReadCumulativeData(void);
void eepromWriteCalData(void);

// VC: 20-11-2018
//void eepromWriteEmInfo(void);

//char eepromReadEmInfo(void);

// this functions are used for mbtcp handling
void writeCalDataFromMbtoEeprom(unsigned int, unsigned int);
void writeEmInfoFromMbtoEeprom(long , unsigned int);
void eepromWriteRTUConfigFromMbtcp(unsigned char);
void writeProductInfoFromMbtoEeprom(long);

void eepromUpdateHexFileHdr(void);
char eepromReadHexFileHdr(void);
void hex_file_ready(unsigned char);
char *get_e2p_frm_ver(void);
char *get_gprs_config (char *data);
void update_gprs_config(char *data);
char read_e2p_log_data();
void e2p_clear_log_sector(void);
void e2p_write_log_data(void);
void clear_e2p_log();
void update_e2p_frm_ver(char *);
//abdul majeed

char e2p_read_y_total(void);
void e2p_write_y_total(void);
void e2p_clear_y_total_sector(void);

/*_________________________________________________________________________________________*/
#endif
