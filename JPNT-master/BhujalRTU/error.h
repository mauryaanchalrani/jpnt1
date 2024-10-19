/*
 * error.h
 *
 * Created: 11-12-2013 10:41:11
 *  Author: HemeshJain
 */ 
#ifndef _ERROR_H_
#define _ERROR_H_

//#define NO_ERROR (0)

//leave 1 and 2 for ack and nack
//Command/Checksum Errors
#define ACK		('A')		//R
#define NACK	('N')		//R
//#define ERROR_INVALID_CMD (0x05)

//??
#define ERROR_RFID  (0x02)
#define ERROR_		(0x05)
#define ERROR_CMD	(0x01)

//Data Errors
#define ERROR_INVALID_SR_NUM			(0xA0)
#define ERROR_INVALID_PASS				(0xA1)
#define ERROR_INVALID_SYS_STATE			(0xA2)
#define ERROR_PASS_LEN_MISMATCH			(0xA3)


typedef enum
{
	SYSTEM_OK,
	ERROR_FUEL_LVL,
	
	MAX_ERROR_CODE
}error_code_t;

typedef struct
{
	error_code_t error_code;
	unsigned int error;
}error_sts_t;
/*

//ADC Read Error
#define ERROR_ADC_READ					(0xA4)

//GSM Init Error
#define ERROR_GSM_INIT					(0xA5)

//E2P Errors
#define ERROR_E2P_DATA_MISMATCH (0xE0)
#define ERROR_E2P_CALLIBRATION_RECORD_MISMATCH (0xE1)
#define ERROR_E2P_PRODUCT_SERIAL_NUMBER_INVALID	(0xE2)

//Flash Errors
#define ERROR_FLASH_DATA_RECORD_MISMATCH (0xF0)
#define ERROR_FLASH_REFUEL_RECORD_MISMATCH (0xF1)
#define ERROR_FLASH_FLUSH_REQUIRED (0xF2)

#define ERROR_SYSTEM_HANG (0xFF)		/ *Anand 02.07.2014* /*/
/*

#define BATTERY_FULL_SCALE_VOLT		(13.0)
#define BATTERY_FULL_SCALE_CNT		(1000)				// Because of Resistance value max possible count is 1000 instead of 1023

#define MAIN_BAT_CRITICAL_LVL		((11.5 / BATTERY_FULL_SCALE_VOLT) * BATTERY_FULL_SCALE_CNT)
#define MAIN_BAT_LOW_LVL			((12.0 / BATTERY_FULL_SCALE_VOLT) * BATTERY_FULL_SCALE_CNT)
//HJ 29-11-2016
#define MAIN_BAT_DISP_TIME_CRITICAL_LVL		((10.5 / BATTERY_FULL_SCALE_VOLT) * BATTERY_FULL_SCALE_CNT)
#define MAIN_BAT_DISP_TIME_LOW_LVL			((11.0 / BATTERY_FULL_SCALE_VOLT) * BATTERY_FULL_SCALE_CNT)

#define BACKUP_BAT_LOW_LVL					((11.25 / BATTERY_FULL_SCALE_VOLT) * BATTERY_FULL_SCALE_CNT)
#define BACKUP_BATTERY_SUDDEN_DIP_VLTG		(2.0)
#define BACKUP_BAT_CRITICAL_LVL				(((10.5 - BACKUP_BATTERY_SUDDEN_DIP_VLTG) / BATTERY_FULL_SCALE_VOLT) * BATTERY_FULL_SCALE_CNT)

#define ONE_MINUTE_CNT				(60*(1000/CTRL_STATE_SCAN_TIME))
#define TEN_SECOND_CNT				(10*(1000/CTRL_STATE_SCAN_TIME))					// change one sec to ten sec

//HJ 12-09-2016

#if PUMP_SUPPLY_TYPE == DC_PUMP
#define PUMP_DUTY_CYCLE_MIN_THRESHOLD		(700.0)			//LTR		// HJ 14-08-2017	Change 700 to 700.0 because time per litre calculation of pump duty cycle to work properly
#elif PUMP_SUPPLY_TYPE == AC_PUMP
#define PUMP_DUTY_CYCLE_MIN_THRESHOLD		(1500.0)		//Threshold not require for AC or PTO drive so using maximum number.
#elif PUMP_SUPPLY_TYPE == PTO_PUMP
#define PUMP_DUTY_CYCLE_MIN_THRESHOLD		(9999.0)
#endif

#define PUMP_DUTY_CYCLE_MIN_TIME			(300)		// 5 Min
#define PUMP_DUTY_CYCLE_MAX_TIME			(900)		// 15 Min

//#define PUMP_DUTY_CYCLE_TIME_PER_LTR		((BOWSER_FUEL_DISPENSE_LIMIT - PUMP_DUTY_CYCLE_MIN_THRESHOLD) / (PUMP_DUTY_CYCLE_MAX_TIME - PUMP_DUTY_CYCLE_MIN_TIME))
#define PUMP_DUTY_CYCLE_TIME_PER_LTR		((PUMP_DUTY_CYCLE_MAX_TIME - PUMP_DUTY_CYCLE_MIN_TIME) / (BOWSER_FUEL_DISPENSE_LIMIT - PUMP_DUTY_CYCLE_MIN_THRESHOLD))
*/

/*
//HJ 21-6-2016				// Error bits location for respective fault
typedef enum
{
	SYSTEM_OK,
	MAIN_BAT_CRITICAL,
	MAIN_BAT_LOW,
	BACKUP_BATT_LOW,
	RTC_FAULT,
	
	MAX_ERROR_CODE
}error_code_t;
typedef enum
{
	NO_ERROR,
	NORMAL_ERROR,
	MEDIUM_ERROR,
	FATAL_ERROR,
}error_lvl_t;
typedef enum
{
	DIAG_STS_CHK,
	DAIG_CONFIRM
}diag_state_t;
typedef struct
{
	error_code_t error_code;
	error_lvl_t error_lvl;			// Fatal, Medium and Normal
	unsigned char event_log_en;
	//char error_msg[21];
}sys_error_t;
typedef struct
{
	error_code_t error_code;
	char error_msg[24];
	char error_action[24];
}sys_error_msg_t;*/

/*
typedef struct
{
	unsigned long int sys_error_status;	
	diag_state_t diag_state;
	error_code_t error_code;
}diag_t;

//HJ 21-6-2016				Self diagnostic routines
void diag_backup_batt_check(void);
void self_diagnose (void);

//HJ 09-09-2016
void power_on_backup_batt_check(void);
void power_on_daignostics(void);
void runtime_daignostics(void);
//void update_error_msg(void);
void power_on_check_main_battery(void);
void check_main_battery(void);
void check_backup_battery(void);
error_lvl_t check_error_lvl(error_code_t *);
void notify_operator(error_code_t);

void displayDiagSts(void);							//HJ 10-09-2016
void cal_pump_duty_cycle_elapse_time(void);			//HJ 12-09-2016*/

#endif
