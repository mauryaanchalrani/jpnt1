/*
 * gprs.h
 *
 * Created: 3/9/2016 2:18:57 PM
 *  Author: Trilok
 */ 


#ifndef GPRS_H_
#define GPRS_H_

//#include "tank_monitoring.h"
#include "_debug.h"
#include "gprs_config.h"
//#include "jpnt_rtu.h"
#include "config.h"
#include "sys_info.h"
//Anand 16-3-16 For GPRS
#ifdef HTTP_ENABLE
#define GPRS_TX_BUFFER_MAX 192
#endif

#ifdef TCP_ENABLE
#define GPRS_TX_BUFFER_MAX 128
#endif
//#define GPRS_RX_BUFFER_MAX 64
#define GPRS_RX_BUFFER_MAX 128		//can not be less than 128 because of GPS response
#define GPRS_RX_READ_BUFFER_MAX 1400		//can not be less than 128 because of GPS response

//#define GPRS_TEMP_RX_BUFFER_MAX	(57 + 20)
#define GPRS_USART_RXC (USART0_RX_vect)
#define GPRS_UDR UDR0
#define GPRS_USART_TXC (USART0_TX_vect)
#define GPRS_UCSRA UCSR0A
#define GPRS_UDRE UDRE0

#define RETRY_CNT (3)

#define GPRS_SYNC_RETRY_CNT	(2)		// 15-2-19 VC: vehicle sync process retry cnt
#define GPRS_STATE_MC_TIME	(WEB_COMMS_SCHEDULAR_TIME)

#define	GPRS_CFG_RETRY_CNT	(3)

#define GSM_WARMUP_SECS		(10 *(1000/GPRS_STATE_MC_TIME)) // 10 sec
#define CCALR_TIMEOUT		(1000/GPRS_STATE_MC_TIME)		// 1 sec
#define GPRS_AT_TIMEOUT		(1000/GPRS_STATE_MC_TIME)		// 1 sec
#define CSQ_TIMEOUT			(1000/GPRS_STATE_MC_TIME)		// 1 sec
#define CGATT_TIMEOUT		(1000/GPRS_STATE_MC_TIME)		// 1 sec
#define CLTS_TIMEOUT		(1000/GPRS_STATE_MC_TIME)		// 1 sec
#define CCLK_TIMEOUT		(600/GPRS_STATE_MC_TIME)		// 600 msec
#define GPRS_MB_RSP_TIMEOUT	(2000/GPRS_STATE_MC_TIME)		// 1 sec

#ifdef	SOFT_RST_GSM
#define GSM_RST_SECS		(5 *(1000/GPRS_STATE_MC_TIME)) // 5 sec
#endif

#define TCP_STATE_MC_TIME		(GPRS_STATE_MC_TIME)

#define CREG_TIMEOUT			(1000/TCP_STATE_MC_TIME)		// 1 sec
#define CSTT_TIMEOUT			(1000/TCP_STATE_MC_TIME)		// 1 sec
#define CIPSHUT_TIMEOUT			(1000/TCP_STATE_MC_TIME)



#define GPRS_CONN_RETRY_TIME	(10000/GPRS_STATE_MC_TIME)		//30Sec			// Change 30 sec to 10 sec

#define GPRS_RETRY_CNT			(3)
#define SIM_MODULE_RESET_CNT	(3)
#ifdef HTTP_ENABLE
#define SAPBR_TYPE_TIMEOUT		(1000/GPRS_STATE_MC_TIME)		// 1 sec
#define SAPBR_APN_TIMEOUT		(20000/GPRS_STATE_MC_TIME)		// 20 sec
#define SAPBR_DISCON_TIMEOUT	(3000/GPRS_STATE_MC_TIME)		// 3 sec
#define SAPBR_STS_TIMEOUT		(1000/GPRS_STATE_MC_TIME)		// 1 sec

