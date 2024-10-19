/*
 * timers.c
 *
 * Created: 22-11-2012 12:59:58
 *  Author: HemeshJain
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer.h"
#include "common.h"
#include "schedular.h"
#include "error.h"
#include "main.h"
#include "global.h"
#include "_debug.h"		//Anand 28-4-15
#ifdef _DEBUG_WIFI_TIMER		//Anand 28-4-15
#include "app_uart.h"
#endif
#ifdef GPRS_ENABLE
#include "gprs.h"		//Anand 06-04-16
#endif


/* Define flags for schedular */
volatile schedular_flg_t schedular_flg;
volatile unsigned char uart_rx_timeout = 0;
volatile unsigned char uart_rx_timeout_flg = FALSE;

/*For millis()*/
volatile unsigned int _cnt = 0;
extern volatile unsigned int curr_pulse_cnt;		// 22-2-19 VC:
extern volatile Rx_Buff_t Rx_Buff[3];

#ifdef GPRS_ENABLE
//extern rx_isr_handler_t rx_isr_handler;		//Anand 07-04-16
extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;		//Anand 07-04-16
#endif

/*#if CONN_TYPE == PULSE
	extern sensor_data_t sensor_data;
#endif*/
/* Initialize timer 0 for running a scheduler of 10 msec*/
void timer0Init (void)
{
	// initialize timer 0
	timer0SetPrescaler( TIMER_CLK_DIV1024/*TIMER0PRESCALE*/ );	// set prescaler
	//outb(TCNT0, 0);							// reset TCNT0
	TCNT0 = 0;
	//sbi(TIMSK0, TOIE0);						// enable TCNT0 overflow interrupt
	TIMSK0 |= (1<<TOIE0);
	//outb(TCNT0, ( TIMERRTC_TOP_VALUE - TIMER0_INIT_VALUE));		//Initialize with a prefix time
	TCNT0 = ( TIMERRTC_TOP_VALUE - TIMER0_INIT_VALUE);
	schedularInit();
}

void timer0SetPrescaler(unsigned char prescale)
{
	unsigned char timer_control_bit;
	// set prescaler on timer 0
	//outb(TCCR0B, (inb(TCCR0B) & ~TIMERRTC_PRESCALE_MASK) | prescale);
	timer_control_bit = ((TCCR0B) & ~TIMERRTC_PRESCALE_MASK) ;
	TCCR0B |= (timer_control_bit  | prescale);
}

/**/
void schedularInit( void )
{
	schedular_flg.flg_10ms = FALSE;
	schedular_flg.flg_50ms = FALSE;
	schedular_flg.flg_100ms = FALSE;
	schedular_flg.flg_1sec = FALSE;
}

/* Design the scheduler to perform different task at different instances */
ISR(TIMER0_OVF_vect)
{
	//static unsigned char cnt_10ms = 0;		//Anand 08-07-15
	static unsigned char cnt_50ms = 0;
	static unsigned char cnt_100ms = 0;
	static unsigned int cnt_1sec = 0;
	/*static unsigned char sys_hang_duration = 0;*/
	//cli();
	//outb(TCNT0, ( TIMERRTC_TOP_VALUE - TIMER0_INIT_VALUE));
	TCNT0 = ( TIMERRTC_TOP_VALUE - TIMER0_INIT_VALUE);
	sei();
	/*if (uart_rx_timeout++ > UART_RX_TIMEOUT)
	{
		uart_rx_timeout = 0;
		uart_rx_timeout_flg = TRUE;
	}*/
	
	++_cnt;		//Anand 29.09.2014
	
#ifdef GPRS_ENABLE
	if(gprs_rx_isr_handler.elapsed < 0xFF)
		gprs_rx_isr_handler.elapsed++;		//Anand 07-04-16
#endif

	if(Rx_Buff[UART_485].elapsed < 0xFF)
		Rx_Buff[UART_485].elapsed++;


	/* 10msec flag check to perform 10msec task */
	//if(cnt_10ms++ > SCHEDULE_10MS_CNT)
	{
		//cnt_10ms = 0;		//Anand 08-07-15
		schedular_flg.flg_10ms  = TRUE;
	}
	/* 50msec flag check to perform 10msec task */
	if(cnt_50ms++ >= SCHEDULE_50MS_CNT)
	{
		cnt_50ms = 0;
		schedular_flg.flg_50ms  = TRUE;
	}
	/* 100msec flag check to perform 100msec task */
	if(cnt_100ms++ >= SCHEDULE_100MS_CNT)
	{
		cnt_100ms = 0;
		schedular_flg.flg_100ms = TRUE;
	}
	/* 1sec flag check to perform 1sec task */
	if(cnt_1sec++ >= SCHEDULE_1SEC_CNT)
	{
		cnt_1sec = 0;
		
//#if CONN_TYPE == PULSE
//		sensor_data.sample_1s[0] = curr_pulse_cnt;
		curr_pulse_cnt = 0;
//#endif
		
		schedular_flg.flg_1sec = TRUE;
	}
	//sei();
}

unsigned int millis()
{
	return _cnt*TIMER0_VALUE;
}


//Anand 17-4-15
void timer1SetPrescaler(unsigned char prescale)
{
	unsigned char timer_control_bit;
	// set prescaler on timer 1
	//outb(TCCR1B, (inb(TCCR1B) & ~TIMERRTC_PRESCALE_MASK) | prescale);
	timer_control_bit = ((TCCR1B) & ~TIMERRTC_PRESCALE_MASK) ;
	TCCR1B |= (timer_control_bit | prescale);
}

