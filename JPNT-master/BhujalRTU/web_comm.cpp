/*
 * web_comm.cpp
 *
 * Created: 01-11-2017 09:58:04
 *  Author: HemeshJain
 */
 
#include "web_comm.h"
#include "gprs.h"

#include "config.h"


#include "app_uart.h"

#include "sms.h"
#include "pins.h"


extern gprs_t gprs;


conn_state_t conn_state = CONNECT_POWER_ON;
unsigned int gps_read_timeout = 0;
unsigned int gprs_read_timeout = 0;


extern WEB_data_u WEB_data;
void manage_gps_gprs(void)
{
	
	switch (conn_state)
	{
		case CONNECT_POWER_ON:
		{
			if(gprs_power_toggle())
			{
				#ifdef GPRS_DEBUG
				UWriteString("gprs on",UART0);
				#endif
				
				conn_state = CONNECT_DATA_UPLOAD;
				
				
			}
			else
			{
				
			}

		}
		break;
		case CONNECT_DATA_UPLOAD:
		{

#ifdef HTTP_ENABLE

			http_handler();
#endif	//HTTP_ENABLE
			
			if((gps_read_timeout++ >= GPS_READ_RATE))
			{

		
				if (gprs.state != GPRS_LOGS_UPLOAD)
				{
					
					gps_read_timeout = 0;
				}	
			}
			
		}
	
		break;
		default:
		
		break;
	}
}

unsigned char gprs_power_toggle()
{
	static sim_reset_state_t sim_reset_state = SIM_RESET_PIN_HIGH;
	static unsigned int wait_time = 0;
	char power_state = FALSE;
	switch(sim_reset_state)
	{
		case SIM_RESET_PIN_HIGH :
		
			
			GPRS_PWRKEY_PORT &= ~(1<<GPRS_PWRKEY_PIN);
			
			sim_reset_state = SIM_RESET_WAIT;

		break;
		
		case SIM_RESET_WAIT:
			if(wait_time++ >= SIM_MODULE_DELAY_TIME)			//set waiting time
			{
				sim_reset_state = SIM_RESET_PIN_LOW;
				wait_time = 0;
			
			}
		break;
		
		case SIM_RESET_PIN_LOW :
		
			
			sim_reset_state = SIM_RESET_PIN_HIGH;
			GPRS_PWRKEY_PORT |= (1<<GPRS_PWRKEY_PIN);
		
			//initialize the gps and gprs routines
			power_state = TRUE;
		
		break;
		
		default:
		
		break;
		
		
	}
	
	return power_state;
	
}

void check_gsm_status()
{
	static unsigned int check_time = 0;
	
	{
		if(++check_time >= 5000/100)
		{
			check_time = 0;
			conn_state = CONNECT_POWER_ON;
		}
	}

}

void set_conn_state(conn_state_t state)
{
	conn_state = state;
}

conn_state_t get_conn_state(void)
{
	return conn_state;
}

void set_live_pckt_ready(unsigned char val)
{
	WEB_data.GPRS_rtu_data.pkt_ready = val;
}

unsigned char get_live_pckt_ready(void)
{
	return WEB_data.GPRS_rtu_data.pkt_ready;
}