#ifdef SOFT_RST_GSM
#define SOFT_RST_TIMEOUT	(3000/GPRS_STATE_MC_TIME)		// 5 sec
#endif

#define HTTP_STATE_MC_TIME		(GPRS_STATE_MC_TIME)
#define HTTPINIT_TIMEOUT		(1000/HTTP_STATE_MC_TIME)
#define HTTPCID_TIMEOUT			(1000/HTTP_STATE_MC_TIME)
#define HTTPURL_TIMEOUT			(3000/HTTP_STATE_MC_TIME)
#define HTTPPOST_TIMEOUT		(3000/HTTP_STATE_MC_TIME)
#define HTTPPOST_1_TIMEOUT		(75000UL/HTTP_STATE_MC_TIME)		// Module give error response after 72 sec delay in case server is not present

#define HTTPREAD_TIMEOUT		(1000/HTTP_STATE_MC_TIME)
#define HTTP_RETRY_CNT			(3)
#define HTTP_ERR_OFFSET	(0xC0)
#endif	//HTTP_ENABLE

#define READ_RETRY_CNT	(3)

#define MIN_NETWORK_STRENGTH_DB (10)
#define MAX_NETWORK_STRENGTH_DB (30)

#define UPLOAD_RETRY_CNT		(3)

#define CON_ERR_OFFSET	(0xA0)

#define CMD_BUFFER_LEN	(32)
//#define HTTP_URL_BUFF_LEN	(192)

#define SER_PROVIDER_LEN	(16)
#define LOCKED 1
#define UNLOCKED 0

#define lock(a) {a = LOCKED;}
#define unlock(a) {a = UNLOCKED;}

#define DEFAULT_YEAR	(21)
	
#ifdef HTTP_ENABLE
#define HTTP_RESP_SUCCESS 200		//200
#else
#define HTTP_RESP_SUCCESS 200
#endif	//HTTP_ENABLE
#define RESP_CHK_TIME	(60/UPLOAD_RATE_ONLINE)

#define SIM_NUM_LEN		(14)
#define SIM_INFO_UNUSED_LEN	(1)

#define IP_LENGTH		(16)//including null
#define GPRS_URL_LEN	(32)
#define APN_LEN			(32)
#define GPRS_CNFG_RESERVED	(13)

#ifdef GPRS_ENABLE
/*
#define GPRS_UNUSED_LEN (31)
#define GPRS_APN_LEN (32)
#define GPRS_URL_LEN (48)*/

#define NUM_OF_GPRS_RX_BUFF	(3)

#define IMEI_NUM_LEN		(15)
#define IMEI_WAIT_TIME		(5)

/*
typedef struct
{
	char apn[GPRS_APN_LEN];
	char websiteUrl[GPRS_URL_LEN];
	char unused[GPRS_UNUSED_LEN];
	char chksum;		//Anand 5-4-16
}__attribute__((packed)) GPRSCfgData_t;*/
#endif

#ifdef ONLINE_CONFIG_EN
#ifdef ENABLE_4G
#define FUEL_RATE_INDEX	(21)
#define PID_INDEX	(21)
#else
#define FUEL_RATE_INDEX	(17)
#define PID_INDEX		(17)
#endif
#define FUEL_RATE_LEN	(5)
//#define PASS_LEN		(4)

#define	PID_LEN	(4)

#endif
/*
typedef struct
{
	unsigned char sim_no[SIM_NUM_LEN];
	unsigned char unused[SIM_INFO_UNUSED_LEN];
	unsigned char chksum;
}__attribute__((packed)) sim_info_t;*/ 

typedef enum
{
	NOT_AVBL,
	AVBL,
}gprs_status_t;

typedef enum
{
	CON_OK,
	CON_FAIL,
	CON_IN_PRG
}con_status_t;

