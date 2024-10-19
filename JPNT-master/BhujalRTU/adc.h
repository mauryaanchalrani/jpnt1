#ifndef _ADC_H
#define _ADC_H

#define FM_420_CH (0)		//for E4E RTU
//#define FM_420_CH (1)		//for arduino
#define CH1 (1)
#define CH2 (2)
#define CH3 (3)
#define CH4 (4)
#define CH5 (5)
#define CH6 (6)
#define CH7 (7)

#define MAX_ADC_VALUE (1020)

void initADC();
unsigned int readADC(char);
//HJ 19-12-2016
void disable_ADC(void);

#endif
