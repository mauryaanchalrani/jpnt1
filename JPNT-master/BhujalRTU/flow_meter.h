/*
 * flow_meter.h
 *
 * Created: 20-01-2021 5.21.32 PM
 *  Author: Shubham
 */ 


#ifndef FLOW_METER_H_
#define FLOW_METER_H_


#define FM_RESP_TIMEOUT			(2000 / 100)

enum
{
	FM_NO_NEW_MSG,
	FM_MATCH_FAIL,
	FM_MATCH_OK,
};

typedef enum
{
	FM_READ_CURR_FLOW,
	FM_READ_NET_FLOW,
	FM_RSP,
	FM_WAIT,
}fm_state_t;

typedef struct  
{
	unsigned long curr_flow;
	unsigned long net_flow;
}fm_instant_data_t;

typedef struct
{
	fm_instant_data_t fm_instant_data;
	fm_state_t fm_state;
	fm_state_t fm_cbst;
}flow_meter_t;



void initFMSensor();
void readFMSensor();
void decodeFMData();
void readFMSensorADC();
char get_rx_data_atg(char *copy_here);
char check_string_fm(const char *str, char *copy_here, int *numbytes);
#endif /* FLOW_METER_H_ */