typedef enum
{
	GPRS_IMEI,
	GPRS_IDLE,
	GPRS_CONNECT,
	GPRS_CONN_STS,				// HJ 02-09-2016
#ifdef HTTP_ENABLE
	GPRS_HTTP_INIT,
	GPRS_HTTP_CLOSE,
#endif
#ifdef TCP_ENABLE
	GPRS_TCP_CONNECT,
	GPRS_TCP_DISCONNECT,
	GPRS_WAIT_MB_CMD,
	GPRS_WAIT_MB_RSP,
	GPRS_SEND_MB_TCP,
	GPRS_CON_PAYLOAD,
	GPRS_TIME, // VC:18/08/2018
#endif
	GPRS_CONNECT_FMS_SERVER,
	GPRS_PREPARE_LOG,
	GPRS_LOGS_UPLOAD,
	GPRS_SESSION_IDLE,
	GPRS_CONFIG
}gprs_handler_state_t;
typedef enum
{
	GPRS_CONNCT_RESET,
	GPRS_CONNCT_POWER_UP,				//Wait for call ready
	GPRS_CONNCT_CMD_ECHO_OFF,
	GPRS_CONNCT_RSP_ECHO_OFF,
	GPRS_CONNCT_CMD_IMEI,
	GPRS_CONNCT_RSP_IMEI,				
	GPRS_CONNCT_CMD_CFUN,
	GPRS_CONNCT_RSP_CFUN,
	GPRS_CONNCT_CMD_AT,
	GPRS_CONNCT_RSP_AT,
	GPRS_CONNCT_RSP_1_CCALR,
	GPRS_CONNCT_RSP_CHK,				//General state to check response
	GPRS_CONNCT_CMD_CSQ,
	GPRS_CONNCT_RSP_CSQ,
	GPRS_CONNCT_RSP_1_CSQ,
	GPRS_CONNCT_CMD_CGATT,
	GPRS_CONNCT_RSP_CGATT,
	GPRS_CONNCT_RSP_1_CGATT,
	GPRS_CONNCT_CMD_CCALR,
	GPRS_CONNCT_RSP_CCALR,	
	GPRS_CONNCT_CMD_CREG,
	GPRS_CONNCT_RSP_CREG,
	GPRS_CONNCT_CMD_CSTT,
	GPRS_CONNCT_RSP_CSTT,
	GPRS_CONNCT_CMD_CIPSHUT,
	GPRS_CONNCT_RSP_CIPSHUT,
	GPRS_CONNCT_RSP_1_CREG,
#ifdef HTTP_ENABLE
	GPRS_CONNCT_CMD_SAPBR_TYPE,
	GPRS_CONNCT_RSP_SAPBR_TYPE,
	GPRS_CONNCT_CMD_SAPBR_APN,
	GPRS_CONNCT_RSP_SAPBR_APN,
	GPRS_CONNCT_CMD_SAPBR_IP,
	GPRS_CONNCT_RSP_SAPBR_IP,
	GPRS_CONNCT_CMD_SAPBR_CONN_STS,			// HJ 02-09-2016
	GPRS_CONNCT_RSP_SAPBR_CONN_STS,
	GPRS_CONNCT_CMD_CCLK,
	GPRS_CONNCT_RSP_CCLK,
	GPRS_CONNCT_RSP_1_CCLK,
	GPRS_CONNCT_CMD_CLTS,
	GPRS_CONNCT_RSP_CLTS,
#ifdef	SOFT_RST_GSM
	GPRS_CONNCT_CMD_RFUN,
	GPRS_CONNCT_CMD_RFUN_2,
	GPRS_CONNCT_RSP_RFUN,
	GPRS_CONNCT_RFUN_WAIT,
#endif
#endif	//HTTP_ENABLE
#ifdef TCP_ENABLE
	GPRS_CONNCT_CMD_CREG,
	GPRS_CONNCT_RSP_CREG,
	GPRS_CONNCT_RSP_1_CREG,
	GPRS_CONNCT_CMD_CSTT,
	GPRS_CONNCT_RSP_CSTT,
	GPRS_CONNCT_CMD_CIPSHUT, 
	GPRS_CONNCT_RSP_CIPSHUT,
#endif
	GPRS_CONNCT_CMD_SAPBR_DISCON,
	GPRS_CONNCT_RSP_SAPBR_DISCON,
}gprs_connct_state_t;

