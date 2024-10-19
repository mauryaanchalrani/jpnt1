#ifndef PULSER_H
#define PULSER_H

#include <avr/io.h>
#include "pins.h"

//#define PULSER_DIVIDE_CONST 33.25			


#define PULSER_ON   (EIFR |= (1<<PULSER)) // PE4
#define PULSER_OFF	(EIFR &= (~(1<<PULSER)))
#define PULSER1_ON   (EIFR |= (1<<PULSER1))  //PE5
#define PULSER1_OFF	(EIFR &= (~(1<<PULSER1))) 

#define PULSER_ISC  (ISC41) // interrupt pin E4
#define PULSER1_ISC  (ISC51) // interrupt pin E5
/*
//#define TICKS_PER_LTR (73)			//For Calibration Change this Only  changed from 100 to 125 by rohit

#ifndef TICKS_PER_LTR
#define TICKS_PER_5_LTR (980)
#endif*/

//#define JERK_OFFSET (0)//(0.400)			//In Lt., Depends on Ticks Per Lt.
//#define ML_PER_TICK (1000.0/TICKS_PER_LTR)


void initPulser(void);		
unsigned long int getPulserCnt(void);
void clrPulserCnt(void);
float convert_to_litre(unsigned long int);
unsigned long convertToPulse(float);
void pulserON(void);
void pulserOFF(void);
#ifdef _DEBUG_PULSER
unsigned long int getDebugPulserCnt(void);
#endif

#endif