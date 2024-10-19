/*
 * Time_Diff.h
 *
 * Created: 18-May-15 4:15:04 PM
 *  Author: Kamlesh
 */ 

#include "config.h"
#include "clock.h"


#ifndef TIME_DIFF_H_
#define TIME_DIFF_H_

#define TCF_MAX_AMT		(100.0)	//Time corr Factor for 24 hrs, Should be in float
#define TCF_MAX_TIME	(48)	//After this period apply corr of Max amt*1.5

typedef enum 
{
	JAN = 1,
	FEB,
	MAR,
	APR,
	MAY,
	JUN,
	JUL,
	AUG,
	SEP,
	OCT,
	NOV,
	DECMB
} Month;

typedef struct
{
	char diffHrs;
	char diffMins;
	long  diffDays;
}timeDiff_t;

typedef struct 
{
	time_stamp_t last_dispense_time;
	int corr_factor;
}TCF_t;

int isLeapYear(int year);
unsigned int getDaysOfMonth(char , unsigned int );
int getDaysOfMonth(int month, int year);
int timestampToMinutes(time_stamp_t ts) ;//neetu parihar;
timeDiff_t calcTimeDiff(time_stamp_t start, time_stamp_t stop);

#endif /* TIME_DIFF_H_ */
//#endif