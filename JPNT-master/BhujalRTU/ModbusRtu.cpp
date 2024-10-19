/*
 * ModbusRtu.cpp
 *
 * Created: 7/10/18 4:21:56 PM
 *  Author: Mindfield
 */ 


#include "ModbusRtu.h"
#include "_debug.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "common.h"
#include "uart.h"
#include "app_uart.h"
#include "modbus_addr.h"
#include "pins.h"
#include "_debug.h"

extern volatile Rx_Buff_t Rx_Buff[3];
extern volatile Tx_Buff_t Tx_Buff[3];
Modbus master(0,1,MOD_TXEN_PIN);
/* _____COMMON FUNCTIONS_____________________________________________________ */ // :VC:11/7/2018: common fuctions defination

unsigned int word( unsigned char hibyte, unsigned char lobyte)
{
	unsigned int val;
	val = (hibyte<<8) | lobyte;
	return val;
}

unsigned char highByte(unsigned int val)
{
	return (unsigned char)(val>>8);
}

unsigned char lowByte(unsigned int val)
{
	return (unsigned char)(val);
}

void pinMode(uint8_t pin, uint8_t mode,int* port)
{	
	if(mode)
		*port |=(1<<pin); // :VC:11/07/2018 select a mode for direction pin
	else
		*port &=~(1<<pin);
}


void digitalWrite(uint8_t pin, uint8_t val, int* port)
{
	if(val) // :VC:11/07/2018 write a value on given pin
		*port |=(1<<pin);
	else
		*port &=~(1<<pin);	
}
void pinModeRtu(uint8_t pin, uint8_t mode)
{
	if(mode)
	GENERAL_IO_DDR |=(1<<pin); // :VC:11/07/2018 select a mode for direction pin
	else
	GENERAL_IO_DDR &=~(1<<pin);
}

uint16_t digitalRead(uint16_t cmd)
{
	if(GENERAL_IO_PIN & (1 << cmd))
	{
		return HIGH;
	}
	else
		return LOW;	
}
bool bitRead(uint8_t src,uint8_t bit_num)
{
	if(src &(1<<bit_num)) 
		return 1;
	else
		return 0;
	
}

void bitWrite(uint8_t dst, uint8_t bit_num,bool val)
{
	if(val)
		dst=dst|(1<<bit_num);
	else
		dst=dst&(~(1<bit_num));		
}
uint16_t digitalReadRtu(uint16_t addr)
{
	uint16_t ret_val = 0;
	for(int i=0; i<NUM_OF_MB_IO; i++)
	{
	
	}
	return ret_val;
}
void digitalWriteRtu(uint16_t addr, uint8_t val)
{

	for(int i=0; i<NUM_OF_MB_IO; i++)
	{

	}
}

/* _____PUBLIC FUNCTIONS_____________________________________________________ */

Modbus::Modbus()
{
    init(0, 0, 0);
}


Modbus::Modbus(uint8_t u8id, uint8_t u8serno)
{
    init(u8id, u8serno, 0);
}


Modbus::Modbus(uint8_t u8id, uint8_t u8serno, uint8_t u8txenpin)
{
	 init(u8id, u8serno, u8txenpin);
}


Modbus::Modbus(uint8_t u8id)
{
    init(u8id);
}


void Modbus::begin(long u32speed)
{
	initUart(UART1);
    
    {
        pinMode(u8txenpin, OUTPUT,(int*) &MODBUS_DDR);
        digitalWrite(u8txenpin, LOW,(int*) &MODBUS_PORT);
    }

    u8lastRec = u8BufferSize = 0;
    u16InCnt = u16OutCnt = u16errCnt = 0;
}


void Modbus::begin()
{
    begin(19200);
}


void Modbus::setID( uint8_t u8id)
{
    if (( u8id != 0) && (u8id <= 247))
    {
        this->u8id = u8id;
    }
}


uint8_t Modbus::getID()
{
    return this->u8id;
}


uint16_t Modbus::getInCnt()
{
    return u16InCnt;
}


