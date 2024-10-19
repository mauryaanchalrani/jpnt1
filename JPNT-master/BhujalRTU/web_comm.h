#ifndef __WEB_COMM_H__
#define __WEB_COMM_H_

#include "main.h"
#include "events.h"

#define WEB_STATE_MC_TIME	(WEB_COMMS_SCHEDULAR_TIME)
#define GPS_READ_RATE		((/*UPLOAD_RATE_ONLINE*/10 * 1000)/ WEB_STATE_MC_TIME)
#define GPS_FAIL_TIMEOUT	(1000/WEB_STATE_MC_TIME)
#define SIM_MODULE_DELAY_TIME ((1200)/WEB_STATE_MC_TIME)	//24/1/19 SK

typedef enum
{
	CONNECT_NO_STATE,
	CONNECT_POWER_ON,
	//CONNECT_LOCATION,
	CONNECT_DATA_UPLOAD,
	CONNECT_MSG,
}conn_state_t;

typedef enum
{
	SIM_RESET_PIN_HIGH,
	SIM_RESET_WAIT,
	SIM_RESET_PIN_LOW
}sim_reset_state_t;

typedef union
{
	GPRS_rtu_data_t GPRS_rtu_data;
	//GPRS_evt_data_t GPRS_evt_data;
}__attribute__((packed)) WEB_data_u;


void manage_gps_gprs(void);

unsigned char gprs_power_toggle();
void check_gsm_status();
void set_conn_state(conn_state_t state);
conn_state_t get_conn_state(void);
void set_live_pckt_ready(unsigned char);
unsigned char get_live_pckt_ready(void);
#endif