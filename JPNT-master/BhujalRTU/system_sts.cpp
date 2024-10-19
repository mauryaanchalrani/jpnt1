/*
 * system_sts.cpp
 *
 * Created: 18-10-2017 16:36:51
 *  Author: HemeshJain
 */ 

#include "main.h"
#include "system_sts.h"
#include "pins.h"
#include "always.h"


//static imv_state_t imv_state;
//HJ 21-09-2017
/*
void update_sts_bit(sts_bits_t bit_no, int sts)
{
	if (sts)
	{
		lvl_data.sts |= (1 << bit_no);
	}
	else
	{
		lvl_data.sts &= ~(1 << bit_no);
	}
}
void check_alive(void)
{
	if (getGPRSConnSts())
	{
		update_sts_bit(STS_ALIVE, HIGH);
	}
	else
	{
		update_sts_bit(STS_ALIVE, LOW);
	}
}

void update_system_sts(unsigned int *sts)
{
	*sts = lvl_data.sts;
}
*/