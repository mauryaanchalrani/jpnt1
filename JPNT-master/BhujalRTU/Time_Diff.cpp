/*
 * Time_Diff.cpp
 *
 * Created: 18-May-15 4:14:45 PM
 *  Author: Kamlesh
 * Description- 
 */ 

#include "config.h"
#include "Time_Diff.h"
 #include "_debug.h"
 #include "app_uart.h"
 #include "common.h"

time_stamp_t last_dispense_time;
time_stamp_t end_time;


int isLeapYear(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int getDaysOfMonth(int month, int year) 
{
	if (month == FEB) 
	{
		return isLeapYear(year) ? 29 : 28;
	}

	switch (month)
	{
		case JAN: case MAR: case MAY: case JUL: case AUG: case OCT: case DECMB:
		return 31;
		case APR: case JUN: case SEP: case NOV:
		return 30;
		default:
		return 0;
	}
}



int timestampToMinutes(time_stamp_t ts)			//neetu parihar
{
	int totalMinutes = 0;

	totalMinutes += ts.min;
	totalMinutes += ts.hr * 60;
	totalMinutes += ts.dd * 24 * 60;
	for (int month = JAN; month < ts.mm; ++month)
	{
		totalMinutes += getDaysOfMonth(month, ts.yr) * 24 * 60;
	}
	for (int year = 1; year < ts.yr; ++year)
	{
		totalMinutes += (isLeapYear(year) ? 366 : 365) * 24 * 60;
	}


	return totalMinutes;
}

timeDiff_t calcTimeDiff(time_stamp_t start, time_stamp_t stop)		//neetu parihar
{
	timeDiff_t diff = {0, 0, 0};

	int startMinutes = timestampToMinutes(start);
	int stopMinutes = timestampToMinutes(stop);

	int totalMinutes = stopMinutes - startMinutes;
	
	if (totalMinutes < 0)
	{
		time_stamp_t temp = start;
		start = stop;
		stop = temp;
		totalMinutes = -totalMinutes;
	}
	diff.diffDays = totalMinutes / (24 * 60);
	totalMinutes %= (24 * 60);
	diff.diffHrs = totalMinutes / 60;
	diff.diffMins = totalMinutes % 60;
	

	return diff;
}
