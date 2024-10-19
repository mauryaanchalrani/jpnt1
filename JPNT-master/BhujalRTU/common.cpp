#include "common.h"
#include "main.h"
#include <string.h>
#include "modbus_addr.h"
#include <stdlib.h>
#include <ctype.h>

IEEE_converter_t IEEE_converter;

void convertToBigEndien(unsigned char * dataptr, int size)
{
	char temp;
	
	for(int i = 0; i < size/2; i++ )
	{	
		temp = dataptr[i];
		dataptr[i] = dataptr[size-i-1];
		dataptr[size-i-1] = temp;
	}
			
}

void convertToMbFormat(unsigned char* dataptr, int size)
{
	char temp;
	
	temp = dataptr[0];
	dataptr[0] =dataptr[2];
	dataptr[2] = temp;
	
	temp = dataptr[1];
	dataptr[1] =dataptr[3];
	dataptr[3] =temp;
	
}

unsigned long my_atoul(char *str, int len, int dp)
{
	unsigned long ret_val = 0;
	for (int i = 0; i < (len); i++, str++)
	{
		if(i != (dp-1))
		{
			ret_val *=  10;
			ret_val += (*str - '0');
		}
	}
	return ret_val;
}
/*
float convertToBigEndien(float littelendian)
{
	float ret_val;

	ret_val = ((0x000000ff) & (littelendian >> 24));
	ret_val |= ((0x0000ff00) & (littelendian >> 8));
	ret_val |= ((0x00ff0000) & (littelendian << 8));
	ret_val |= ((0xff000000) & (littelendian << 24));
	
	return ret_val;
}
unsigned long convertToBigEndien(unsigned long littelendian)
{
	unsigned long ret_val;
	
	ret_val = ((0x000000ff) & (littelendian >> 24));
	ret_val |= ((0x0000ff00) & (littelendian >> 8));
	ret_val |= ((0x00ff0000) & (littelendian << 8));
	ret_val |= ((0xff000000) & (littelendian << 24));
	
	return ret_val;
}*/


/*unsigned*/ char getChecksum(unsigned char* buff, char len)
{
	/*unsigned*/ char chksum=0;
	int i=0;

	for(i=0 ; i<len ; i++)
	{
		chksum+=buff[i];
	}
	
	chksum = (~chksum) + 1;

	return chksum;
}

/* atoi: convert s to integer; version 2 */
int atoi_(char s[], char len_limit)
{
	int i, n;
	char sign;
	unsigned char len = 0;
	
	for (i = 0; s[i]=='_' ; i++); /* skip white space */
	
	sign = (s[i] == '-') ? -1 : 1;
	
	for(i = 0 ; s[i] ; ++i, ++len);	//Len Measure
	
	if (s[i] == '+' || s[i] == '-') /* skip sign */
	i++;
	
	for (n = 0, i=(len-len_limit) ; (i<len); i++)		//Anand 02.05.2014
	{
		if((s[i]>='0') && (s[i]<='9'))
		{
			n = 10 * n + (s[i] - '0');
		}
	}
	
	return sign * n;
}



#if 0
void ftoa_(float val, char result[])
{
	int val_int = val, i= (DISP_DATA_MAX_LEN - DISP_DATA_DECI_DIGITS - 1 - 1);
	
	while(i>=0)
	{
		result[i]=('0'+val_int%10);
		val_int=val_int/10;
		i--;
	}
	result[(DISP_DATA_MAX_LEN - DISP_DATA_DECI_DIGITS - 1)] = '.';
	
	/*val_int = ((val - ((int)val))*100);		//Anand 19.06.2014*/
	val_int = ((val - ((int)val))*1000);		//Anand 19.06.2014
	if(val_int%10 > 5)
	{
		val_int += 10;
	}
	val_int /= 10;
	i=(DISP_DATA_MAX_LEN - 1);
	
	while(i>(DISP_DATA_MAX_LEN - DISP_DATA_DECI_DIGITS - 1))
	{
		result[i]=('0'+val_int%10);
		val_int=val_int/10;
		i--;
	}
	
	result[DISP_DATA_MAX_LEN] = '\0';
}
#endif
void ftoa_1(float val, char result[])
{
	int val_int = val, i=2, j=1;
	
	while(j<=val_int)
	{
		j *= 10;
		i++;
	}
	if(j>10)
	{
		j /= 10;
	}
	
	i=0;
	
	while(j)
	{
		result[i++] = ('0'+(val_int/j));
		val_int %= j;
		j /= 10;
	}
	result[i++] = '.';
	
	val_int = ((val - ((int)val))*1000);		//Anand 19.06.2014
	if(val_int%10 > 5)
	{
		val_int += 10;
	}
	val_int /= 10;
	
	j = 10;
	while(j)
	{
		result[i++]=('0'+(val_int/j));
		val_int %= j;
		j /= 10;
	}
	
	result[i] = '\0';
}

