/*
 * daily_limit.h
 *
 * Created: 02/11/2022 13:50:55
 *  Author: ADMIN
 */ 


#ifndef DAILY_LIMIT_H_
#define DAILY_LIMIT_H_

typedef struct
{
	unsigned int industry_id;
	unsigned int jpnt_limit;
	unsigned int rspcb_limit;
}daily_limit_t;



unsigned int get_daily_limit(unsigned int id);
unsigned int read_rom_mem(char *ptr);
#endif /* DAILY_LIMIT_H_ */