uint16_t Modbus::getOutCnt()
{
    return u16OutCnt;
}


uint16_t Modbus::getErrCnt()
{
    return u16errCnt;
}


uint8_t Modbus::getState()
{
    return u8state;
}



void Modbus::setState(uint8_t state)
{
   u8state = state;
}


void Modbus::setbuffsize(uint8_t size)
{
   u8BufferSize = size;
}


uint8_t Modbus::getLastError()
{
    return u8lastError;
}


int8_t Modbus::query( modbus_t telegram )
{
    Tx_Buff[UART_485].tx_buffer[ ID ]         = telegram.u8id;
    Tx_Buff[UART_485].tx_buffer[ FUNC ]       = telegram.u8fct;
    Tx_Buff[UART_485].tx_buffer[ ADD_HI ]     = highByte(telegram.u16RegAdd );
    Tx_Buff[UART_485].tx_buffer[ ADD_LO ]     = lowByte( telegram.u16RegAdd );
	if (u8state != COM_IDLE) return -1;

	if ((telegram.u8id < MODBUS_MIN_SLV_ID) || (telegram.u8id > MODBUS_MAX_SLV_ID)) return -3;

	Tx_Buff[UART_485].tx_buffer[ NB_HI ]      = highByte(telegram.u16CoilsNo );
	Tx_Buff[UART_485].tx_buffer[ NB_LO ]      = lowByte( telegram.u16CoilsNo );
	u8BufferSize = 6;
	
    sendTxBuffer();				// 22-2-19 VC: this is actually send data to modbus
	u8BufferSize = 0;			//need to make it zero
    u8state = COM_WAITING;
	
    flushRxBuffer(UART_485); // need to be flush rxbuffer after receving the data
	
	return 0;
} //query end


int16_t Modbus::poll()
{
	int16_t ret_val = 0;
	
	if(Rx_Buff[UART_485].rx_state == READY)
	{

		int16_t u8exception = validateAnswer();
		if (u8exception != 0)
		{
			u8state = COM_IDLE;
			ret_val = u8exception;
		}
		else
		{
			ret_val = Rx_Buff[UART_485].rx_indx;
		}
		
	}  // end checking for Rx_Buff[UART_485].rx_state 

	return ret_val;
} // master poll end 


int16_t Modbus::poll( uint16_t *regs, uint8_t u8size )
{
	int16_t ret_val = 0;
	
	if(Rx_Buff[UART_485].rx_state == READY)
	{
		uint8_t u8exception = validateAnswer();
		if (u8exception != 0)
		{
			Rx_Buff[UART_485].rx_state = START;
			flushRxBuffer(UART_485);
		 
			u8state = COM_IDLE;
			ret_val =  u8exception;
		}
		else
		{				
			u8state = COM_IDLE;
			ret_val = u8BufferSize;				//:VC:11/07/2018 TT: ???
		}
	}

	 return ret_val;
}


/* _____PRIVATE FUNCTIONS_____________________________________________________ */

void Modbus::init(uint8_t u8id, uint8_t u8serno, uint8_t u8txenpin)
{
    this->u8id = u8id;
    this->u8serno = (u8serno > 3) ? 0 : u8serno;
    this->u8txenpin = u8txenpin;
    this->u16timeOut = 1000;
}

void Modbus::init(uint8_t u8id)
{
    this->u8id = u8id;
    this->u8serno = 4;
    this->u8txenpin = 0;
    this->u16timeOut = 1000;
}