#ifdef WIFI_ENABLE
/* Initialize timer 1 for running a scheduler of 50 msec*/
void timer1Init (void)
{
	// initialize timer 1
	timer1SetPrescaler( TIMER_CLK_DIV1024/*TIMER1PRESCALE*/ );	// set prescaler
	outb(TCNT1, 0);							// reset TCNT1
	sbi(TIMSK1, TOIE1);						// enable TCNT1 overflow interrupt
	outb(TCNT1, ( TIMER_TOP_VALUE - WIFI_TIMER_INIT_VALUE));		//Initialize with a prefix time
}
/*#endif

#ifdef WIFI_ENABLE*/
ISR(WIFI_TIMER_OVF_VECT)
{
	cli();
	
#ifdef _DEBUG_WIFI_TIMER
	UWriteData('T', WIFI_UART);
#endif
	
	//Anand 07-03-15
	if(wifiModule.currCmd != NO_CMD/*Rx_Buff[UART_PC].GPRS_rx_state == RX_BUFFERING*/)
	{
		switch(wifiModule.currCmd)
		{
			//case CIPSTART_TCP :		//todo Depend on size of Data
			case CWJAP :
			case CWSAP :
				if(wifiRxIdleTime++ > (WIFI_RX_MAX_IDLE_TIME*40))	//3sec It was 30		//5 Sec	//It was 100
				{
					//PORTB ^= 0xFF;
					wifiRxIdleTime = 1;
					wifiRxStateBeforeServerConnect = RX_COMPLETED;
					decodeWifiRxData();
				}
			break;

			//case TRANSMIT_DATA :		//todo Depend on size of Data
			case CIPSTART_TCP :		//todo Depend on size of Data
			case CIFSR :
				if(wifiRxIdleTime++ > (WIFI_RX_MAX_IDLE_TIME*60))//60		//3 Sec
				{
					//PORTB ^= 0xFF;
					wifiRxIdleTime = 1;
					wifiRxStateBeforeServerConnect = RX_COMPLETED;

					/*extern volatile uint8_t UARTBuffer[NO_OF_UART][BUFSIZE];
					lcd_clear();
					lcd_putNumber(uart3NumOfResponse); lcd_putc(' '); lcd_putNumber(uart3RxIndx);
					lcd_gotoxy(2, 1);
					for(wifiRxIdleTime=0 ; wifiRxIdleTime<16 ; wifiRxIdleTime++)
						lcd_putc(UARTBuffer[UART3][wifiRxIdleTime]);
					Delay(2000);*/
					decodeWifiRxData();
				}
			break;

			case CIPCLOSE :
				if(wifiRxIdleTime++ > (WIFI_RX_MAX_IDLE_TIME*40))//2		//100 mSec
				{
					//PORTB ^= 0xFF;
					wifiRxIdleTime = 1;
					wifiRxStateBeforeServerConnect = RX_COMPLETED;

					/*extern volatile uint8_t UARTBuffer[NO_OF_UART][BUFSIZE];
					lcd_clear();
					lcd_putNumber(uart3NumOfResponse); lcd_putc(' '); lcd_putNumber(uart3RxIndx);
					lcd_gotoxy(2, 1);
					for(wifiRxIdleTime=0 ; wifiRxIdleTime<16 ; wifiRxIdleTime++)
						lcd_putc(UARTBuffer[UART3][wifiRxIdleTime]);
					Delay(2000);*/
					decodeWifiRxData();
				}			
				break;
			//case TRANSMIT_DATA :
			case CIPSEND :		//aNAND 10-4-15
				if(wifiRxIdleTime++ > (WIFI_RX_MAX_IDLE_TIME*2))//2		//100 mSec
				{
					//PORTB ^= 0xFF;
					wifiRxIdleTime = 1;
					wifiRxStateBeforeServerConnect = RX_COMPLETED;

					/*extern volatile uint8_t UARTBuffer[NO_OF_UART][BUFSIZE];
					lcd_clear();
					lcd_putNumber(uart3NumOfResponse); lcd_putc(' '); lcd_putNumber(uart3RxIndx);
					lcd_gotoxy(2, 1);
					for(wifiRxIdleTime=0 ; wifiRxIdleTime<16 ; wifiRxIdleTime++)
						lcd_putc(UARTBuffer[UART3][wifiRxIdleTime]);
					Delay(2000);*/
					decodeWifiRxData();
				}
			break;
			/*case AT_RST :		//aNAND 10-4-15
				if(wifiRxIdleTime++ > (WIFI_RX_MAX_IDLE_TIME*10))//2		//100 mSec
				{
					//PORTB ^= 0xFF;
					wifiRxIdleTime = 1;
					wifiRxStateBeforeServerConnect = RX_COMPLETED;

					/ *extern volatile uint8_t UARTBuffer[NO_OF_UART][BUFSIZE];
					lcd_clear();
					lcd_putNumber(uart3NumOfResponse); lcd_putc(' '); lcd_putNumber(uart3RxIndx);
					lcd_gotoxy(2, 1);
					for(wifiRxIdleTime=0 ; wifiRxIdleTime<16 ; wifiRxIdleTime++)
						lcd_putc(UARTBuffer[UART3][wifiRxIdleTime]);
					Delay(2000);* /
					decodeWifiRxData();
				}*/
			break;
			default :
				if(wifiRxIdleTime++ > WIFI_RX_MAX_IDLE_TIME)		//50 mSec	//changed to 150 mSec
				{
					wifiRxIdleTime = 1;
					wifiRxStateBeforeServerConnect = RX_COMPLETED;
					decodeWifiRxData();
				}
			break;
		}
	}
	outb(TCNT1, ( TIMER_TOP_VALUE - WIFI_TIMER_INIT_VALUE));		//Initialize with a prefix time
	//~Anand 07-03-15
	
	sei();
}
#endif
//~Anand 07-03-15