#ifdef HTTP_ENABLE
typedef enum
{
	GPRS_HTTP_IDLE,
	GPRS_HTTP_CMD_INIT,
	GPRS_HTTP_RSP_INIT,
	GPRS_HTTP_CMD_CID,
	GPRS_HTTP_RSP_CID,	
	GPRS_HTTP_CMD_URL,
	GPRS_HTTP_RSP_URL,
	GPRS_HTTP_CMD_DATA_LEN,				// Command to set lenght of data
	GPRS_HTTP_RSP_DATA_LEN,
	GPRS_HTTP_CMD_DATA_BUF,				// command to send data to module
	GPRS_HTTP_RSP_DATA_BUF,
	GPRS_HTTP_CMD_POST,
	GPRS_HTTP_RSP_POST,
	GPRS_HTTP_RSP_1_POST,
	GPRS_HTTP_CMD_READ,
	GPRS_HTTP_RSP_READ,
	GPRS_HTTP_CMD_CLOSE,
	GPRS_HTTP_RSP_CLOSE,
}gprs_http_state_t;
typedef enum
{
	HTTP_PASS,
	HTTP_FAIL,
	HTTP_IN_PRG
}http_status_t;

#endif	//HTTP_ENABLE
//HJ20-03-2018
typedef enum
{
	GPRS_NO_DATA=0,
	GPRS_RTU_DATA_RAM = 0xA0,
	GPRS_RTU_DATA_FLASH,
	GPRS_RTU_CONFIG,
	GPRS_HEX_FILE_DATA,
#ifdef ONLINE_CONFIG_EN
	GPRS_ONLINE_CFG,
#endif
	//GPRS_EVT_DATA
}gprs_upload_data_pkt_id_t;

//HJ 29-08-2016
typedef enum
{
	GPRS_DISCON,
	GPRS_CON,
	/*GPRS_WAIT_MB_CMD,
	GPRS_WAIT_MB_RSP,
	GPRS_SEND_MB_TCP,
	GPRS_CON_PAYLOAD,*/
}gprs_data_status_t;

/*
typedef enum
{
	IMEI_CMD_ECHO_OFF,
	IMEI_RSP_ECHO_OFF,
	IMEI_CONNCT_CMD_IMEI,
	IMEI_CONNCT_RSP_IMEI
}imei_status_t;*/

#ifdef ONLINE_CONFIG_EN			//20-03-2020 SK: added for online configuration
/*typedef enum
{
	CFG_VER_CNT,
	CFG_DW_DATA,
}online_cfg_state_t;*/
#endif

typedef enum
{
	NORMAL,
	SOFT_SHUT,
	AFTER_SOFT_SHUT,
	HARD_SHUT,
	AFTER_HARD_SHUT,
}gprs_conn_status;


typedef struct
{
	char imei[15];
	gprs_handler_state_t state;		//For state machine
	
	gprs_status_t module_status;
	
	unsigned char network_strength;		//chk for data type
	gprs_status_t network_status;
	
	gprs_status_t server_status;
	
	gprs_status_t connect_sts;
	
	//gprs_data_status_t gprs_data_status;
	unsigned char gprs_config_sts;					// to show eeprom configration error
	
	char errcode;						
}gprs_t;

typedef struct
{
	//unsigned char ip_address[IP_LENGTH];	// 16 bytes
	unsigned char websiteUrl[GPRS_URL_LEN];	// 32 bytes
	unsigned char apn[APN_LEN];				// 32 bytes
	unsigned int port_address;				// 2 bytes	
	unsigned char reserved[GPRS_CNFG_RESERVED];	// 13 bytes
	unsigned char chksum;					// 1 bytes	
}__attribute__((packed)) gprs_config_data_t; // 80 bytes

