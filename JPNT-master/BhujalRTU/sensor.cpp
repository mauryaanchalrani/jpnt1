/*
 * sensor.cpp
 *
 * Created: 8/22/18 12:14:24 PM
 *  Author: Mindfield
 */ 
#include "sensor.h"
#include "pulser.h"
#include "string.h"
#include "_debug.h"
#include "app_uart.h"
#include "common.h"

//fm_data_t fm_data;
fm_e2p_data_t fm_e2p_data;
calbration_data_t calbration_data;	
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void initRAMData()
{
// 	memset(&fm_e2p_data,0,sizeof(fm_e2p_data));
// 	memset(&fm_data,0,sizeof(fm_data));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void updateRAMData(void)
{
// 	fm_data.inst_data = convert_to_litre(getPulserCnt());
// 	fm_data.cmltv_data = calculateCmltv(fm_data.inst_data, fm_data.cmltv_data);
// #ifdef DEBUG_VTC
// 	displayData(fm_data.inst_data, fm_data.cmltv_data);
// #endif
// 	clrPulserCnt();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

float calculateCmltv(float inst, float cmltv)
{
	//float raw;
	//raw = convert_to_litre(pul);
	cmltv+=0.001*inst;								// Kilo-litres per unit
	return cmltv;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void storeCumulativeData(void)
{
	//calculateCmltv(vtc_data.cmltv_data,vtc_data.inst_data);
	//convertCumulativeData(fm_data.cmltv_data);
	//eepromWriteCumulativeConfig();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void convertCumulativeData(float cmltv)
{
	unsigned long int temp;
	
	////////// Calculation of Lower Significant Bytes //////////
	
	temp=(unsigned long int)cmltv;
	fm_e2p_data.cmltv_lsB=(unsigned int)temp;
	
	////////// Calculation of Decimal Part up to 2 Digits //////////
	
	fm_e2p_data.cmltv_dp=(int)((cmltv-temp)*100);		  // Don't write 10 in place of 100 because then it'll print eg - 12.1 in place of 12.01
	// Note: Even if the cmltv(float) data is larger than temp(unsigned long int), then also there won't be any negative impact,
	//		 as when the vtc_e2p_data.cmltv_dp(char) is printed(by using %d), it will only print the rightmost 2 digits(which are actually
	//		 the decimal digits) as the size of char is only 1 byte(256 characters at max).
	
	////////// Calculation of the Most Significant Byte //////////
	
	temp-=fm_e2p_data.cmltv_lsB;			  				// Not Required
	temp>>=16;
	//vtc_e2p_data.cmltv_msB=(unsigned char)(temp&0x00FF);
	fm_e2p_data.cmltv_msB=(unsigned char)temp;
	// Note: vtc_e2p_data.cmltv_msB(char) will only print the rightmost 2 digits as the size of char is only 1 byte(256 characters at max).
	
#ifdef DEBUG_VTC
		char tempstr[PRINT_STR_LEN];
		UWriteString("\nNow, following data will be written to EEPROM\n", UART_PC);
		if(fm_e2p_data.sno==100)
		sprintf(tempstr,"SNO : %d MSB : %d LSB : %d DP : %d\n",1,fm_e2p_data.cmltv_msB,fm_e2p_data.cmltv_lsB,fm_e2p_data.cmltv_dp);
		else
		sprintf(tempstr,"SNO : %d MSB : %d LSB : %d DP : %d\n",fm_e2p_data.sno+1,fm_e2p_data.cmltv_msB,fm_e2p_data.cmltv_lsB,fm_e2p_data.cmltv_dp);
		UWriteString(tempstr, UART_PC);
#endif	
#ifdef GSM_DEBUG
	UWriteInt((fm_e2p_data.cmltv_msB|fm_e2p_data.cmltv_lsB),UART_PC);
	UWriteString(".",UART_PC);
	UWriteInt(fm_e2p_data.cmltv_dp,UART_PC);
#endif
	
	////////// Calculation of Checksum //////////
	
	// vtc_e2p_data.chksum = getChecksum((unsigned char *)&vtc_e2p_data, sizeof(vtc_e2p_data)-sizeof(vtc_e2p_data.chksum));		 	
}

void getCumulativeData()
{
	unsigned long int temp;
	//eepromReadCumulativeData();
	temp=(long)fm_e2p_data.cmltv_msB;
	// Method 1												// Both the methods are one and the same!!!
	temp<<=16;			 
	//fm_data.cmltv_data += ((float)temp) + ((float)fm_e2p_data.cmltv_lsB) + (0.01 * (float)fm_e2p_data.cmltv_dp);
	// Method 2
	// temp  = (temp<<16) | vtc_e2p_data.cmltv_lsB;
	// vtc_data.cmltv_data += ((float)temp) + (0.01 * (float)vtc_e2p_data.cmltv_dp);
	
#ifdef DEBUG_VTC
/*
	typedef struct{
		int integer;
		int decimal;
	} myFloat;
	
	myFloat c;
	char dspdata[PRINT_STR_LEN];
	c.integer=(int)vtc_data.cmltv_data;
	c.decimal=(int)((vtc_data.cmltv_data-c.integer)*100);
	UWriteString("\nCumulative Data fetched from EEPROM : ", UART_PC);
	sprintf(dspdata,"%d.%d\n",c.integer,c.decimal);
	UWriteString(dspdata, UART_PC);
*/
#endif
}

