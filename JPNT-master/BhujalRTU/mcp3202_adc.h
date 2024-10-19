#ifndef __MCP3202_ADC_H__
#define __MCP3202_ADC_H__

#define MCP3202_CH0		(0x0D)		//0b00001101
#define MCP3202_CH1		(0x0F)		//0b00001111

#define TANK_LVL_CH		(MCP3202_CH0)

void Output_Control(char);
int Input_Data(void);
unsigned int read_mcp3202(unsigned int);

#endif