void Modbus::sendTxBuffer()
{
	uint16_t u16crc;
	u16crc = calcCRC_RS485( (uint8_t *)Tx_Buff[UART_485].tx_buffer, u8BufferSize);
	
    Tx_Buff[UART_485].tx_buffer[ u8BufferSize ] = u16crc >> 8;
    u8BufferSize++;
    Tx_Buff[UART_485].tx_buffer[ u8BufferSize ] = u16crc & 0x00ff;
    u8BufferSize++;
	digitalWrite(u8txenpin,HIGH,(int*) &MODBUS_PORT);
    UWriteBytes((char *)Tx_Buff[UART_485].tx_buffer,u8BufferSize,UART_485); 
	_delay_us(2000);
	digitalWrite(u8txenpin,LOW,(int*) &MODBUS_PORT);
	u16OutCnt++;
}
uint16_t Modbus::calcCRC(uint8_t u8length)
{
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < u8length; i++)
    {
        temp = temp ^ au8Buffer[i];
        for (unsigned char j = 1; j <= 8; j++)
        {
            flag = temp & 0x0001;
            temp >>=1;
            if (flag)
                temp ^= 0xA001;
        }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    return temp;
}

unsigned int Modbus::calcCRC_RS485(uint8_t *au8Buffer, uint8_t u8length)
{
	unsigned int temp, temp2, flag;
	temp = 0xFFFF;
	for (unsigned char i = 0; i < u8length; i++)
	{
		temp = temp ^ au8Buffer[i];
		for (unsigned char j = 1; j <= 8; j++)
		{
			flag = temp & 0x0001;
			temp >>=1;
			if (flag)
			temp ^= 0xA001;
		}
	}
	// Reverse byte order.
	temp2 = temp >> 8;
	temp = (temp << 8) | temp2;
	temp &= 0xFFFF;

	return temp;
}

int16_t Modbus::validateRequest()
{
    uint16_t u16MsgCRC =
        ((au8Buffer[u8BufferSize - 2] << 8)
         | au8Buffer[u8BufferSize - 1]); // combine the crc Low & High bytes
    if ( calcCRC( u8BufferSize-2 ) != u16MsgCRC )
    {
        u16errCnt ++;
        return NO_REPLY;
    }

    // check fct code
    bool isSupported = false;
    for (uint8_t i = 0; i< sizeof( fctsupported ); i++)
    {
        if (fctsupported[i] == au8Buffer[FUNC])
        {
            isSupported = 1;
            break;
        }
    }
    if (!isSupported)
    {
        u16errCnt ++;
        return EXC_FUNC_CODE;
    }

    // check start address & nb range
    uint16_t u16regs = 0;
    uint8_t u8regs;
    switch ( au8Buffer[ FUNC ] )
    {
    case MB_FC_READ_COILS:
    case MB_FC_READ_DISCRETE_INPUTS:
    case MB_FC_WRITE_MULTIPLE_COILS:
        u16regs = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ]) / 16;
        u16regs += word( au8Buffer[ NB_HI ], au8Buffer[ NB_LO ]) /16;
        u8regs = (uint8_t) u16regs;
        if (u8regs > u8regsize) return EXC_ADDR_RANGE;
        break;
    case MB_FC_WRITE_COIL:
        u16regs = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ]) / 16;
        u8regs = (uint8_t) u16regs;
        if (u8regs > u8regsize) return EXC_ADDR_RANGE;
        break;
    case MB_FC_WRITE_REGISTER :
        u16regs = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ]);
        u8regs = (uint8_t) u16regs;
        if (u8regs > u8regsize) return EXC_ADDR_RANGE;
        break;
    case MB_FC_READ_REGISTERS :
    case MB_FC_READ_INPUT_REGISTER :
    case MB_FC_WRITE_MULTIPLE_REGISTERS :
        u16regs = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ]);
        u16regs += word( au8Buffer[ NB_HI ], au8Buffer[ NB_LO ]);
        u8regs = (uint8_t) u16regs;
        if (u8regs > u8regsize) return EXC_ADDR_RANGE;
        break;
    }
    return 0; // OK, no exception code thrown
}