void ftoa_1_7Seg(float val, char result[])
{
	int val_int = val, i=2, j=1;
	
	while(j<=val_int)
	{
		j *= 10;
		i++;
	}
	if(j>10)
	{
		j /= 10;
	}
	
	i=0;
	
	while(j)
	{
		result[i++] = ('0'+(val_int/j));
		val_int %= j;
		j /= 10;		
	}
	result[i++] = '.';
	
	val_int = ((val - ((int)val))*100);		//Anand 19.06.2014
	if(val_int%10 > 5)
	{
		val_int += 10;
	}
	val_int /= 10;
	
	val_int %= 10;
	result[i++] = ('0'+(val_int));		//
		
	result[i] = '\0';
}

void removeUnderScores(char *str)		//Anand 11.11.2014
{
	unsigned int i=0;
	
	while(str[i] && (str[i]=='_'))
		str[i++] = '0';
}

char getHighBitIndex(char byte)
{
	char indx = 0xFF;
	
	switch(byte)
	{
		case 0x01 : indx = 0; break;
		case 0x02 : indx = 1; break;
		case 0x04 : indx = 2; break;
		case 0x08 : indx = 3; break;
		case 0x10 : indx = 4; break;
		case 0x20 : indx = 5; break;
		case 0x40 : indx = 6; break;
		case 0x80 : indx = 7; break;
		default : indx = 0xFF;
	}
	
	return indx;
}

char strstr_(char s1[], char s2[])		//Anand 19.06.2014
{
	int i=0, j=0;
	char same = 0;

	for( ; s1[i] ; i++)
		if(isalnum_(s1[i]))
			{
				if((s1[i] == s2[j]))
				{
					++i, ++j;
					same = 1;
					for( ; s2[j]&& s1[i] ; j++, i++)
					if(s1[i] != s2[j])
					{
						same = 0;
						break;
					}

					if(same)
					{
						if(s1[i] || s2[j])
						{
							same = 0;
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
	return same;
}

char isalnum_(char ch)		//Anand 19.06.2014
{
	char rslt = 0;

	if(((ch>='0') && (ch<='9')) || ((ch>='A') && (ch<='Z')) || ((ch>='a') && (ch<='z')))
		rslt = 1;

	return rslt;
}

void ftoa_gps(float val, char result[])
{
	long int val_int = val, i=2, j=1;
	
	while(j<=val_int)
	{
		j *= 10;
		i++;
	}
	if(j>10)
	{
		j /= 10;
	}
	
	i=0;
	
	while(j)
	{
		result[i++] = ('0'+(val_int/j));
		val_int %= j;
		j /= 10;
	}
	result[i++] = '.';
	
	/*val_int = ((val - ((int)val))*100);		//Anand 19.06.2014*/
	val_int = ((val - ((int)val))*1000000UL);		//Anand 19.06.2014
	/*if(val_int%10 > 5)
	{
		val_int += 10;
	}
	val_int /= 10;*/
	
	j = 100000;
	while(j)
	{
		result[i++]=('0'+(val_int/j));
		val_int %= j;
		j /= 10;
	}
	
	result[i] = '\0';
}

unsigned char isStringDigit(char *str)			///16-02-2019 SK
{
	unsigned char isDigit = 1;
	int i=0;
	while(i < (IMEI_NUM_LEN-1))
	{
		if(!isdigit(str[i]))
		{
			isDigit = 0;
			break;
		}
		i++;
	}
	return isDigit;
}

unsigned char hex2int(char *str)
{
	unsigned char retval = 0,i;
	unsigned char temp = 0;
	for(i = 0; i < 2; i++)
	{
		if(*str >= '0' && *str <= '9')
		{
			temp = *str - '0';
		}
		else
		{
			switch(*str)
			{
				case 'A':
				case 'a': temp = 10;
				break;
				case 'B':
				case 'b': temp = 11;
				break;
				case 'C':
				case 'c': temp = 12;
				break;
				case 'D':
				case 'd': temp = 13;
				break;
				case 'E':
				case 'e': temp = 14;
				break;
				case 'F':
				case 'f': temp = 15;
				break;
				
				default: temp = 0;
			}
		}
		if(i == 0)
		{
			retval = temp*16;
		}
		else
		{
			retval += temp;
		}
		str++;
	}
	return retval;
}

float convert_hex_to_float(unsigned long val)
{
	IEEE_converter.long_value = val;
	return IEEE_converter.float_value;
}

unsigned long convert_float_to_hex(float val)
{
	IEEE_converter.float_value = val;
	return IEEE_converter.long_value;
}
/**
 * @brief
 * This method converts two unit8 data to word
 
 * @param uint8_t hibyte  higher byte
 * @param uint8_t  lower byte
 * @return word
 */
