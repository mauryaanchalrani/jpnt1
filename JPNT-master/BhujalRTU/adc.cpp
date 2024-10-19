#include <avr/io.h>
#include <util/delay.h>
#include "adc.h"
#include "config.h"
//#include "pins.h"

void initADC()
{
	// enable ADC, select ADC clock = F_CPU / 128 (i.e. 62.5 kHz)
	ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
#if BOARD_VER == v2_0
	ADMUX |= (1<<REFS0);	// | (1<<REFS1);// | (1<<ADLAR);		//Anand 07.06.2014
#endif
#if BOARD_VER == v1_0
	ADMUX |= (1<<REFS0) | (1<<REFS1);
#endif
	_delay_ms(10);
}

// HJ 18-12-2016
void disable_ADC(void)
{
	// enable ADC, select ADC clock = F_CPU / 128 (i.e. 62.5 kHz)
	ADCSRA &= ~((1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0));
	ADMUX &= ~(1<<REFS0) ;//| (1<<REFS1);// | (1<<ADLAR);		//Anand 07.06.2014
}
uint16_t readADC(char ch)
{
	/*unsigned int timeout=0;
	static int diesel = 200;*/
	
	//HJ 09-09-2016
	ADMUX &= 0xE0;						// Clear MUX4 - MUX0
	
	ADMUX |= (ch & 0x07);
											// Conversion on channel 0, AVCC reference, 10 bit mode
	ADCSRA |= (1<<ADSC);                // Start conversion
	
	while(!(ADCSRA&(1<<ADIF)))         // Wait for conversion complete
	{
		/*if(timeout++>=1000)				//TT 13 Sep 2017
		{
			diesel--;
			if(diesel <=0)
			{
				diesel = 200;
			}
			return diesel;
		}*/
	}
	
	ADCSRA |= (1 << ADIF);
	
	return(ADC);
}
