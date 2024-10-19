/*
 * flow_meter.cpp
 *
 * Created: 20-01-2021 5.21.16 PM
 *  Author: Shubham
 */ 
#include "flow_meter.h"
#include "uart.h"
#include "common.h"
#include "app_uart.h"
#include <stdio.h>
#include <string.h>
#include "_debug.h"
#include <stdlib.h>
#include <ctype.h>
#include "main.h"
#include "adc.h"
#include "clock.h"
#include "app_eeprom.h"
flow_meter_t flow_meter;
extern volatile Rx_Buff_t Rx_Buff[4];
extern volatile Tx_Buff_t Tx_Buff[4];
extern sensor_data_t sensor_data;
extern product_info_t product_info;	
extern ram_data_t ram_data;
unsigned char fm_error_cnt = 0;

void initFMSensor()
{
	flushTxBuffer(UART_485);
	flushRxBuffer(UART_485);
	flow_meter.fm_state = FM_WAIT;
	flow_meter.fm_cbst = flow_meter.fm_state;
	flow_meter.fm_state = FM_READ_CURR_FLOW;
}

void readFMSensor()
{
	static unsigned int fm_timeout;

	switch(flow_meter.fm_state)
	{
		case FM_READ_CURR_FLOW:
		{

			sprintf((char *)&Tx_Buff[UART_485].tx_buffer[0],"Read 1>\r1\r");
			Tx_Buff[UART_485].tx_indx = strlen((const char *)Tx_Buff[UART_485].tx_buffer);
			Tx_Buff[UART_485].tx_ready = TRUE;
			flow_meter.fm_cbst = flow_meter.fm_state;
			flow_meter.fm_state = FM_RSP;
		}
		break;
		case FM_READ_NET_FLOW:
		{

			sprintf((char *)&Tx_Buff[UART_485].tx_buffer[0],"Read 1>\r5\r");
			Tx_Buff[UART_485].tx_indx = strlen((const char *)Tx_Buff[UART_485].tx_buffer);
			Tx_Buff[UART_485].tx_ready = TRUE;
			flow_meter.fm_cbst = flow_meter.fm_state;
			flow_meter.fm_state = FM_RSP;
		}
		break;
		
		case FM_RSP:
		{

			if(Rx_Buff[UART_485].rx_state == READY)
			{

				decodeFMData();
				flow_meter.fm_state = FM_WAIT;
				flushTxBuffer(UART_485);
				flushRxBuffer(UART_485);
			}
			else
			{
				if(fm_timeout++ > FM_RESP_TIMEOUT)
				{
					fm_timeout = 0;
					
					flow_meter.fm_state = FM_WAIT;
					flushTxBuffer(UART_485);
					flushRxBuffer(UART_485);
				}
			}
			
		}
		break;
		case FM_WAIT:
		{
			if(fm_timeout++ > FM_RESP_TIMEOUT)
			{
				fm_timeout = 0;
				switch(flow_meter.fm_cbst)
				{
					case FM_READ_CURR_FLOW:
						flow_meter.fm_state = FM_READ_NET_FLOW;
					break;
					case FM_READ_NET_FLOW:
						flow_meter.fm_state = FM_READ_CURR_FLOW;
					break;
					default:
					break;
				}
			}
		}
		break;
	}
}

void decodeFMData()
{
	char tmpstr[RX_BUFFER_MAX];
	char temp_buff[RX_BUFFER_MAX];
	int num_bytes = 0;
	unsigned int index = 0, i = 0;
	unsigned long temp_net_flow = 0;
	switch(flow_meter.fm_cbst)
	{
		case FM_READ_CURR_FLOW:
		{
			memset((char *)temp_buff,0,RX_BUFFER_MAX);
			
			
			if(check_string_fm("Read Flow 1>\r\n ",tmpstr,&num_bytes) == FM_MATCH_OK)
			{

				while((isdigit(tmpstr[index]) || (tmpstr[index] == '.')) && (index < RX_BUFFER_MAX) )
				{

					if(tmpstr[index] != '.')
						temp_buff[i++] = tmpstr[index++];
					else
						index++;

				}
				flow_meter.fm_instant_data.curr_flow = strtoul(temp_buff,NULL,10);	

				
			}
		}
		break;
		case FM_READ_NET_FLOW:
			if(check_string_fm("Read Net 5>\r\n ",tmpstr,&num_bytes) == FM_MATCH_OK)
			{
				memset((char *)temp_buff,0,RX_BUFFER_MAX);

				while((isdigit(tmpstr[index])) && (index < RX_BUFFER_MAX))
				{
					temp_buff[i++] = tmpstr[index++];
				}
				temp_net_flow = strtoul(temp_buff,NULL,10)*100;
				
				if(temp_net_flow >= flow_meter.fm_instant_data.net_flow)
				{
					flow_meter.fm_instant_data.net_flow = temp_net_flow;
				}

			}
		break;
		default:
		break;
	}
}

