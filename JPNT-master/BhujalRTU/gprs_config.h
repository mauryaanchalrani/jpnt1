#ifndef GPRS_CONFIG_H_
#define GPRS_CONFIG_H_

#include "config.h"
#include "sys_info.h"

#define NUM_OF_VEHICLE_PER_SYNC_PKT	(50) // 29-1-19 VC: need for sync with server
#define MAX_DATA_SIZE				(6)
#define DEFAULT_YEAR_OFFSET			(20)
#define DUMMY_TIME_SEC				(0x00)

#define GPRS_SYNC_MIN_CMD_INDEX		(0x03)
#define GPRS_SYNC_VEHICLE_LIST_INDX	(0x06)
#define GPRS_SYNC_CMD_INDEX			(0x04)
#define GPRS_SYNC_LEN_INDEX			(0x00)
#define WAIT_UPTO_PRINTING			(10)
#define GPRS_CFG_FAIL_TIMEOUT		(3000)		// 18-2-19 VC: 5min for system error

#define STATE_MCH_TIME 1000
#define LONG_PRESS_DELAY_CFG	(500/STATE_MCH_TIME)
//31-1-19 VC: taking for gprs device server sync flags

#define GPRS_TEMP_RX_BUFFER_MAX			(((30 + 1) * NUM_OF_VEHICLE_PER_SYNC_PKT) + 20)

#define NUM_OF_PKT_PER_QUERY			(30)
#define E2P_HEX_FILE_CNFG_UNUSED		(1)

typedef enum
{
	GPRS_VEHICLE_NO_UPDATE ,
	GPRS_VEHICLE_ERASE,
	GPRS_VEHICLE_UPDATE
}sync_state_t; // state for server req type

typedef enum
{
	GPRS_WAIT_FOR_CMD,
	GPRS_MSG_UPDATE,
	GPRS_GET_DATA,
}gprs_sync_state_t; // this is for main control to state from server 

typedef enum
{
	GPRS_CHECK_CFG_UPDATE,
	GPRS_GET_VEHICLE_INFO,
	GPRS_SAVE_CFG_TIME		
}gprs_config_req_t; // this is used for replay back to server

typedef enum
{
	FDM_GPRS_CONFIG_INIT,
	FDM_GPRS_CONFIG_CONNECTION_ERROR,
	FDM_GPRS_CONFIG_NO_UPDATE,
	FDM_GPRS_CONFIG_PROCESS_DONE,
	FDM_GPRS_CONFIG_UPLOAD_FAIL,
	FDM_GPRS_CONFIG_ERASE_LIST,
	FDM_GPRS_CONFIG_END
}gprs_cfg_disp_state_t;

typedef enum
{
	HEX_FILE_INIT,
	CHK_UPDT,
	CNFRM_DWNLD,
	RESUME_PENDING,
	DWNLD_CMPLT,
	FILE_NOT_FOUND,
	CONN_ERROR,
	DOWNLOADING,
	NO_UPDATE,
	DISPLAY_HOLD,
	UPDATE_FAIL,
	VERIFYING,
	DEFAULT,
}hex_file_updt_state_t;

typedef enum
{
	HEX_NO_RQST,	//25-08-2020 SK: This will be used when we are confirming from the user if he wants to download or not
	HEX_CHK_FRMWR_UPDT,
	HEX_RQST_NEW_REC,
}hex_file_updt_gprs_state_t;

typedef enum
{
	HEX_FILE_UPDT_MANUAL,
	HEX_FILE_UPDT_AUTO,
}hex_file_updt_mode_t;
typedef struct
{
	gprs_config_req_t gprs_config_req;
	sync_state_t sync_state;
	gprs_sync_state_t gprs_sync_state;
	gprs_cfg_disp_state_t gprs_cfg_disp_state;
	unsigned int num_of_vehicle;	
	unsigned int current_vehicle_num;	
}__attribute__((packed)) vehicle_sync_t;


typedef struct
{
	unsigned long addr;
	unsigned int pkt_no;
	hex_file_updt_state_t hex_file_updt_state;
	hex_file_updt_gprs_state_t state;
	hex_file_updt_mode_t hex_file_updt_mode;
}flash_hexfile_t;

typedef struct
{
	unsigned char hex_file_update_flag;
	char project_version[FIRMWARE_VER_LEN];
	char unused[E2P_HEX_FILE_CNFG_UNUSED];
	char checksum;
}__attribute__((packed)) e2p_hex_file_hdr_t;		//24-08-2020 SK

#ifdef ONLINE_CONFIG_EN
typedef struct  
{
	unsigned char online_cfg_version_count;		//1
}__attribute__((packed)) e2p_online_cnfg_t;
#endif

#ifdef GPRS_ENABLE
void gprsConfigRoutines(void);	//25-1-2019 VC:
void syncCnfgData(char *);	//30-1-2019 VC: take for device sync with server
unsigned char syncVehicleList(char *);
void prepareServerResponse(char *);
void setServerResponseState(gprs_config_req_t );
gprs_config_req_t getServerResponseState(void );
void releaseCnfgState(void);
void resetVehicleSyncTime(void);
void updateCurrentTransactionOnLcd(unsigned int, unsigned int);
void gprs_cfg_updt_disp(void);
void setGprsCfgDispState(gprs_cfg_disp_state_t);
void saveCnfgData(void);
void clearFlashData(void);
unsigned char update_flash_hexfile(char *);
char validate_data(unsigned char *, char *, int);
void release_hex_file_states(void);
void init_hex_file_data(void);
hex_file_updt_state_t get_hex_file_updt_state(void);
void set_hex_file_updt_state(hex_file_updt_state_t);
unsigned char check_pending_hex_data(void);
void set_flash_file_rqst_type(hex_file_updt_gprs_state_t);
unsigned char allot_extra_mem(void);
hex_file_updt_mode_t get_hex_file_updt_mode(void);
void set_hex_file_updt_mode(hex_file_updt_mode_t mode);
hex_file_updt_gprs_state_t get_flash_file_rqst_type(void);
#ifdef ONLINE_CONFIG_EN
unsigned char get_online_cfg_ver_cnt(void);
void set_online_cfg_ver_cnt(unsigned char);
unsigned char syncOnlineCnfgData(char *);//AM 29/05/21
#endif
#endif
#endif /* GPRS_CONFIG_H_ */