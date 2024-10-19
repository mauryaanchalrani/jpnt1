#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "app_uart.h"
#include "common.h"
#include "config.h"
#include "_debug.h"
#include "pins.h"
#include "pulser.h"
#include "sensor.h"


volatile unsigned long int pulser_raw_cnt;
extern calbration_data_t calbration_data;

unsigned long int getPulserCnt(void)
{
	return pulser_raw_cnt;
}

void clrPulserCnt(void)
{
	pulser_raw_cnt = 0;
}

float convert_to_litre(unsigned long int pulse)	
{
	float retVal = 0;
	//retVal = (float)pulse/CALIBRATION_CONST;
	
	// need to update for as new struct
	//retVal = ((float)pulse)*(5000.0/calbration_data.cal_data_flow/1000.0); // right now we are adding for every 200 tick is like 1 ltr.
	
	return 	retVal;
}

void initPulser(void)
{
	pulser_raw_cnt = 0;
	
	EICRB |= (1<<PULSER_ISC); // enable extenral interrupt
	EIMSK |= (1<<PULSER); // mask
	//EIFR |= (1<<INT4);	//flag resistor	
	/*EICRB |= (1<<PULSER1_ISC);
	EIMSK |= (1<<PULSER1);*/   
	PULSER_ON;
	 
}

void pulserON(void)
{
	clrPulserCnt();
	PULSER_ON;	
	PULSER1_ON; 
	EIMSK |= (1<<PULSER);
	EIMSK |= (1<<PULSER1);
}

void pulserOFF(void)
{	
	clrPulserCnt();
	PULSER_OFF;		
	PULSER1_OFF;    
	EIMSK &= (~(1<<PULSER));
	EIMSK &= (~(1<<PULSER1));
}

ISR(INT4_vect)
{
#ifdef _DEBUG_PULSER
	debug_pulser_1++;
#endif
	cli();
	pulser_raw_cnt++;
	//pulser.pulsertimeout = 0;
	sei();
}

ISR(INT5_vect)
{
#ifdef _DEBUG_PULSER
	debug_pulser_2++;
#endif
	cli();
	pulser_raw_cnt++;
	//pulser.pulsertimeout = 0;	
	sei();
}