int16_t Modbus::validateAnswer()
{
	
	uint16_t u16MsgCRC; 
	
	u16MsgCRC = ((Rx_Buff[UART_485].rx_buffer[Rx_Buff[UART_485].rx_indx- 2]) << 8);
	 u16MsgCRC |= Rx_Buff[UART_485].rx_buffer[Rx_Buff[UART_485].rx_indx - 1]; // combine the crc Low & High bytes

	if(calcCRC_RS485((uint8_t*)Rx_Buff[UART_485].rx_buffer, Rx_Buff[UART_485].rx_indx - 2) != u16MsgCRC)
	
    {
        u16errCnt ++;
        return NO_REPLY;
    }

    // check exception
    if ((Rx_Buff[UART_485].rx_buffer[ FUNC ] & 0x80) != 0)
    {
        u16errCnt ++;
        return ERR_EXCEPTION;
    }

    // check fct code
    bool isSupported = false;
    for (uint8_t i = 0; i< sizeof( fctsupported ); i++)
    {
        if (fctsupported[i] == Rx_Buff[UART_485].rx_buffer[ FUNC ])
        {
            isSupported = 1;
            break;
        }
    }
    if (!isSupported)
    {
        u16errCnt ++;
        return EXC_FUNC_CODE;
    }

    return 0; // OK, no exception code thrown
}


void Modbus::buildException( uint8_t u8exception )
{
    uint8_t u8func = au8Buffer[ FUNC ];  // get the original FUNC code

    au8Buffer[ ID ]      = u8id;
    au8Buffer[ FUNC ]    = u8func + 0x80;
    au8Buffer[ 2 ]       = u8exception;
    u8BufferSize         = EXCEPTION_SIZE;
}


void Modbus::get_FC1()
{
  
}

void Modbus::get_FC3()
{
    uint8_t u8byte, i;
    u8byte = 3;

    for (i=0; i< au8Buffer[ 2 ] /2; i++)
    {
        au16regs[ i ] = word(
                            au8Buffer[ u8byte ],
                            au8Buffer[ u8byte +1 ]);
        u8byte += 2;
    }
}

int8_t Modbus::process_FC1( uint16_t *regs, uint8_t u8size )
{
    uint8_t u8currentRegister, u8currentBit, u8bytesno, u8bitsno;
    uint8_t u8CopyBufferSize;
    uint16_t u16currentCoil, u16coil;

    // get the first and last coil from the message
    uint16_t u16StartCoil = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ] );
    uint16_t u16Coilno = word( au8Buffer[ NB_HI ], au8Buffer[ NB_LO ] );

    // put the number of bytes in the outcoming message
    u8bytesno = (uint8_t) (u16Coilno / 8);
    if (u16Coilno % 8 != 0) u8bytesno ++;
    au8Buffer[ ADD_HI ]  = u8bytesno;
    u8BufferSize         = ADD_LO;

    // read each coil from the register map and put its value inside the outcoming message
    u8bitsno = 0;

    for (u16currentCoil = 0; u16currentCoil < u16Coilno; u16currentCoil++)
    {
        u16coil = u16StartCoil + u16currentCoil;
        u8currentRegister = (uint8_t) (u16coil / 16);
        u8currentBit = (uint8_t) (u16coil % 16);

        bitWrite(
            au8Buffer[ u8BufferSize ],
            u8bitsno,
            bitRead( regs[ u8currentRegister ], u8currentBit ) );
        u8bitsno ++;

        if (u8bitsno > 7)
        {
            u8bitsno = 0;
            u8BufferSize++;
        }
    }

    // send outcoming message
    if (u16Coilno % 8 != 0) u8BufferSize ++;
    u8CopyBufferSize = u8BufferSize +2;
    sendTxBuffer();
    return u8CopyBufferSize;
}

int8_t Modbus::process_FC3( uint16_t *regs, uint8_t u8size )
{

    uint8_t u8StartAdd = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ] );
    uint8_t u8regsno = word( au8Buffer[ NB_HI ], au8Buffer[ NB_LO ] );
    uint8_t u8CopyBufferSize;
    uint8_t i;

    au8Buffer[ 2 ]       = u8regsno * 2;
    u8BufferSize         = 3;

    for (i = u8StartAdd; i < u8StartAdd + u8regsno; i++)
    {
        au8Buffer[ u8BufferSize ] = highByte(regs[i]);
        u8BufferSize++;
        au8Buffer[ u8BufferSize ] = lowByte(regs[i]);
        u8BufferSize++;
    }
    u8CopyBufferSize = u8BufferSize +2;
    sendTxBuffer();

    return u8CopyBufferSize;
}

