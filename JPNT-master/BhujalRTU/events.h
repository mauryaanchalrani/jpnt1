/*
 * events.h
 *
 * Created: 10/15/2017 7:10:57 AM
 *  Author: Trilok
 */ 


#ifndef __EVENTS_H__
#define __EVENTS_H__


/*#include "ds1307.h"*/
#include "clock.h"
#include "main.h"

#define EVT_UNUSED_LEN		(40)

//#define VEHICLE_STOPPED_SPD_MIN (3) //KMPL
#define MIN_FILL_RATE		(5)

#define EVT_STRT_HOLD_TIME (1)		
#define EVT_STOP_HOLD_TIME (2)

#define DEFAULT_VEHICLE_SPEED			(30)		//MPH
#define DEFAULT_ROC_NORM_KMPL			(3)
#define DEFAULT_ROC_NORM_LPH			(5)

#define MIN_LVL_DELTA					(500)		//In ML used for long duration events
#define HD_DET_THR						(300)		//In ML used for real time events
#define REFUEL_THRESHOLD				(1000)
#define IGN_OFF_ON_THR					(3*HD_DET_THR)

#define NUM_OF_AVG_SAMPLES				(4)
#define NUM_OF_AVG_SAMPLES_12_MIN		(6)

#define NUM_OF_SPEED_SAMPLES			(3)

#define NORMAL_EVT_START_CODE  (1)
#define HD_EVT_START_CODE		(11)
#define SYSTEM_EVT_START_CODE	(31)
typedef enum
{
	EVT_NONE,					//0. No Event
	// Normal Events
	EVT_REFUEL = NORMAL_EVT_START_CODE,					//1. Speed: 0, Ignition: NA,  Tank Lid: OPEN, ROC: + @ MIN_FILL_RATE
	EVT_TANK_OPEN_NO_REFUEL,	//2. Speed: 0, Ignition: NA,  Tank Lid: OPEN, ROC: 0 over xx time

	//This should always be last number for HD event
	EVT_NORM_MAX_NUM,
	//////////////////////////////////////////////////////////////////////
	
		
	// High Discharge Events	
	EVT_HD_SPD0_FAST = HD_EVT_START_CODE,	//11. Speed: 0, Ignition: OFF, Tank Lid: OPEN,  ROC: - @ Norms
	EVT_HD_SPD0_MEDIUM,	//12. Speed: 0, Ignition: OFF, Tank Lid: OPEN,  ROC: - @ Norms
	EVT_HD_SPD0_SLOW,	//13. Speed: 0, Ignition: OFF, Tank Lid: OPEN,  ROC: - @ Norms
	EVT_HD_SPD0_ACC_ON,			//14. Speed: 0, Ignition: ON,  Tank Lid: OPEN,  ROC: - @ 2xNorms
	EVT_HD_RUNTIME,				//15. Speed:>0, Ignition: ON,  Tank Lid: CLOSE, ROC: - @ 2xNorms
	EVT_ACC_OFF_ON,				//16. Event to trigger if any change between ignition OFF to ON
	EVT_TANK_LID,				//17. Event to check fuel status between tank lid open to close 
	EVT_SPD_ZERO_NON_ZERO,		//18
	
	//This should always be last number for HD event
	EVT_HD_MAX_NUM,
	///////////////////////////////////////////////////////////////////
	
	
	// System Events
	EVT_OVER_VOLTAGE = SYSTEM_EVT_START_CODE,		//31. Over Voltage Detected
	EVT_MAINS_OFF,				//32. Main supply OFF
	
	//This should always be last number for sys event
	EVT_SYS_MAX_NUM,
}event_code_t;



typedef enum
{
	EVT_INIT,
	EVT_DETECT,
	EVT_MONITOR,
	EVT_END,		
}event_state_t;

typedef enum
{
	EVT_FINISH = 'E',
	EVT_START = 'S',
}event_type_t;

/*
typedef struct
{
	unsigned int code;
	event_state_t state;	 
}event_t;
*/

typedef struct
{
	unsigned int transaction_id;		//2
	unsigned char transaction_id_MSB;	//1
	unsigned char evt_code;				//1
	unsigned char evt_type;				//1
	ram_data_t ram_data;				//18
	
	char unused[EVT_UNUSED_LEN];		//30
	unsigned char chksum;				//1
}__attribute__((packed)) FL_evt_log_t;

typedef struct
{
	//URL and device ID will be populated at runtime.
	unsigned char pkt_id;
	//(:) Delimiter populated at runtime
	unsigned char evt_code;				//1
	unsigned char evt_type;				//1
	ram_data_t ram_data;
} __attribute__((packed)) GPRS_evt_data_t;

typedef struct
{
	long avg_samples[NUM_OF_AVG_SAMPLES];	
	long _12min_avg_samples[NUM_OF_AVG_SAMPLES_12_MIN];
	char hd_event_status[EVT_HD_MAX_NUM - HD_EVT_START_CODE];
	unsigned char avg_speed;
	//unsigned char dummy[3];
} __attribute__((packed)) event_common_data_t;

void manage_events(void);
void evt_hd_spd0_accOff_Fast(void);
void evt_hd_spd0_accOff_Medium(void);
void evt_hd_spd0_accOff_Slow(void);
void evt_hd_spd0_accOn(void);
void evt_hd_return_pipe(void);
void evt_refuel(void);
void evt_tankopen_norefuel(void);
void evt_over_voltage(void);

void evt_ignition_off_on(void);
void evt_spd_zero_nonzero(void);
void evt_tank_lid(void);

void update_event_code (unsigned char *, event_code_t);
void update_event_type (unsigned char *, event_type_t);

void get_roc_norms(void);

void clearEventData(void);
void calculate_avg_for_event(void);
void collect_12min_data(long);
void get_avg_speed(void);
	
#endif /* EVENTS_H_ */