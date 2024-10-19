/*
 * ext_interrupt.cpp
 *
 * Created: 02-05-2018 03:19:11 PM
 *  Author: OWNER
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ext_interrupt.h"

//volatile unsigned int timer_pulse_cnt = 0;
//extern instaneous_sensor_data_t instaneous_sensor_data;

volatile unsigned int curr_pulse_cnt = 0;
void init_ext_interrupt(void)
{
	EIMSK |= (1 << INT0);
	EICRA |= (1 << ISC01); // faling edge interrupt
}

ISR(INT0_vect)
{
	cli();	
	curr_pulse_cnt++;
	sei();
}