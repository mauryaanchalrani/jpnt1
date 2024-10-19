
/*
 * PLC_comms.cpp
 *
 * Created: 7/12/18 10:45:05 AM
 *  Author: Mindfield
 */ 

#include <stdint.h>
#include <string.h>

#include "ModbusRtu.h"
#include "RS_485_device.h"
#include "uart.h"
#include "gprs.h"
#include "app_uart.h"
#include "_debug.h"
#include "error.h"
#include "main.h"
#include "common.h"
#include <stdio.h>



RS485_device RS485_device_handler;

extern volatile Rx_Buff_t Rx_Buff[3];
extern volatile Tx_Buff_t Tx_Buff[3];

extern volatile gprs_tx_data_buff_t gprs_tx_buff;
extern volatile gprs_rx_data_buff_t gprs_rx_buff;
extern gprs_rx_isr_handler_t gprs_rx_isr_handler;
extern Modbus master;
extern ram_data_t ram_data;
extern sensor_data_t sensor_data;

void RS485_device::send_data(modbus_t query_data)
{	

	master.query(query_data);
}

int RS485_device::get_data(void)
{
	
	int num_bytes;

	num_bytes = master.poll();
	return num_bytes;	 			
}

void RS485_device::slave_handler(void)
{

	modbus_t query_data;
	
	
	
	static MB_handler_state_t MB_handler_state = MB_QUERY;
	uint16_t num_bytes=0;
	static uint16_t timeout = 0;
	static uint8_t slave_id;
	static uint8_t query_id;
	
	switch(MB_handler_state)
	{
		

		case MB_QUERY:
		

				{					

					RS485_device_handler.get_data_from_cmd_table(&query_data);  // 22-2-19 VC: get all data from device & put on bus
					slave_id = query_data.u8id;
					query_id = query_data.query_id;
					RS485_device_handler.send_data(query_data);// 23/10/2018 vc: for testing purpose need to remove
		

					MB_handler_state = MB_RD_POLL;

				}		
			
				timeout = 0;
			break;
		
		
		
		case MB_RD_POLL: //checking for data from slave
	
					num_bytes = RS485_device_handler.get_data();
					
					
					if((num_bytes > 4) && (num_bytes < 255)) // if data is recevied from a valid slave
					{
					// need to be updata the values of energy meter. 



						copyFromModbusReg(query_id, (char*)&Rx_Buff[UART_485].rx_buffer[0]);
						MB_handler_state = MB_QUERY;
						
						flushRxBuffer(UART_485);
						
						master.setState(COM_IDLE);
						
						RS485_device_handler.update_dev_sts(slave_id, 1);
					}
					else // slave is not response on the given time slot
					{

						if(num_bytes == 0)
						{ 

							if(timeout++ >= MB_CMD_TIMEOUT) //wating upto 100ms for slave response 
							{ // need to be add for slave disconnected indication

								timeout = 0;
								MB_handler_state = MB_QUERY;
								flushRxBuffer(UART_485);
								master.setState(COM_IDLE);
								
								//Lets update device status here
								RS485_device_handler.update_dev_sts(slave_id, 0);
								sensor_data.ml_s = 0;
							}
						}
						else
						{
							//we have got some exception

							
							MB_handler_state = MB_QUERY;
							flushRxBuffer(UART_485);
							master.setState(COM_IDLE);	
							
							RS485_device_handler.update_dev_sts(slave_id, 1);			// 22-2-19 VC: need to think for error in slave
						}
					}
						
		break;
		
		case MB_WR_POLL: // checking for response from the slave
			num_bytes = RS485_device_handler.get_data();

			if((num_bytes > 4) && (num_bytes < 255)) 
			{
					RS485_device_handler.reRouteData(RS485_device_handler.get_req_port(),num_bytes);

								//Lets indicate the RTU
					MB_handler_state = MB_QUERY;
					flushRxBuffer(UART_485);
					master.setState(COM_IDLE);
				
					RS485_device_handler.update_dev_sts(slave_id, 1);
				
			
			}
			else // slave has not responding in the correct time slot
			{
				if(num_bytes == 0)
				{
					if(timeout++ >= (MB_CMD_TIMEOUT)) // waiting upto 200ms for response of slave
					{

						timeout = 0;	
						//Lets indicate the RTU
						MB_handler_state = MB_QUERY;
						flushRxBuffer(UART_485);
						master.setState(COM_IDLE);
						
						RS485_device_handler.update_dev_sts(slave_id, 0);

					}
				}
				else
				{
					{
						num_bytes = MB_ERROR_RESP_PCK_SIZE;			//Need to 
						RS485_device_handler.reRouteData(RS485_device_handler.get_req_port(),num_bytes); 
					}
					MB_handler_state = MB_QUERY;
					flushRxBuffer(UART_485);
					master.setState(COM_IDLE);
					
					RS485_device_handler.update_dev_sts(slave_id, 1);
					timeout = 0;
				}
			}// else end 
		
		break;
		default:
		
		break;
	} //switch end
	
}// function end