int8_t Modbus::process_FC5( uint16_t *regs, uint8_t u8size )
{
    uint8_t u8currentRegister, u8currentBit;
    uint8_t u8CopyBufferSize;
    uint16_t u16coil = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ] );

    // point to the register and its bit
    u8currentRegister = (uint8_t) (u16coil / 16);
    u8currentBit = (uint8_t) (u16coil % 16);

    // write to coil
    bitWrite(
        regs[ u8currentRegister ],
        u8currentBit,
        au8Buffer[ NB_HI ] == 0xff );


    // send answer to master
    u8BufferSize = 6;
    u8CopyBufferSize = u8BufferSize +2;
    sendTxBuffer();

    return u8CopyBufferSize;
}

int8_t Modbus::process_FC6( uint16_t *regs, uint8_t u8size )
{

    uint8_t u8add = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ] );
    uint8_t u8CopyBufferSize;
    uint16_t u16val = word( au8Buffer[ NB_HI ], au8Buffer[ NB_LO ] );

    regs[ u8add ] = u16val;

    // keep the same header
    u8BufferSize         = RESPONSE_SIZE;

    u8CopyBufferSize = u8BufferSize +2;
    sendTxBuffer();

    return u8CopyBufferSize;
}


int8_t Modbus::process_FC15( uint16_t *regs, uint8_t u8size )
{
    uint8_t u8currentRegister, u8currentBit, u8frameByte, u8bitsno;
    uint8_t u8CopyBufferSize;
    uint16_t u16currentCoil, u16coil;
    bool bTemp;

    // get the first and last coil from the message
    uint16_t u16StartCoil = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ] );
    uint16_t u16Coilno = word( au8Buffer[ NB_HI ], au8Buffer[ NB_LO ] );


    // read each coil from the register map and put its value inside the outcoming message
    u8bitsno = 0;
    u8frameByte = 7;
    for (u16currentCoil = 0; u16currentCoil < u16Coilno; u16currentCoil++)
    {

        u16coil = u16StartCoil + u16currentCoil;
        u8currentRegister = (uint8_t) (u16coil / 16);
        u8currentBit = (uint8_t) (u16coil % 16);

        bTemp = bitRead(
                    au8Buffer[ u8frameByte ],
                    u8bitsno );

        bitWrite(
            regs[ u8currentRegister ],
            u8currentBit,
            bTemp );

        u8bitsno ++;

        if (u8bitsno > 7)
        {
            u8bitsno = 0;
            u8frameByte++;
        }
    }

    // send outcoming message
    u8BufferSize         = 6;
    u8CopyBufferSize = u8BufferSize +2;
    sendTxBuffer();
    return u8CopyBufferSize;
}


int8_t Modbus::process_FC16( uint16_t *regs, uint8_t u8size )
{
    uint8_t u8StartAdd = au8Buffer[ ADD_HI ] << 8 | au8Buffer[ ADD_LO ];
    uint8_t u8regsno = au8Buffer[ NB_HI ] << 8 | au8Buffer[ NB_LO ];
    uint8_t u8CopyBufferSize;
    uint8_t i;
    uint16_t temp;

    // build header
    au8Buffer[ NB_HI ]   = 0;
    au8Buffer[ NB_LO ]   = u8regsno;
    u8BufferSize         = RESPONSE_SIZE;

    // write registers
    for (i = 0; i < u8regsno; i++)
    {
        temp = word(
                   au8Buffer[ (BYTE_CNT + 1) + i * 2 ],
                   au8Buffer[ (BYTE_CNT + 2) + i * 2 ]);

        regs[ u8StartAdd + i ] = temp;
    }
    u8CopyBufferSize = u8BufferSize +2;
    sendTxBuffer();

    return u8CopyBufferSize;
}