/*
typedef struct
{
	char imei_no[IMEI_NUM_LEN];
}sim_info_t;*/

#ifdef HTTP_ENABLE
typedef struct
{
	//gprs_http_state_t state;
	char errcode;
	
}http_t;
#endif	//HTTP_ENABLE

typedef struct  {
	char locked;
	char buffer[GPRS_RX_READ_BUFFER_MAX];
	unsigned int index;
}gprs_rx_data_buff_t;
/*
typedef struct
{
	char locked;
	char *buff;
	unsigned int index;
}gprs_temp_rx_buff_t;*/
typedef struct  {
	char locked;
	char buffer[GPRS_TX_BUFFER_MAX];
	unsigned int index;
}gprs_tx_data_buff_t;




typedef enum 
{
	GPRS_RX_IDLE=0,
	GPRS_RX_INPROG,
/*
	GPRS_RX_START_GET_LF,
	GPRS_RX_STOP_GET_LF,
	GPRS_RX_MSG,
	GPRS_RX_ENSURE
*/

} GPRS_rx_states;

/*
typedef struct  {
	char elapsed;
	GPRS_rx_states state;
}rx_isr_handler_t;*/

typedef struct  {
	unsigned char elapsed;
	GPRS_rx_states state;
}gprs_rx_isr_handler_t;		

typedef struct
{
	char yy;
	char mm;
	char dd;
	char hr;
	char min;
	char sec;
	char update_time_aval;
}gprs_date_time_t;

#ifdef ONLINE_CONFIG_EN
typedef struct
{
	online_cfg_state_t	online_cfg_state;
	unsigned char		cfg_pending;
	unsigned char		ver_cnt;
	
	unsigned int		industry_id;
	unsigned char		ind_name[INDUS_NAME_LEN];
	unsigned int		daily_limit;
	unsigned char 		daily_limit_dp;
	unsigned long		today_total;
	unsigned long		month_total;
	unsigned long		cum_total;
	unsigned int		calib_factor;
	unsigned int		ADC_offset;
	unsigned int		time_slot;
	flow_meter_make_t flow_meter_make;//unsigned char	flow_meter_make;
	connection_type_t	connection_type;//unsigned char	connection_type;
}online_cfg_t;
#endif
con_status_t gprs_connect(void);
//HJ 02-09-2016
con_status_t gprs_connect_status(void);
//void gprs_disconnect(void);

#ifdef HTTP_ENABLE
void http_handler(void);
http_status_t http_connect (void);
http_status_t http_close(void);
http_status_t post_request(char *, char *);
#endif	//HTTP_ENABLE
int get_rx_data(char*);
//void setGPRSDataReadyFlag(FDM_state_t);
char trylock(char*);

char getGPRSConfigErrSts(void);
void setGPRSConfigErrSts(char);

gprs_status_t getGPRSConnSts(void);
void setGPRSConnSts(gprs_status_t);		
char decode_gprs_response(char *);
char check_string_from_data(const char *, char *, char *);
char rtuPrepareGPRSData(char *, gprs_upload_data_pkt_id_t);
//char get_buff_data(char *);
con_status_t getTimeFromGprs(void);
void gprsConnSoftRst(void);
void gprsConnHardRst(void);
int gprsConnAction(gprs_conn_status);
void gprsConnAfterHardRst(void);
//HJ 17-11-2018
gprs_status_t getGPRSSrvrSts(void);
void setGPRSSrvrSts(gprs_status_t);
char getImei(void);
char *readImei(void);
void set_imei(char *);
#ifdef ONLINE_CONFIG_EN
void init_online_cfg(void);
unsigned char decode_online_resp(char *);
#endif
#endif /* GPRS_H_ */