uint16_t* RS485_device::get_data_from_cmd_table(modbus_t*  query_data)
{
	static int cmd_table_indx = 0;
	
	uint16_t* ret_addr = NULL;
	
	query_data->query_id = modbus_cmd[cmd_table_indx].query_id;
	query_data->u8id = modbus_cmd[cmd_table_indx].slave_id;
	query_data->u8fct = modbus_cmd[cmd_table_indx].fun_code;
	query_data->u16RegAdd = modbus_cmd[cmd_table_indx].regi_start_addr;
	query_data->u16CoilsNo = modbus_cmd[cmd_table_indx].length;
	cmd_table_indx++;
	if(modbus_cmd[cmd_table_indx].slave_id == 0xFF)
		cmd_table_indx = 0;
	
	
	return ret_addr;
}
void RS485_device::update_dev_sts(uint8_t slave_id, uint8_t status)
{
	device_sts[slave_id-1] = status;
}

void RS485_device::set_req_port(uint8_t port)
{
	req_port = port;
}
uint8_t RS485_device::get_req_port(void)
{
	return req_port;
}



void RS485_device::reRouteData(int uart_no, unsigned char len)		//
{
	int i=0;
	flushTxBuffer(uart_no);
	
	if(uart_no <= UART2)
	{
		
		Tx_Buff[uart_no].tx_indx = 0;
	
		while(i<(len-1))
		{
			Tx_Buff[uart_no].tx_buffer[i] = Rx_Buff[UART_485].rx_buffer[i];
			i++;
			Tx_Buff[uart_no].tx_indx++;
		}
	
		Tx_Buff[uart_no].tx_ready = TRUE;		//Anand 21.05.2014
	}
	else if (uart_no == GPRS_UART)
	{
		gprs_tx_buff.index = 0;
		
		while(i<(len-1))
		{
			gprs_tx_buff.buffer[i] = Rx_Buff[UART_485].rx_buffer[i];
			i++;
			gprs_tx_buff.index++;
		}		
		
		gprs_tx_buff.locked = LOCKED;
		
		
	}
}
float ModbusHex2Float(char *data)
{
	float  *fptr;
	
	fptr = (float *)&data[0];
	return (*fptr);
}
unsigned long ModbusHex2ulong(char *data)
{
	unsigned long temp_long;
	
	temp_long = ((((unsigned long)data[1])<<24)|(((unsigned long)data[0])<<16)|(((unsigned long)data[3])<<8)|((unsigned long)data[2]));
	return temp_long ;
}
void copyFromModbusReg(uint16_t query_id, char *data_buff)

{
	
	float temp_float = 0;

	switch(query_id)
	{
		
		case QUERY_0:
				unsigned long temp_hex;// = data_buff[MB_RSP_DATA_INDX];
				temp_hex = (unsigned long)data_buff[MB_RSP_DATA_INDX + 3];
				temp_hex |= (unsigned long)data_buff[MB_RSP_DATA_INDX + 2]<<8;
				temp_hex |= (unsigned long)data_buff[MB_RSP_DATA_INDX + 1]<<16;
				temp_hex |= (unsigned long)data_buff[MB_RSP_DATA_INDX]<<24;
				temp_float = convert_hex_to_float(temp_hex);

				getInstFlow(temp_float);			
		break;
		default:
		break;
	}
}
