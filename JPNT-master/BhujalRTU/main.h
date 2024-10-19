
#ifndef JPNT_RTU_H_
#define JPNT_RTU_H_


#include <stdio.h>
#include <avr/wdt.h>
#include "_debug.h"
#include "config.h"
#include "uart.h"
#include "gprs.h"
/*#include "ds1307.h"*/
#include "clock.h"


#define FREQ_UNUSED_LEN		(12)
//#define NEW_BOARD
//#define SIMULATED_DATA

#ifdef SIMULATED_DATA
#define MAX_VOLTAGE		(240)
#define MAX_CURRENT		(10)
#define MAX_CURRENT_DP	(5)
#define MAX_PF			(0)
#define MAX_PF_DP		(9)		
#endif

#define MINUTE					(60)
#define FL_RESERVED_BYTE		(4)
#define TM_CFG_UNUSED_LEN		(26)
#ifdef TESTING_MODE
#define RTU_SAVE_LOG_TIME		(1*MINUTE)
#else
#define RTU_SAVE_LOG_TIME		(15*MINUTE)//15 min
#endif

#define RTU_UPLOAD_TIME			(60)
#define MAX_NUM_OF_SWITCH		(4) //VC:20/08/22018
#ifdef TESTING_MODE
#define DB_SAVE_TIME			(3)
#else
#define DB_SAVE_TIME			(15)
#endif
#define TM_IO_CFG_UNUSED_LEN	(5)

//#define CONFIG_SWITCH_PIN	(PINC)

#define MB_TCP_PROTOCOL		(0)
#define MB_TCP_PKT_LNTH		(73)
#define MB_TCP_KEEP_ALIVE_DATA_PKT	(70) 
#define DATA_PKT_LGTH		(73)

#define SYSTEM_ERROR_TIME	(120)
#define GPRS_SOFT_RST_TIME	(14400/SYSTEM_ERROR_TIME) // for 2 min
#define GPRS_HARD_RST_TIME	(72000/GPRS_SOFT_RST_TIME)// for 10 min
#define SERIAL_PENDING_CMD_RSP	(3)
#define NUM_OF_SAMPLES	(5)
#define CONVERT_TO_ML	(1000000UL)
#define CONVERT_TO_KL	(1000000UL)
#define CONVERT_TO_L	(10)
#define CONVERT_HR_TO_SEC	(3600UL)
#ifdef GPRS_ENABLE 
#define VEHICLE_SYNC_UNUSED			(2)
#endif

#define CTRL_STATE_SCAN_TIME	(100)

#define LIMIT_TH	(90)
#define MOTOR_OFF_PERC	(99)
#define MIN_FLOW_RATE	(2)		//2 KL/hr
#define SNOOZE_TIME		((unsigned long)10*60*1000/100)		//10 minute
#ifdef TESTING_MODE
	#define FIRMWARE_DWNLD_TIME	((unsigned long)5*MINUTE)	//2 minutes
#else
	#define FIRMWARE_DWNLD_TIME	((unsigned long)30*MINUTE)	//30 minutes
#endif
#define soft_reset()        \
do                          \
{                           \
	wdt_enable(WDTO_15MS);  \
	for(;;)                 \
	{                       \
	}                       \
} while(0)

typedef enum
{
	RTU_NORMAL,
	RTU_SMS_CONFIG,
	RTU_UART_CONFIG,
	RTU_GPRS_CONFIG,
	RTU_FRMWR_UPDATE,
}RTU_state_t;

typedef enum
{
	PD_DETECT,
	PD_DOWN
}power_status_t;

typedef enum
{
	OVER_LIMIT,		//0
	MAIN_CUTOFF,	//1
	DEVICE_OPEN,	//2
	VALVE_OPEN,		//3
	POWER_ON_BIT,	//4
	FLOW_METER_ERR,	//5
	MOTOR_ON,		//6
}ram_data_status;

