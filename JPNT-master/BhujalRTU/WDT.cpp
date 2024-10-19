/*
 * WDT.cpp
 *
 * Created: 24-06-2021 3.52.04 PM
 *  Author: Shubham
 */ 

#include "WDT.h"

void WDT_Init(void)
{
	cli();
	wdt_reset();
	/* Start timed equence */
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	/* Set new prescaler(time-out) value =  (~8 s) */
	WDTCSR = (1<<WDE) | (1<<WDP3) | (1<<WDP0);
	sei();
}