void readFMSensorADC()
{
	unsigned int temp_val,i;

	temp_val = readADC(FM_420_CH);
	sensor_data.l_hr = 0;
	sensor_data.avg = 0;
	
	if(temp_val < MAX_ADC_VALUE)
	{
		for(i=0;i<(NUM_OF_SAMPLES-1);i++)
		{
			sensor_data.sample_1s[NUM_OF_SAMPLES-i-1] = sensor_data.sample_1s[NUM_OF_SAMPLES-i-2];
		}
	
		sensor_data.sample_1s[0] = temp_val;
	
		for(i=0;i<(NUM_OF_SAMPLES);i++)
		{
			sensor_data.avg += sensor_data.sample_1s[i];
		}
	
		sensor_data.avg = sensor_data.avg/NUM_OF_SAMPLES;
	
		if(product_info.calib_factor == 0)		//25-06-21 SK: in case if ram data and product info structure is zero then lets read from eeprom and flash again
		{

			eepromReadProductInfo();
			get_present_time(&ram_data.time);
			get_last_rtu_log();
			
		}
		
		if((sensor_data.avg > product_info.ADC_offset) && (product_info.calib_factor != 0))
		{
			sensor_data.l_hr = (((unsigned long)sensor_data.avg-product_info.ADC_offset)*1000*100)/(product_info.calib_factor);
			fm_error_cnt = 0;
			ram_data.status &= ~(1 << FLOW_METER_ERR);
		}
		else if(sensor_data.avg == 0)
		{
			if(fm_error_cnt++>20)
			{
				ram_data.status |= (1 << FLOW_METER_ERR);
				fm_error_cnt = 0;
			}
		}
	}
	else
	{
	}


#ifdef DEBUG_RAM_DATA
	UWriteString("\nadc:",UART_PC);
	UWriteInt(temp_val,UART_PC);
	UWriteData(',',UART_PC);
	UWriteInt(sensor_data.avg,UART_PC);
	UWriteData(',',UART_PC);
	UWriteInt(sensor_data.l_hr,UART_PC);
#endif
}

char get_rx_data_atg(char *copy_here)
{
	char retval = 0;
	
	if(copy_here)
	{
		memcpy (copy_here, (const void *)Rx_Buff[UART_485].rx_buffer, Rx_Buff[UART_485].rx_indx);
		copy_here[Rx_Buff[UART_485].rx_indx] = '\0';
		retval = Rx_Buff[UART_485].rx_indx;
	}
	return retval;
}
char check_string_fm(const char *str, char *copy_here, int *numbytes)
{
	char retval = FM_NO_NEW_MSG;
	unsigned char i, j, length;
	length = strlen(str);
	*numbytes = get_rx_data_atg(copy_here);

	if((str[0]) != '\0')
	{
		if(*numbytes > 0)
		{
			for (i=0; i < *numbytes; i++)
			{
				if (!memcmp(&copy_here[i], str, length)) break;
			}
			if(i >= *numbytes)
			{
				return retval = FM_MATCH_FAIL;
			}
			retval = FM_MATCH_OK;
			for(j = 0; j < (*numbytes - i - length); j++)
			{
				copy_here[j] = copy_here[j + i + length];
			}
			*numbytes = j;
			copy_here[j] = '\0';
		}
	}
	else
	{
		retval = FM_MATCH_OK;
	}
	return (retval);
}