typedef enum
{
	SYS_PRODUCT_CONF_ERROR= 1,	// 1
	SYS_GSM_CONF_ERROR ,		// 2
	SYS_GPRS_CONF_ERROR,		// 3
	SYS_CAL_CONF_ERROR,			// 4
	SYS_EM_CONF_ERROR,			// 5
	SYS_RTU_CONF_ERROR,
	
}RTU_sys_error_t;

typedef enum
{
	HEX_FILE_UPDATE_AUTH,
	HEX_FILE_UPDATE_CHK,
	HEX_FILE_UPDATE_PRG,
	HEX_FILE_UPDATE_VERIFY,
	HEX_FILE_UPDATE_RELEASE
}frmwr_update_state_t;


/*
typedef struct
{
	unsigned int system_status;  //char
	unsigned char ip_status;     //char
	unsigned char op_status;     //char
	unsigned int inst_flow;
	unsigned long cum_flow;	
	unsigned int pressure_in;
	unsigned int pressure_out;
}__attribute__((packed)) pump_data_t;*/

/*
typedef struct
{
	int V_R; //unsigned char & int
	unsigned char V_R_dp; //unsigned char & int
	int V_Y;
	unsigned char V_Y_dp;
	int V_B;
	unsigned char V_B_dp;
	unsigned char I_R;
	unsigned char I_R_DP;
	unsigned char I_Y;
	unsigned char I_Y_DP;
	unsigned char I_B;
	unsigned char I_B_DP;
	char PF_R;
	char PF_R_dp;
	char PF_Y;
	char PF_Y_dp;
	char PF_B;
	char PF_B_dp;
	char PF_AVG;
	char PF_AVG_dp;
	unsigned long KWh;
	unsigned char KWh_dp; // char 
	unsigned long KVah;
	unsigned char KVah_dp;
	
}__attribute__((packed)) EM_data_t;
*/

typedef enum
{
	ALARM_OFF,
	ALARM_ON,
	ALARM_SNOOZE,
}alarm_state_t;

enum
{
	BOARD_VER_BIT,
#ifdef	TESTING_MODE
	TESTING_MODE_BIT = 15,
#endif
};


typedef struct
{
		unsigned long int inst_reading;
		unsigned long int todays_total;
		unsigned long int months_total;
		unsigned long int cuml_reading;
		//unsigned long int yest_reading;
		unsigned int status;
		time_stamp_t time;										//6
} __attribute__((packed)) ram_data_t;		// 24 bytes

typedef struct
{
	unsigned int transaction_id_rtu_log;			//2
	unsigned char transaction_id_MSB_rtu_log;		//1

	ram_data_t ram_data;							//24
	
	unsigned char RESERVED[FL_RESERVED_BYTE];		//4
	unsigned char chksum;							//1
} __attribute__((packed)) FL_rtu_log_t;

typedef struct
{
	unsigned char cnt;
	unsigned int unsent_rtu_logs_GPRS;	
	char unsued[FREQ_UNUSED_LEN];	
	char chksum;
}__attribute__((packed)) freq_updated_data_t;

typedef struct
{
	//URL and device ID will be populated at runtime.
	unsigned char pkt_id;
	//(:) Delimiter populated at runtime
	ram_data_t ram_data;
	unsigned char pkt_ready;
} __attribute__((packed)) GPRS_rtu_data_t;

/*
typedef struct
{	
	unsigned char rtu_MB_slav_Addr;				//1
	
	unsigned char unused[TM_CFG_UNUSED_LEN];	//26
	unsigned char chksum;						//1
}__attribute__((packed))rtu_config_t;*/

typedef struct
{
	unsigned char keep_alive;
	unsigned char ip_status_flash;				//char
	unsigned char op_status_flash;				//char
	unsigned char unused[TM_IO_CFG_UNUSED_LEN];	//5
}__attribute__((packed))rtu_control_data_t;

typedef struct
{
	unsigned long kwh_offset;
	unsigned long kwah_offset;
	unsigned char chksum;	
}__attribute__((packed))em_data_t;

