#ifndef __CLOCK_H__
#define __CLOCK_H__

typedef struct
{
	char yr;
	char mm;
	char dd;
	
	char hr;
	char min;
	char sec;		//K & A
	
}__attribute__((packed)) time_stamp_t;		//Anand 19-9-15

typedef struct
{
	char dan;
	char ampm, sec, minute, hour, date, month, year;
	//char rtc_read_error;
}__attribute__((packed)) time_main_t;

void getDateTime(void);
void get_present_time(time_stamp_t *);
void set_rtc(unsigned char *);
void time_sync(void);
unsigned char check_date_time(char *str, char *arr);
void updatetimefromegprs(time_stamp_t *);

void gettime_from_time_main(time_stamp_t *time_stamp);	//abdul majeed
void gettime_from_gprs(time_stamp_t *time_stamp);

#endif