typedef struct  
{
	unsigned long sample_1s[5];
	unsigned long ml_s;
	unsigned long l_hr;
	unsigned long temp;
	unsigned long avg;
}sensor_data_t;

typedef struct
{
	unsigned int pulse_flow_meter_constant;
	unsigned int _420ma_flow_meter_constant;
	unsigned int adc_flow_meter_constant;
}calibration_const_t;

typedef struct
{
	time_stamp_t last_update_time;
	unsigned char checksum;
	unsigned char unused[VEHICLE_SYNC_UNUSED];
}__attribute__((packed)) vehicle_sync_time_t;


typedef struct
{
	unsigned int rtu_config;
}rtu_data_t;

typedef struct
{
	frmwr_update_state_t frmwr_update_state;
	unsigned char choice;
}manage_frmwr_updt_t;

typedef enum
{
	CHECK_ALERT,
	SMS_ALERT_90,
	SMS_ALERT_100,
	NO_ALERT,
}sms_alert_state_t;

typedef enum 
{
	GOT_TIME_FROM_GPRS, 
	IDLE,
} rtc_state_t;


void check_sms_threshold(void);

void init_lcd(void);
void gpioInit(void);
void bootingUp(void);
void writeDefaults(void);
//void write_default_rtu_config(rtu_config_t *rtu_config);
void read_IO_status(void);
void read_flow_data(void);
void read_pressure_data(void);

void updateLocalDesplay(void);
void updateRamData(void);
gprs_upload_data_pkt_id_t controlRTUGprsUpload(void);
void getAllCfgData(void);
void save_FL_rtu_data(void);
void prepare_FL_LogData(void);
void prepare_e2p_logdata(void);
void update_online_data(void);
void init_rtu_ctrl_data(void);
void factory_defaults(void);
void decrement_unsent_log_cnt(char upload_code);
char check_unsent_log(void);
//vCheckifWeCanDisableIntr(void);
void  setRtuState( RTU_state_t);
RTU_state_t  getRtuState( void );
void readButton(void);
//int readSwitch(void);
uint16_t digitalReadSwitch(uint16_t);
char rtuPrepareGPRSData_Mb(char *,int *, gprs_upload_data_pkt_id_t );

void convertFloatToChar(char *,char*,int);
gprs_conn_status monitorGprsConn(void);
void update_energy_data(void);
void getInstFlow(float);
void updateFlowRamData(void);
void checkForAlert(void);
void check_for_day_month_change(void);
void get_last_rtu_log(void);
void check_for_power_down(void);
void sw_led_cntrl(void);
void gprsPowerUpInit(void);

void set_frmwr_update_state(frmwr_update_state_t);
frmwr_update_state_t get_frmwr_update_state(void);
unsigned int get_rtu_cnfg(void);
void manage_firmware_update(void);
unsigned char get_frmwr_choice(void);
void set_frmwr_choice(unsigned char);
void controlRTU(void);
void alarm_handler();
void motor_handler();
unsigned char check_alarm();
void set_dummy_values();
void clear_ram_logs(void);
void online_firmware_download(void);
void reset_avr();
void jump_to_bootloader();
void WDT_off(void);
void updt_sys_config(void);

//abdul majeed
void prepare_e2p_y_total(void);
void get_last_y_tot(void);
void check_for_day_month_change_y_tot(FL_rtu_log_t *);

void check_for_MOTOR_POWER(void);//AM 19-07-21
#ifdef	SUPERCAP
unsigned char check_mains_cut(void);//AM 22-07-2021
#endif
void save_mains_off_logs(void);//AM 22-07-2021
#if 0
void get_last_time_log(void);
void update_time_from_log(void);
#endif
void compare_time_from_log(void);//AM
void set_rtc_state(rtc_state_t);//AM
rtc_state_t get_rtc_state();
#endif /* JPNT_RTU_H_ */