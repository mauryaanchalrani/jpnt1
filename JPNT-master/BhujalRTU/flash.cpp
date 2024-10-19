
/*
	Filename:	SST25VF064C
 	Author:		Hardik Patel
	Date:		July 25, 2011
 	Notes:		Verified on  Sept 6, 2011 using explorer 16board and Serial Flash Board.  Serial Flash Board supports both SPI and SQI serial flash.


Software Driver

SST25VF064C 64 Mbit Serial Flash Memory

July 25, 2011, Rev. 1.0

ABOUT THE SOFTWARE
This application note provides software driver examples for SST25VF064C,
Serial Flash. Extensive comments are included in each routine to describe 
the function of each routine.  The interface coding uses polling method 
rather than the SPI protocol to interface with these serial devices.  The
functions are differentiated below in terms of the communication protocols
(uses Mode 0) and specific device operation instructions. 


ABOUT THE SST25VF064C

Companion product datasheets for the SST25VF064C should be reviewed in 
conjunction with this application note for a complete understanding 
of the device.			
*/
#include "main.h"
#include "flash.h"
#include "pins.h"
#include "common.h"
#include "error.h"		//Anand 4.30.2014
#include <string.h>
#include "app_uart.h"
#ifdef _20x4_LCD
#include "lcd.h"
#endif
#include <util/delay.h>
#include <avr/interrupt.h>

#include "events.h"
#include "_debug.h"
#include "app_eeprom.h"
#include <avr/wdt.h>


static FL_data_t FL_data = {0, FL_RTU_START_ADDR, FL_RTU_START_ADDR, 0, 0, /*FL_EVT_START_ADDR, FL_EVT_START_ADDR, 0, 0, 0, 0,*/ 0, 0};		//TT 14-09-17	
extern freq_updated_data_t freq_updated_data;

#ifdef DEBUG_BOOT_HEX
unsigned long hex_file_address = 0;
#endif

//extern FL_evt_log_t FL_evt_log;

/************************************************************************/
/* PROCEDURE: Port Assign												*/
/*																		*/
/* This procedure sets the Ports for SPI communicaiton and LED Display.	*/
/* LATA is for LED display.  LATF is for SPI communication.			*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		None															*/
/*																		*/
/************************************************************************/					


void Portassign()
{
	/*LATA = 0x0000;		// set LAT A output latches low
	TRISA = 0x0000;		// set LAT A pins to be outputs
	
	LATF = 0x0000;		
	TRISF =0x0080;  	//Set RF7 to be input for MISO. Everything else is output.*/
	SPIDDR|=(1<<SI)/*|(1<<RST)*/|(1<<SCK);//|(1<<WP2)|(1<<CE2);
	SPIDDR&=~(1<<SO);
	SPIPORT&=~0XFF;
	FLASH_CNTRL_DDR |= (1<<CE1)/*|(1<<WP1)*/;
	FLASH_CNTRL_PORT &= ~((1<<CE1)/*|(1<<WP1)*/);
	
	FLASH_WP_DDR |= (1 << WP1);
	FLASH_WP_PORT &= ~(1 << WP1);
	
	FLASH_RST_DDR |= (1 << RST);
	FLASH_RST_PORT &= ~(1 << RST);
}	

	

/************************************************************************/
/* PROCEDURE: Display LED												*/
/*																		*/
/* This procedure is used to diplay a byte on 8 LEDs which are 			*/
/* connected to LATA													*/
/* Input:																*/
/*		Led Display														*/
/*																		*/
/* Output:																*/
/*		Led Display														*/
/*																		*/
/************************************************************************/
void DisplayLED(char Led_Stat)
{
	//UWriteData(Led_Stat);
}
						

void SetSI_Output()
{
	SPIDDR|=(1<<SI);
	//_delay_ms(1);
	//_delay_us(1);			// HJ 13-08-2016
}
void SetSI_Input()
{
	SPIDDR&=~(1<<SI);
	//_delay_ms(1);
	//_delay_us(1);			// HJ 13-08-2016
}
void SetSO_Output()
{
	SPIDDR|=(1<<SO);
	//_delay_ms(1);
	//_delay_us(1);			// HJ 13-08-2016
}
void SetSO_Input()
{
	SPIDDR&=~(1<<SO);
//	_delay_ms(1);
	//_delay_us(1);			// HJ 13-08-2016
}					

/************************************************************************/
/* PROCEDURE: CE_High							*/
/*									*/
/* This procedure set CE = High.					*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		CE							*/
/*									*/
/************************************************************************/
void CE_High() 
{
	FLASH_CNTRL_PORT|=(1<<CE1);
	//LATF = LATF | (0x04);					/* set CE high */
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: CE_Low							*/
/*									*/
/* This procedure drives the CE of the device to low.  			*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		CE							*/
/*									*/
/************************************************************************/
void CE_Low() 
{	
	FLASH_CNTRL_PORT&=~(1<<CE1);
	//_delay_ms(1);						/* clear CE low */
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: Reset_Hold_Low()						*/
/*									*/
/* This procedure clears the Reset_Hold pin to low.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		Hold							*/
/************************************************************************/
void Reset_Hold_Low()
{
	//SPIPORT&=~(1<<RST);
	FLASH_RST_PORT&=~(1<<RST);
	//_delay_ms(1);						/* clear Hold pin */
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: Reset_Hold_High()						*/
/*									*/
/* This procedure sets the Reset_Hold pin to high.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		Hold							*/
/************************************************************************/
void Reset_Hold_High()
{
	//SPIPORT|=(1<<RST);		/* set Hold pin */
	FLASH_RST_PORT|=(1<<RST);
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: WP_Low()							*/
/*									*/
/* This procedure clears the WP pin to low.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		WP							*/
/************************************************************************/
void WP_Low()
{
	//FLASH_CNTRL_PORT&=~(1<<WP1);		/* clear WP pin */
	FLASH_WP_PORT&=~(1<<WP1);
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: WP_High()							*/
/*									*/
/* This procedure sets the WP pin to high.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		WP							*/
/************************************************************************/
void WP_High()
{
	//FLASH_CNTRL_PORT|=(1<<WP1); 	/* set WP pin */
	FLASH_WP_PORT|=(1<<WP1);
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: SCK_High													*/
/*																		*/
/* This procedure set SCK = High.										*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		SCK																*/
/*																		*/
/************************************************************************/
void SCK_High() 
{
	SPIPORT|=(1<<SCK);			/* set SCK high */
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: SCK_Low													*/
/*																		*/
/* This procedure drives the SCK of the device to low.  					*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		SCK																*/
/*																		*/
/************************************************************************/
void SCK_Low() 
{	
	SPIPORT&=~(1<<SCK);		/* clear SCK low */
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: SI_High													*/
/*																		*/
/* This procedure set SI = High.										*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		SI																*/
/*																		*/
/************************************************************************/
void SI_High() 
{
	SPIPORT|=(1<<SI);		/* set SI high */
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: SI_Low													*/
/*																		*/
/* This procedure drives the SI of the device to low. 		 			*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		SI																*/
/*																		*/
/************************************************************************/
void SI_Low() 
{	
	SPIPORT&=~(1<<SI);		/* clear SI low */
	//_delay_us(1);			// HJ 13-08-2016
}


/************************************************************************/
/* PROCEDURE: SO_High													*/
/*																		*/
/* This procedure set SO = High.										*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		SO																*/
/*																		*/
/************************************************************************/
void SO_High() 
{
	SPIPORT|=(1<<SO);			/* set SO high */
	//_delay_us(1);			// HJ 13-08-2016
}

/************************************************************************/
/* PROCEDURE: SO_Low													*/
/*																		*/
/* This procedure drives the SO of the device to low. 		 			*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		SO																*/
/*																		*/
/************************************************************************/
void SO_Low() 
{	
	SPIPORT&=~(1<<SO);		/* clear SO low */
	//_delay_us(1);			// HJ 13-08-2016
}
		

/************************************************************************/
/* PROCEDURE: init							*/
/*									*/
/* This procedure initializes the SCK to low. Must be called prior to 	*/
/* setting up mode 0.							*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		SCK							*/
/************************************************************************/
void flashInit()
{
#ifdef _DEBUG_FLASH_UART
	//char debug_data[32] = {"HELLO FLASH"};
#endif
	 Portassign();		 //ADD BY ROHIT
	 CE_High();			 //ADD BY ROHIT
	 //WP_High();			//ADD BY ROHIT		// HJ 27-7-2016   Move to WREN() and WRDI() 
	 WP_Low();				// Lets ensure that on start up flash is write protected
	 Reset_Hold_High(); //ADD BY ROHIT
	 SCK_Low();	/* set clock to low initial state */
	 _delay_ms(10);
										
	 //Lets give a reset on power up	//Kamlesh 24-01-2015
#ifdef SST26VF064B
	 flashSwReset();
	 //   Enable write protection pin functionality
	enable_wp_pin();
#endif
	 //Edit by Anand
	 
#ifdef SST25VF064C	
	WREN();
	EWSR();
	WRSR(0x7F);		//Reset BPL Bit
	WREN();
	EWSR();
	WRSR(0);			//Reset All BP0-BP3
#else
	//remove_block_protection();
#endif

	 
	 #if _ADD_DUMMY_DISPENSE_LOGS || _ADD_DUMMY_REFUEL_LOGS
//	 WREN();
//	 Chip_Erase();
//	 Wait_Busy();
//	WRDI();				// HJ 29-12-2015    // Write Disable 
	 #endif
#ifdef _DEBUG_FLASH_UART
	/*#ifdef FLASH_WP_ENABLE
	remove_block_protection();
	#endif
	WREN();
	Sector_Erase(0x00/ *FL_RR_START_ADDR* /);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable
	#ifdef FLASH_WP_ENABLE
	WBPR(0);
	#endif
	
	#ifdef FLASH_WP_ENABLE
	remove_block_protection();
	#endif
	WREN();
	flashPacketProgram((char*)&debug_data, sizeof(debug_data), 0);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable
	#ifdef FLASH_WP_ENABLE
	WBPR(0);
	#endif
	
	memset(&debug_data,0,sizeof(debug_data));
	
	WREN();
	readContToBuff(0, FL_RTU_LOG_LEN, (char *)&debug_data);
	Wait_Busy();
	WRDI();	
	
	UWriteString("flash_data:",UART_PC);
	UWriteBytes(debug_data,sizeof(debug_data),UART_PC);*/
#endif
	 updateFlashCurrAddr();
}
// HJ 27-7-2016
void remove_block_protection(void)
{
	cli();
	WP_High();
	WREN();
	CE_Low();
	//WP_High();
	Send_Byte(0x98);	//Remove Write Protection from entire flash
	//WP_Low();
	CE_High();
	WRDI();
	WP_Low();
	sei();
}
// HJ 29-7-2016
void enable_wp_pin(void)
{
	WP_High();			//??
	WREN();
	CE_Low();
	WRCR(0x88);
	CE_High();
	WRDI();
	WP_Low();
}
/************************************************************************/
/* PROCEDURE: Send_Byte							*/
/*									*/
/* This procedure outputs a byte shifting out 1-bit per clock rising	*/
/* edge on the the SI pin(MSB 1st).					*/
/*									*/
/* Input:								*/
/*		out							*/
/*									*/
/* Output:								*/
/*		SI							*/
/************************************************************************/

 void Send_Byte(unsigned char out)
{

		SetSI_Output();
		_delay_us(10);		//??
		
		if ((out & 0x80) ==0x80)
			{SI_High();}
		else
			{SI_Low();}

		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */
  		
		if ((out & 0x40) ==0x40)
			{SI_High();}
		else
			{SI_Low();}
				
		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */
  

		if ((out & 0x20) ==0x20)
			{SI_High();}
		else
			{SI_Low();}

		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */
  
		
		if ((out & 0x10) ==0x10)
			{SI_High();}
		else
			{SI_Low();}
				
		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */

		if ((out & 0x08) ==0x08)
			{SI_High();}
		else
			{SI_Low();}
		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */
  
		if ((out & 0x04) ==0x04)
			{SI_High();}
		else
			{SI_Low();}
				
		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */

		
		if ((out & 0x02) ==0x02)
			{SI_High();}
		else
			{SI_Low();}
		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */
  
		
		if ((out & 0x01) ==0x01)
			{SI_High();}
		else
			{SI_Low();}
				
		SCK_High();			/* toggle clock high */
		
		//SetSI_Input();  //Set them as Inputs		//TT 13 Aug
		SetSO_Input();
			
		SCK_Low();			/* toggle clock low */

/////
SI_Low();


		   	
}

/************************************************************************/
/* PROCEDURE: Send_Double_Byte						*/
/*									*/
/* This procedure outputs a byte shifting out 2-bit per clock rising	*/
/* edge on the the SI pin and SO pin(MSB 1st).				*/
/*									*/
/* Input:								*/
/*		out							*/
/*									*/
/* Output:								*/
/*		SI, SO							*/
/************************************************************************/
 void Send_Double_Byte(unsigned char out)
{

		SetSI_Output();
		SetSO_Output();
		
		if ((out & 0x80) ==0x80)
			{SO_High();}
		else
			{SO_Low();}
		
		if ((out & 0x40) ==0x40)
			{SI_High();}
		else
			{SI_Low();}
				
		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */
  

		if ((out & 0x20) ==0x20)
			{SO_High();}
		else
			{SO_Low();}
		
		if ((out & 0x10) ==0x10)
			{SI_High();}
		else
			{SI_Low();}
				
		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */

	
		if ((out & 0x08) ==0x08)
			{SO_High();}
		else
			{SO_Low();}
		
		if ((out & 0x04) ==0x04)
			{SI_High();}
		else
			{SI_Low();}
				
		SCK_High();			/* toggle clock high */
					
		SCK_Low();			/* toggle clock low */

		
		if ((out & 0x02) ==0x02)
			{SO_High();}
		else
			{SO_Low();}
		
		if ((out & 0x01) ==0x01)
			{SI_High();}
		else
			{SI_Low();}
				
		SCK_High();			/* toggle clock high */
		
		SetSI_Input();  //Set them as Inputs
		SetSO_Input();
			
		SCK_Low();			/* toggle clock low */

		   	
}

/************************************************************************/
/* PROCEDURE: Get_Byte							*/
/*									*/
/* This procedure inputs a byte shifting in 1-bit per clock falling	*/
/* edge on the SO pin(MSB 1st).						*/
/*									*/
/* Input:								*/
/*		SO							*/
/*									*/
/* Output:								*/
/*		None							*/
/************************************************************************/
unsigned char Get_Byte()
{
	unsigned char i = 0, in = 0, temp = 0;
	SetSO_Input();

	_delay_us(10);		//Changed from 1 ms			//HJ 14-08-2016 Change from 100us 
	for (i = 0; i < 8; i++)
	{
		in = (in << 1);			/* shift 1 place to the left or shift in 0 */
		temp = ((SPIPIN & (1<<SO))>>SO) ;			/* save input */
		SCK_High();			/* toggle clock high */
	if (temp == 1)				/* check to see if bit is high */
		in = in | 0x01;			/* if high, make bit high */
		SCK_Low();			/* toggle clock low */
	}
	return in;
	
}

/************************************************************************/
/* PROCEDURE: Get_Double_Byte						*/
/*									*/
/* This procedure inputs a byte shifting in 2-bit per clock falling	*/
/* edge on the SO pin and SI pin(MSB 1st).				*/
/*									*/
/* Input:								*/
/*		SO, SI							*/
/*									*/
/* Output:								*/
/*		None							*/
/************************************************************************/
unsigned char Get_Double_Byte()
{
	unsigned char i = 0, in = 0, temp = 0, temp1=0;
	SetSI_Input();
	SetSO_Input();
	
	for (i = 0; i < 4; i++)
	{
		in = (in << 1);			/* shift 1 place to the left or shift in 0 */
		temp = ((SPIPIN & (1<<SO))>>SO);			/* save input */
		temp1 = ((SPIPIN & (1<<SI))>>SI);			/* save input */
		SCK_High();			/* toggle clock high */

		if (temp == 1)			/* check to see if bit is high */
		{in = in | 0x01;}		/* if high, make bit high */
	
		in=(in << 1);

		if (temp1 == 1)
		{in = in | 0x01;}

		SCK_Low();			/* toggle clock low */

	}
	return in;
}


/************************************************************************/
/* PROCEDURE: Read_Status_Register					*/
/*									*/
/* This procedure read the status register and returns the byte.	*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		byte							*/
/************************************************************************/
unsigned char Read_Status_Register()
{
	unsigned char byte = 0;
	CE_Low();				/* enable device */
	Send_Byte(0x05);			/* send RDSR command */
	byte = Get_Byte();			/* receive byte */
	CE_High();				/* disable device */
	return byte;
}
//HJ 26-7-2016
/************************************************************************/
/* PROCEDURE: Read Block protection register					*/
/*									*/
/* This procedure read the Block Protection register and returns the byte.	*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		byte							*/
/************************************************************************/
void RBPR(unsigned char *buff)
{
	int i;
	cli();
	CE_Low();				/* enable device */
	Send_Byte(0x72);			/* send RDSR command */
	for (i = 0; i < 18; i++)		/* read until no_bytes is reached */
	{
		buff[i] = Get_Byte();	/* receive bytes */
	}
	CE_High();				/* disable device */
	buff[i] = '\0';
	sei();
}
//HJ 27-7-2016
/************************************************************************/
/* PROCEDURE: Write Block protection register					*/
/*									*/
/* This procedure Write the Block Protection register and returns the byte.	*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		byte							*/
/************************************************************************/
void WBPR(char state)
{
	unsigned char data = 0;
	int i;
	if(!state)			// if disable write 0xFF;
	{
		data = 0xFF;
	}
	cli();
	WP_High();
	WREN();
	//Send_Byte(0x98);	//Remove Write Protection from entire flash
	CE_Low();				/* enable device */
	Send_Byte(0x42);			/* send RDSR command */
	if(!state)			// if disable write 0xFF;
	{
		Send_Byte(0x55);
		Send_Byte(0x55);
	}
	else
	{
		Send_Byte(0x00);
		Send_Byte(0x00);
	}
	for (i = 0; i < 16; i++)		/* read until no_bytes is reached */
	{
		Send_Byte(data);
	}
	CE_High();				/* disable device */
	WRDI();
	WP_Low();
	sei();
}
/************************************************************************/
/* PROCEDURE: Read_Configration_Register					*/
/*									*/
/* This procedure read the Configration register and returns the byte.	*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		byte							*/
/************************************************************************/
unsigned char Read_Configration_Register()
{
	unsigned char byte = 0;
	CE_Low();				/* enable device */
	Send_Byte(0x35);			/* send RDSR command */
	byte = Get_Byte();			/* receive byte */
	CE_High();				/* disable device */
	return byte;
}
////////////////////
/************************************************************************/
/* PROCEDURE: EWSR							*/
/*									*/
/* This procedure Enables Write Status Register.  			*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void EWSR()
{
	CE_Low();				/* enable device */
	Send_Byte(0x50);			/* enable writing to the status register */
	CE_High();				/* disable device */
}

/************************************************************************/
/* PROCEDURE: WRSR							*/
/*									*/
/* This procedure writes a byte to the Status Register.			*/
/*									*/
/* Input:								*/
/*		byte							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void WRSR(char byte)
{
	CE_Low();				/* enable device */
	Send_Byte(0x01);			/* select write to status register */
	Send_Byte(byte);			/* data that will change the status of BPx 
					  	 or BPL (only bits 2,3,4,5,7 can be written) */
	CE_High();				/* disable the device */
}
/////////////////
/************************************************************************/
/* PROCEDURE: WRSR							*/
/*									*/
/* This procedure writes a byte to the Status Register.			*/
/*									*/
/* Input:								*/
/*		byte							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void WRCR(char byte)
{
	CE_Low();				/* enable device */
	Send_Byte(0x01);			/* select write to configration register */
	Send_Byte(0x00);
	Send_Byte(byte);			/* data that will change the status of BPx 
					  	 or BPL (only bits 2,3,4,5,7 can be written) */
	CE_High();				/* disable the device */
}
//////////////////
/************************************************************************/
/* PROCEDURE: WREN							*/
/*									*/
/* This procedure enables the Write Enable Latch.  It can also be used 	*/
/* to Enables Write Status Register.					*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void WREN()
{
	CE_Low();				/* enable device */
	Send_Byte(0x06);			/* send WREN command */
	CE_High();				/* disable device */
}

/************************************************************************/
/* PROCEDURE: WRDI							*/
/*									*/
/* This procedure disables the Write Enable Latch.			*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void WRDI()
{
	CE_Low();				/* enable device */
	Send_Byte(0x04);			/* send WRDI command */
	CE_High();				/* disable device */
}


/************************************************************************/
/* PROCEDURE: Read_ID							*/
/*									*/
/* This procedure Reads the manufacturer's ID and device ID.  It will 	*/
/* use 90h or ABh as the command to read the ID (90h in this sample).   */
/* It is up to the user to give the last byte ID_addr to determine      */
/* whether the device outputs manufacturer's ID first, or device ID 	*/
/* first.  Please see the product datasheet for details.  Returns ID in */
/* variable byte.							*/
/*									*/
/* Input:								*/
/*		ID_addr							*/
/*									*/
/* Returns:								*/
/*		byte:	ID1(Manufacture's ID = BFh or Device ID = 4Bh)	*/
/*									*/
/************************************************************************/
unsigned long Read_ID(char ID_addr)
{
	unsigned long byte;
	byte=0;
	CE_Low();				/* enable device */
	Send_Byte(0x90);			/* send read ID command (90h or ABh) */
    Send_Byte(0x00);			/* send address */
	Send_Byte(0x00);			/* send address */
	Send_Byte(ID_addr);			/* send address - either 00H or 01H */
	byte = (byte|Get_Byte())<<8;		/* receive byte */
	byte = (byte|Get_Byte());		/* receive byte */
	CE_High();				/* disable device */
	return byte;
}

/************************************************************************/
/* PROCEDURE: Jedec_ID_Read						*/
/*									*/
/* This procedure Reads the manufacturer's ID (BFh), memory type (25h)  */
/* and device ID (4Bh).  It will use 9Fh as the JEDEC ID command.    	*/
/* Please see the product datasheet for details.  			*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		IDs_Read:ID1(Manufacture's ID = BFh, Memory Type (25h), */
/*		 and Device ID (4Bh)					*/
/*									*/
/************************************************************************/
unsigned long Jedec_ID_Read() 
{
	unsigned long temp;
	
	temp = 0;
	CE_Low();			 	/* enable device */
	Send_Byte(0x9F);		 	/* send JEDEC ID command (9Fh) */
    temp = (temp | Get_Byte()) << 8; 	/* receive byte */
	temp = (temp | Get_Byte()) << 8;	
	temp = (temp | Get_Byte()); 	 	/* temp value = 0xBF254B */
	CE_High();			 	/* disable device */

	return temp;
}

/************************************************************************/
/* PROCEDURE:	Read							*/
/*									*/		
/* This procedure reads one address of the device.  It will return the 	*/
/* byte read in variable byte.						*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		Dst:	Destination Address 000000H - 7FFFFFH		*/
/*      								*/
/*									*/
/* Returns:								*/
/*		byte							*/
/*									*/
/************************************************************************/
unsigned char Read(unsigned long Dst) 
{
	unsigned char byte = 0;	
	CE_Low();				/* enable device */
	Send_Byte(0x03); 			/* read command */
	Send_Byte(((Dst & 0xFFFFFF) >> 16));	/* send 3 address bytes */
	Send_Byte(((Dst & 0xFFFF) >> 8));
	Send_Byte(Dst & 0xFF);
	byte = Get_Byte();
	CE_High();				/* disable device */
	return byte;				/* return one byte read */
}

/************************************************************************/
/* PROCEDURE:	Read_Cont						*/
/*									*/		
/* This procedure reads multiple addresses of the device and stores	*/
/* data into 256 byte buffer. Maximum byte that can be read is 256 bytes*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*      	no_bytes	Number of bytes to read	(max = 256)	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/
//void Read_Cont(unsigned long Dst, unsigned long no_bytes)			//Note : Use if Need
//{
	//unsigned long i = 0;
	//CE_Low();				/* enable device */
	//Send_Byte(0x03); 			/* read command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
	//for (i = 0; i < no_bytes; i++)		/* read until no_bytes is reached */
	//{
		//upper_256[i] = Get_Byte();	/* receive bytes */
	//}
	//CE_High();				/* disable device */
//
//}

/************************************************************************/
/* PROCEDURE:	HighSpeed_Read						*/
/*									*/		
/* This procedure reads one address of the device.  It will return the 	*/
/* byte read in variable byte.						*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		Dst:	Destination Address 000000H - 7FFFFFH		*/
/*      								*/
/*									*/
/* Returns:								*/
/*		byte							*/
/*									*/
/************************************************************************/
unsigned char HighSpeed_Read(unsigned long Dst) 
{
	unsigned char byte = 0;	

	CE_Low();				/* enable device */
	Send_Byte(0x0B); 			/* read command */
	Send_Byte(((Dst & 0xFFFFFF) >> 16));	/* send 3 address bytes */
	Send_Byte(((Dst & 0xFFFF) >> 8));
	Send_Byte(Dst & 0xFF);
	Send_Byte(0xFF);			/*dummy byte*/
	byte = Get_Byte();
	CE_High();				/* disable device */
	return byte;				/* return one byte read */
}

/************************************************************************/
/* PROCEDURE:	HighSpeed_Read_Cont					*/
/*									*/		
/* This procedure reads multiple addresses of the device and stores	*/
/* data into 256 byte buffer. Maximum byte that can be read is 256 bytes*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*      	no_bytes	Number of bytes to read	(max = 256)	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/
//void HighSpeed_Read_Cont(unsigned long Dst, unsigned long no_bytes)			//Note : Use if Need
//{
	//unsigned long i = 0;
	//CE_Low();				/* enable device */
	//Send_Byte(0x0B); 			/* read command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
	//Send_Byte(0xFF);			/*dummy byte*/
	//for (i = 0; i < no_bytes; i++)		/* read until no_bytes is reached */
	//{
		//upper_256[i] = Get_Byte();	/* receive byte and store at address 80H - FFH */
	//}
	//CE_High();				/* disable device */
//}
//

									

/************************************************************************/
/* PROCEDURE:	Fast_Read_Dual_IO					*/
/*									*/		
/* This procedure reads multiple addresses of the device and stores	*/
/* data into 256 byte buffer. Maximum byte that can be read is 256 bytes*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*      	no_bytes	Number of bytes to read	(max = 256)	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/

//void Fast_Read_Dual_IO(unsigned long Dst, unsigned long no_bytes)			//Note : Use if Need
//{ 
 	//unsigned long i = 0;
	//CE_Low();					/* enable device */
	//Send_Byte(0xbb); 				/* read command */
	//Send_Double_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	//Send_Double_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Double_Byte(Dst & 0xFF);
	//Send_Double_Byte(Dst & 0xFF);  //Dummy cycle
 	//for (i = 0; i < no_bytes; i++)			/* read until no_bytes is reached */
	//{
		//upper_256[i] = Get_Double_Byte();	/* receive byte and store at address 80H - FFH */
	//}
	//CE_High();					/* disable device */
//}

									

/************************************************************************/
/* PROCEDURE:	Fast_Read_Dual_Output					*/
/*									*/		
/* This procedure reads multiple addresses of the device and stores	*/
/* data into 256 byte buffer. Maximum byte that can be read is 256 bytes*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*      	no_bytes	Number of bytes to read	(max = 256)	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/

//void Fast_Read_Dual_Output(unsigned long Dst, unsigned long no_bytes)			//Note : Use if Need
//{ 
 	//unsigned long i = 0;
	//CE_Low();					/* enable device */
	//Send_Byte(0x3b); 				/* read command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 		/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
	//Send_Byte(Dst & 0xFF);  //Dummy cycle
 	//for (i = 0; i < no_bytes; i++)			/* read until no_bytes is reached */
	//{
		//upper_256[i] = Get_Double_Byte();	/* receive byte and store at address 80H - FFH */
	//}
	//CE_High();					/* disable device */
//}	


/************************************************************************/
/* PROCEDURE: Chip_Erase						*/
/*									*/
/* This procedure erases the entire Chip.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Chip_Erase()
{						
	CE_Low();				/* enable device */
#ifdef SST25VF064C
	Send_Byte(0x60);			/* send Chip Erase command (60h or C7h) */
#else
	Send_Byte(0xC7);
#endif
	CE_High();				/* disable device */
}




/************************************************************************/
/* PROCEDURE: Sector_Erase						*/
/*									*/
/* This procedure Sector Erases the Chip.				*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Sector_Erase(unsigned long Dst)
{


	CE_Low();				/* enable device */
	Send_Byte(0x20);			/* send Sector Erase command */
	Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	Send_Byte(((Dst & 0xFFFF) >> 8));
	Send_Byte(Dst & 0xFF);
	CE_High();				/* disable device */
}

/************************************************************************/
/* PROCEDURE: Block_Erase_32K						*/
/*									*/
/* This procedure Block Erases 32 KByte of the Chip.			*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Block_Erase_32K(unsigned long Dst)
{
	CE_Low();				/* enable device */
	#ifdef SST26VF064B
		Send_Byte(0xD8);
	#else
		Send_Byte(0x52);			/* send 32 KByte Block Erase command */
	#endif
	Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	Send_Byte(((Dst & 0xFFFF) >> 8));
	Send_Byte(Dst & 0xFF);
	CE_High();				/* disable device */
}

/************************************************************************/
/* PROCEDURE: Block_Erase_64K						*/
/*									*/
/* This procedure Block Erases 64 KByte of the Chip.			*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Block_Erase_64K(unsigned long Dst)
{
	CE_Low();				/* enable device */
	Send_Byte(0xD8);			/* send 64KByte Block Erase command */
	Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	Send_Byte(((Dst & 0xFFFF) >> 8));
	Send_Byte(Dst & 0xFF);
	CE_High();				/* disable device */
}

/************************************************************************/
/* PROCEDURE: Wait_Busy							*/
/*									*/
/* This procedure waits until device is no longer busy (can be used by	*/
/* Page-Program, Sector-Erase, Block-Erase, Chip-Erase).		*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
 				
  void Wait_Busy()
{
	unsigned long int timeout = 0;
	cli();		//Anand 18-07-15
		 unsigned char temp = 0;
		 SetSO_Input();
		 CE_Low();
		 Send_Byte(0x05);		/* send RDSR command */
		 SCK_High();
		 SCK_Low();
		 SCK_High();
		 SCK_Low();
		 SCK_High();
		 SCK_Low();
		 SCK_High();
		 SCK_Low();
		 SCK_High();
		 SCK_Low();
		 SCK_High();
		 SCK_Low();
		 SCK_High();
		 SCK_Low();
		 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register
		 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register
		 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register
		 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register
		 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register	
		 while (temp==1)
		 {
			 SCK_High();
			 SCK_Low();
			 SCK_High();
			 SCK_Low();
			 SCK_High();
			 SCK_Low();
			 SCK_High();
			 SCK_Low();
			 SCK_High();
			 SCK_Low();
			 SCK_High();
			 SCK_Low();
			 SCK_High();
			 SCK_Low();
			 SCK_High();
			 SCK_Low();
			 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register
			 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register
			 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register
			 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register
			 temp = ((SPIPIN & (1<<SO))>>SO);		//Get the Value of Busy bit in the status register
			 _delay_ms(1);				// HJ 26-7-2016         
			 if (timeout++ > FLASH_READ_TIMEOUT)				//Max chip erase time is 50msec.lets wait for 100msec
			 {
				 //flashInit();					// HJ 27-6-2016    We dont know what else to do.
				 // Lets try reset 
				 /*Reset_Hold_Low();
				 _delay_ms(5);
				 Reset_Hold_High();*/
#ifdef SST26VF064B
				 flashSwReset();
#endif

				 break;
			 }
		 }
		
		
		///////
		SCK_Low();
		SI_Low();
		////////
		 CE_High();
	sei();		//Anand 18-07-15
}	  

/************************************************************************/
/* PROCEDURE: EHLD			  				*/
/* 									*/
/* Enables Hold pin functionality					*/
/*									*/
/* Input:								*/
/*		None							*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/

void EHLD()
{
  	CE_Low();				/* enable device */
	Send_Byte(0xaa);			
	CE_High();				/* disable device */
}


/************************************************************************/
/* PROCEDURE:	LockSID							*/
/*									*/		
/* This procedure Locks the security ID setting				*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		None							*/
/*      								*/
/*									*/
/* Returns:								*/
/*		None							*/
/*									*/
/************************************************************************/
void LockSID() 
{

	CE_Low();				/* enable device */
	Send_Byte(0x85); 		
	CE_High();				/* disable device */
}

void flashPacketProgram(char* buff, uint16_t size, unsigned long addr)		//Added by Anand : Writes a Packet to flash_curr_data_write_addr
{
	unsigned long i;
	i=0;
	
	cli();		//??
	
	/*LED_PORT |= (1<<FLASH_WRITE_LED);		//Anand 18.04.2014*/
	
	CE_Low();				/* enable device */
	Send_Byte(0x02); 			/* send Byte Program command */
	Send_Byte(((addr & 0xFFFFFF) >> 16));	/* send 3 address bytes */
	Send_Byte(((addr & 0xFFFF) >> 8));
	Send_Byte(addr & 0xFF);
	for (i=0;i<size;i++)
	{	
		Send_Byte(buff[i]);	/* send byte to be programmed */
	}
	CE_High();				/* disable device */
	
	/*LED_PORT &= (~(1<<FLASH_WRITE_LED));		//Anand 18.04.2014*/
	
	sei();		//??
}

void flashWriteMR_BR()		//Anand	04-07-15
{
	MR_BR_t MR;
	if(FL_data.MR_BR_ser >= FL_MAX_MR_ENTRIES)		//Rollover of Master + Backup Flash Memory Logs
	{
		flashEraseMaster();
		flashEraseBackup();
		FL_data.MR_BR_ser = 0;
	}
	
	MR.pkt_ser = FL_data.MR_BR_ser;
	
	MR.pkt_data[RTU_WRITE_ADDR_INDEX] = FL_data.curr_data_write_addr_rtu_log&0xFF;
	MR.pkt_data[RTU_WRITE_ADDR_BYTE2] = ((FL_data.curr_data_write_addr_rtu_log&0xFFFF)>>8)&0xFF;
	MR.pkt_data[RTU_WRITE_ADDR_BYTE3] = ((FL_data.curr_data_write_addr_rtu_log&0xFFFFFF)>>16)&0xFF;	
	
	MR.pkt_data[RTU_UPLOAD_ADDR_INDEX] = FL_data.curr_upload_addr_rtu_log&0xFF;
	MR.pkt_data[RTU_UPLOAD_ADDR_BYTE2] = ((FL_data.curr_upload_addr_rtu_log&0xFFFF)>>8)&0xFF;
	MR.pkt_data[RTU_UPLOAD_ADDR_BYTE3] = ((FL_data.curr_upload_addr_rtu_log&0xFFFFFF)>>16)&0xFF;
	
	MR.pkt_data[RTU_TRANS_ID_INDEX] = FL_data.transaction_id_rtu_log&0xFF;
	MR.pkt_data[RTU_TRANS_ID_BYTE2] = ((FL_data.transaction_id_rtu_log&0xFFFF)>>8)&0xFF;
	MR.pkt_data[RTU_TRANS_ID_BYTE3] = FL_data.transaction_id_MSB_rtu_log;
/*	
	MR.pkt_data[EVT_WRITE_ADDR_INDEX] = FL_data.curr_data_write_addr_evt_log&0xFF;
	MR.pkt_data[EVT_WRITE_ADDR_BYTE2] = ((FL_data.curr_data_write_addr_evt_log&0xFFFF)>>8)&0xFF;
	MR.pkt_data[EVT_WRITE_ADDR_BYTE3] = ((FL_data.curr_data_write_addr_evt_log&0xFFFFFF)>>16)&0xFF;
	
	MR.pkt_data[EVT_UPLOAD_ADDR_INDEX] = FL_data.curr_upload_addr_evt_log&0xFF;
	MR.pkt_data[EVT_UPLOAD_ADDR_BYTE2] = ((FL_data.curr_upload_addr_evt_log&0xFFFF)>>8)&0xFF;
	MR.pkt_data[EVT_UPLOAD_ADDR_BYTE3] = ((FL_data.curr_upload_addr_evt_log&0xFFFFFF)>>16)&0xFF;
	
	MR.pkt_data[EVT_TRANS_ID_INDEX] = FL_data.transaction_id_evt_log&0xFF;
	MR.pkt_data[EVT_TRANS_ID_BYTE2] = ((FL_data.transaction_id_evt_log&0xFFFF)>>8)&0xFF;
	MR.pkt_data[EVT_TRANS_ID_BYTE3] = FL_data.transaction_id_MSB_evt_log;
*/	
	MR.pkt_data_chksm = getChecksum((unsigned char*)&MR, FL_MR_BR_LEN - MR_BR_UNUSED_LEN - 1);
	
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	flashPacketProgram((char*)&MR, FL_MR_BR_LEN, ((((unsigned long)FL_data.MR_BR_ser)*FL_MR_BR_LEN)+FL_MR_START_ADDR));
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
	
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	flashPacketProgram((char*)&MR, FL_MR_BR_LEN, ((((unsigned long)FL_data.MR_BR_ser)*FL_MR_BR_LEN)+FL_MR_BKP_ADDR));
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
	
	#ifdef _DEBUG_FLASH_UART
		UWriteString((char*)"AaYa3", UART_PC);		//Debug
		UWriteData(FL_data.MR_BR_ser, UART_PC);
		/*UWriteData(FL_data.curr_data_write_addr_dispense_log & 0xff, UART_PC);
		UWriteData((FL_data.curr_data_write_addr_dispense_log>>8) & 0xff, UART_PC);
		UWriteData((FL_data.curr_data_write_addr_dispense_log>>16) & 0xff, UART_PC);*/		//Debug
	#endif
	
	FL_data.MR_BR_ser++;
	
	/*if(FL_data.refuel_log_ser >= FL_MAX_MR_ENTRIES)		//Anand 18.04.2014		Note : TODO
	{
		FL_data.MR_BR_ser = 0;		//Anand 02.05.2014
		flashEraseMaster();		//Anand 02.05.2014
		flashEraseBackup();		//Anand 02.05.2014
		
		configRoutines(ERROR_FLASH_FLUSH_REQUIRED);		//Anand 02.05.2014
	}*/			//Error!!! It will Paralyse the whole Address Management System of Dispense Records
}

void readContToBuff(unsigned long Dst, unsigned long no_bytes, char* buff)		//Anand
{
	cli();		//Anand 28.03.14
	unsigned long i = 0;
	CE_Low();				/* enable device */
	Send_Byte(0x03); 			/* read command */
	Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	Send_Byte(((Dst & 0xFFFF) >> 8));
	Send_Byte(Dst & 0xFF);
	
	//_delay_us(100);		//HJ 14-08-2016
	for (i = 0; i < no_bytes; i++)		/* read until no_bytes is reached */
	{
		buff[i] = Get_Byte();	/* receive bytes */
	}
	CE_High();				/* disable device */
	sei();		//Anand 28.03.14
}

/*
unsigned int flashGetCurrUploadPktNum(void)		//Anand 22-4-15
{
	return FL_data.curr_upload_pkt;
}*/		//Anand 03-07-15

void updateFlashCurrAddr(void)		//Anand
{
	FL_data.MR_BR_ser = 0;
	MR_BR_t MR, BR, MR_backup;
	char pass_DR = FALSE/*, pass_RR = FALSE*/, flash_read_retries = 1;		//Anand 06-07-15
	//char refuel_log_buff[FL_RR_LEN];		//Anand 28.04.2014		//Anand 06-07-15
	//FL_log_data_t log_data;		//Anand 06-07-15
	//FL_data.last_erase_addr = 0x1000;	//Added by KP
	//FL_data.flash_overrite = FALSE;	
#ifdef _DEBUG_FLASH_UART
	char i, *MR_ptr=(char*)&MR, /**BR_ptr=(char*)&BR,*/ buff[FL_MR_BR_LEN];
#endif
	
	cli();		//Anand 28.03.14
	while(FL_data.MR_BR_ser < FL_MAX_MR_ENTRIES)
	{	
		WREN();
		readContToBuff((((unsigned long)FL_data.MR_BR_ser)*FL_MR_BR_LEN)+FL_MR_START_ADDR, FL_MR_BR_LEN, (char*)&MR);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable 
		
		WREN();
		readContToBuff((((unsigned long)FL_data.MR_BR_ser)*FL_MR_BR_LEN)+FL_MR_BKP_ADDR, FL_MR_BR_LEN, (char*)&BR);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable 
		
		#ifdef _DEBUG_FLASH_UART
			WREN();
			readContToBuff((((unsigned long)FL_data.MR_BR_ser)*FL_MR_BR_LEN)+FL_MR_BKP_ADDR, FL_MR_BR_LEN, buff);
			//Wait_Busy();
			WRDI();				// HJ 29-12-2015    // Write Disable 
		
			/*for(i=0 ; i<FL_MR_BR_LEN ; i++)
			{
				UWriteData(buff[i], UART_PC);
			}
			for(i=0 ; i<FL_MR_BR_LEN ; i++)
			{
				UWriteData(*(MR_ptr+i), UART_PC);
				UWriteData(*(BR_ptr+i), UART_PC);
			}
			for(i=0 ; i<FL_MR_BR_LEN ; i++)
			{
				UWriteData(*(MR_ptr+i), UART_PC);
				UWriteData(*(BR_ptr+i), UART_PC);
			}*/
			
		#endif

			
		// Checking if both Data from Backup & Master are Same
		//if(flashCompareMR_BR((char*)&MR, (char*)&BR))		//Anand 18-7-15
		{
			#ifdef _DEBUG_FLASH_UART
				UWriteString((char*)"CompareOK", UART_PC);
			#endif
			
			if(MR.pkt_ser == FL_data.MR_BR_ser)
			{
				#ifdef _DEBUG_FLASH_UART
					UWriteString((char*)"SerOK", UART_PC);
				#endif
				
				if(MR.pkt_data_chksm == getChecksum((unsigned char*)&MR, FL_MR_BR_LEN - MR_BR_UNUSED_LEN - 1))		//Anand 04-07-15
				{
					#ifdef _DEBUG_FLASH_UART
						UWriteString((char*)"ChksmOK", UART_PC);
					#endif
					
					FL_data.MR_BR_ser++;
					
					pass_DR = TRUE;
					//Taking Backup before Next Read
					flashCopyMR((char*)&MR, (char*)&MR_backup);
					
					#ifdef _DEBUG_FLASH_UART
						MR_ptr = (char*)&MR_backup;
						UWriteData(0xAA, UART_PC);
						for(i=0 ; i<FL_MR_BR_LEN ; i++)
						{
							UWriteData(*(MR_ptr+i), UART_PC);
						}
						UWriteData(0xAA, UART_PC);
					#endif
				}
				else
				{
					if(BR.pkt_data_chksm == getChecksum((unsigned char*)&BR, FL_MR_BR_LEN - MR_BR_UNUSED_LEN - 1))		//Anand 18-07-15
					{						
						FL_data.MR_BR_ser++;
						
						pass_DR = TRUE;
						//Taking Backup before Next Read
						flashCopyMR((char*)&BR, (char*)&MR_backup);
					}
					else
					{
						flash_read_retries++;
					}
				}
			}
			else
			{
				if(MR.pkt_ser == 0xFF)		//Anand 18-7-15
					flash_read_retries= 4;
				else
					++flash_read_retries;
			}
		}/*
		else
		{
			flash_read_retries++;
		}*/
		
		//can Send Flash Data Corrupted Error
		if(flash_read_retries>3)
		{
			break;
		}
	}
	

	if(pass_DR)	//We got a Valid ADDR Packet. Now Extract the curr_flash_addr		//Anand 19.04.2014
	{
		
		{
			FL_data.MR_BR_ser = (MR_backup.pkt_ser+1);		//Anand 29.03.2014
			
			//Anand 06-07-15
			FL_data.curr_data_write_addr_rtu_log = (MR_backup.pkt_data[RTU_WRITE_ADDR_INDEX]&0xFF) | ((((unsigned long)MR_backup.pkt_data[RTU_WRITE_ADDR_BYTE2])<<8)&(0xFF00)) | (((unsigned long)MR_backup.pkt_data[RTU_WRITE_ADDR_BYTE3]<<16)&(0xFF0000));
			FL_data.curr_upload_addr_rtu_log = (MR_backup.pkt_data[RTU_UPLOAD_ADDR_INDEX]&0xFF) | ((((unsigned long)MR_backup.pkt_data[RTU_UPLOAD_ADDR_BYTE2])<<8)&(0xFF00)) | (((unsigned long)MR_backup.pkt_data[RTU_UPLOAD_ADDR_BYTE3]<<16)&(0xFF0000));
			FL_data.transaction_id_rtu_log = (MR_backup.pkt_data[RTU_TRANS_ID_INDEX]&0xFF) + ((((unsigned long)MR_backup.pkt_data[RTU_TRANS_ID_BYTE2])<<8)&0xFF00);// + 1;
			FL_data.transaction_id_MSB_rtu_log = (MR_backup.pkt_data[RTU_TRANS_ID_BYTE3]&0xFF);

			if(flashGetCurrDataWriteAddr(RTU) != flashGetCurrUploadAddr(RTU))
			{
				
				FL_data.curr_upload_transaction_id_rtu_log_avail = 1;
				FL_rtu_log_t curr_upload_log_data_rtu;
				WREN();
				readContToBuff(FL_data.curr_upload_addr_rtu_log, FL_RTU_LOG_LEN, (char*)&curr_upload_log_data_rtu);
				Wait_Busy();		// HJ 12-08-2016
				WRDI();				// HJ 29-12-2015    // Write Disable 
				

				FL_data.curr_upload_transaction_id_rtu_log = curr_upload_log_data_rtu.transaction_id_rtu_log + (((unsigned long)curr_upload_log_data_rtu.transaction_id_MSB_rtu_log)*0x10000UL);
			}
			else
				FL_data.curr_upload_transaction_id_rtu_log_avail = 0;
#if 0			
			FL_data.curr_data_write_addr_evt_log = (MR_backup.pkt_data[EVT_WRITE_ADDR_INDEX]&0xFF) | ((((unsigned long)MR_backup.pkt_data[EVT_WRITE_ADDR_BYTE2])<<8)&(0xFF00)) | (((unsigned long)MR_backup.pkt_data[EVT_WRITE_ADDR_BYTE3]<<16)&(0xFF0000));
			FL_data.curr_upload_addr_evt_log = (MR_backup.pkt_data[EVT_UPLOAD_ADDR_INDEX]&0xFF) | ((((unsigned long)MR_backup.pkt_data[EVT_UPLOAD_ADDR_BYTE2])<<8)&(0xFF00)) | (((unsigned long)MR_backup.pkt_data[EVT_UPLOAD_ADDR_BYTE3]<<16)&(0xFF0000));
			FL_data.transaction_id_evt_log = (MR_backup.pkt_data[EVT_TRANS_ID_INDEX]&0xFF) + ((((unsigned long)MR_backup.pkt_data[EVT_TRANS_ID_BYTE2])<<8)&0xFF00);// + 1;
			FL_data.transaction_id_MSB_evt_log = (MR_backup.pkt_data[EVT_TRANS_ID_BYTE3]&0xFF);
			
			if(flashGetCurrDataWriteAddr(EVENT) != flashGetCurrUploadAddr(EVENT))
			{
				FL_data.curr_upload_transaction_id_evt_log_avail = 1;
				FL_evt_log_t curr_upload_log_data_evt;
				WREN();
				readContToBuff(FL_data.curr_upload_addr_evt_log, FL_EVT_LOG_LEN, (char*)&curr_upload_log_data_evt);
				Wait_Busy();		// HJ 12-08-2016
				WRDI();				// HJ 29-12-2015    // Write Disable 
				
				FL_data.curr_upload_transaction_id_evt_log = curr_upload_log_data_evt.transaction_id + (((unsigned long)curr_upload_log_data_evt.transaction_id_MSB)*0x10000UL);
			}
			else
				FL_data.curr_upload_transaction_id_evt_log_avail = 0;
			//~Anand 06-07-15			
#endif			
#ifdef _DEBUG_FLASH_UART
			UWriteString((char*)"Pass:", UART_PC);		//Debug
			/*UWriteData(FL_data.curr_data_write_addr_rtu_log&0xff, UART_PC);
			UWriteData((FL_data.curr_data_write_addr_rtu_log>>8)&0xff, UART_PC);
			UWriteData((FL_data.curr_data_write_addr_rtu_log>>16)&0xff, UART_PC);		//Debug*/
			UWriteInt(FL_data.curr_data_write_addr_rtu_log,UART_PC);
			UWriteData(',',UART_PC);
			UWriteInt(FL_data.curr_upload_addr_rtu_log,UART_PC);
			
#endif
		}
	}
	else	if(FL_data.MR_BR_ser==0)		//if 1st Packet is FF, it means Bowser/Flash is New
	{
		
		FL_data.curr_data_write_addr_rtu_log = FL_RTU_START_ADDR;
		FL_data.curr_upload_addr_rtu_log = FL_RTU_START_ADDR;
		FL_data.transaction_id_rtu_log = 0;
		FL_data.transaction_id_MSB_rtu_log = 0;
#if 0		
		FL_data.curr_data_write_addr_evt_log = FL_EVT_START_ADDR;
		FL_data.curr_upload_addr_evt_log = FL_EVT_START_ADDR;
		FL_data.transaction_id_evt_log = 0;
		FL_data.transaction_id_MSB_evt_log = 0;
#endif		
		flashEraseMaster();			//Anand 19.04.2014
		flashEraseBackup();			//Anand 19.04.2014
		flashEraseDRSector();			//Anand 02.05.2014
		flashWriteMR_BR();
		
#ifdef _DEBUG_FLASH_UART
		UWriteString((char*)"init_addr", UART_PC);		//Debug
		UWriteInt(FL_data.curr_data_write_addr_rtu_log,UART_PC);
		UWriteData(',',UART_PC);
		UWriteInt(FL_data.curr_upload_addr_rtu_log,UART_PC);
#endif
	}
	else
	{
		FL_data.curr_data_write_addr_rtu_log = FL_RTU_START_ADDR;
		//FL_data.curr_data_write_addr_evt_log = FL_EVT_START_ADDR;
		SYS_ERR_LED_PORT |= (1<<SYS_ERR_LED);		//Anand 18.04.2014
// 		#ifdef _DEBUG_ERR_LCD		//Anand 30.04.2014
// 		configRoutines(ERROR_FLASH_DATA_RECORD_MISMATCH);
// 		#else
// 		configRoutines();
// 		#endif
	}
		
	sei();		//Anand 28.03.14
#ifdef DEBUG_FL_ADDR
	char temp[20];
	sprintf(temp, "rtu_addr = %08X\n",FL_data.curr_data_write_addr_rtu_log);
	UWriteString(temp, UART_PC);
//	sprintf(temp, "evt_addr = %lu\n",FL_data.curr_data_write_addr_evt_log);
//	UWriteString(temp, UART_PC);		
#endif	
}

void flashEraseMaster(void)		//Anand
{
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	Sector_Erase(FL_MR_START_ADDR);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
}

void flashEraseBackup(void)		//Anand
{
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	Sector_Erase(FL_MR_BKP_ADDR);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
}
#if 0
void flashEraseRefuelSector(void)		//Anand 04-07-2015
{
	for(unsigned long addr = FL_EVT_START_ADDR ; addr < FL_EVT_MAX_ADDR ; addr+=0x1000)
	{
#ifdef FLASH_WP_ENABLE
		remove_block_protection();
#endif
		WREN();
		Sector_Erase(addr);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
		WBPR(0);
#endif
	}
}
#endif
void flashEraseDRSector(void)		//Anand 02.05.2014
{
	unsigned long addr = FL_RTU_START_ADDR;
	
	/*while*/if(addr < FL_RTU_MAX_ADDR)		//Anand 19.06.2014
	{
#ifdef FLASH_WP_ENABLE
		remove_block_protection();
#endif
		WREN();
		Sector_Erase(addr/*FL_RR_START_ADDR*/);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
		WBPR(0);
#endif		
		addr += 0x1000;
	}
}

void flashEraseSector(unsigned long addr, unsigned long max_addr)		//Anand 20-7-15
{
	
	if(addr < max_addr)		//Anand 20-7-15
	{
#ifdef FLASH_WP_ENABLE
		remove_block_protection();
#endif
		WREN();
		Sector_Erase(addr);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
		WBPR(0);
#endif
	}
}

char flashCompareMR_BR(char* pck1_ptr, char* pck2_ptr)		//Anand 22.05.2014
{
	char ok = TRUE;
	
	for(char i=0 ; i<FL_MR_BR_LEN ; i++)
		if((*(pck1_ptr+i)) != (*(pck2_ptr+i)))
		{
			ok = FALSE;
			break;
		}
	
	return ok;
}

void flashCopyMR(char* src, char* dst)
{
	for(char i=0 ; i<FL_MR_BR_LEN ; i++)
		(*(dst+i)) = (*(src+i));
}


unsigned long flashGetCurrDataWriteAddr(char type)		//Anand 04-07-15
{
	unsigned long retVal = 0;
	switch(type)
	{
		case RTU :
			retVal = FL_data.curr_data_write_addr_rtu_log; 
		break;
#if 0		
		case EVENT :
		
			retVal = FL_data.curr_data_write_addr_evt_log;
		break;
#endif
	}
	
	return retVal;
}


unsigned long flashGetCurrUploadAddr(char type)		//Anand 04-07-15
{
	unsigned long retVal = 0;
	switch(type)
	{
		case RTU :
			retVal = FL_data.curr_upload_addr_rtu_log;
		break;
#if 0		
		case EVENT :
			retVal = FL_data.curr_upload_addr_evt_log;
		break;
#endif
	}
	
	return retVal;
}


/*
long flashGetCurrRefuelWriteLogSerNo()		//Anand 04.07.2015
{
	return ((FL_data.transaction_id_MSB_evt_log*0x10000UL) + FL_data.transaction_id_evt_log);
}*/

int8_t flashGetSerMR_BR(void)		//Anand
{
	return FL_data.MR_BR_ser;
}

void flashWriteLR(FL_rtu_log_t *log_data)		//Anand
{	
	if(FL_data.transaction_id_rtu_log == 0xFFFF)		
	{
		++FL_data.transaction_id_MSB_rtu_log;
	}
	
	log_data->transaction_id_rtu_log = ++FL_data.transaction_id_rtu_log;		// Start transaction with 1
	log_data->transaction_id_MSB_rtu_log = FL_data.transaction_id_MSB_rtu_log;		//Anand 02-07-15
	
	if(FL_data.curr_data_write_addr_rtu_log >= FL_RTU_MAX_ADDR)		//Anand 08.10.2014	Note : TODO
	{
		FL_data.curr_upload_addr_rtu_log = SECTOR_SIZE + FL_RTU_START_ADDR;
			
		if(FL_data.curr_upload_addr_rtu_log >= FL_RTU_MAX_ADDR)		//Anand 20-07-15 For one sector size.
		{
			FL_data.curr_upload_addr_rtu_log = FL_RTU_START_ADDR;
		}
		
		FL_data.curr_data_write_addr_rtu_log = FL_RTU_START_ADDR;		//Anand 02.05.2014
		flashEraseSector(FL_RTU_START_ADDR, FL_RTU_MAX_ADDR);		//Anand 20-07-15
		
		FL_data.curr_upload_transaction_id_rtu_log_avail = 0;		//Anand 20-7-15 We did a skip so new upload tx id is required.
		//configRoutines(ERROR_FLASH_FLUSH_REQUIRED);		//Anand 16-7-15 Don't need any message!
	}
	else    //Anand 16-7-15	We don't want two skips.
		if(FL_data.curr_upload_addr_rtu_log == FL_data.curr_data_write_addr_rtu_log)		//Anand 16-7-15	We want this for successful execution.
		{
			if(((FL_data.curr_data_write_addr_rtu_log % SECTOR_SIZE) == 0) && (FL_data.curr_data_write_addr_rtu_log != FL_RTU_START_ADDR))		//Anand 18-07-15
			{
				flashEraseSector(FL_data.curr_data_write_addr_rtu_log, FL_RTU_MAX_ADDR);		//Anand 20-07-15
				FL_data.curr_upload_addr_rtu_log += SECTOR_SIZE;
				if(FL_data.curr_upload_addr_rtu_log >= FL_RTU_MAX_ADDR)
				{
					FL_data.curr_upload_addr_rtu_log = FL_RTU_START_ADDR;
				}
			
				FL_data.curr_upload_transaction_id_rtu_log_avail = 0;		//Anand 20-7-15 We did a skip so new upload tx id is required.
			}
		}
	
	
	//HJ 12-8-2016       Calculate checksum
	log_data->chksum = getChecksum((unsigned char*)log_data, (FL_RTU_LOG_LEN-1));		//Anand 19.04.2014
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	flashPacketProgram((char*)log_data, FL_RTU_LOG_LEN, FL_data.curr_data_write_addr_rtu_log);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write DisableWRDI();
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
		
	FL_data.curr_data_write_addr_rtu_log += (FL_RTU_LOG_LEN);
}
#if 0
void flashWriteER(FL_evt_log_t* log_data)
{	
	if(FL_data.transaction_id_evt_log == 0xFFFF)		//Anand 02-07-15
	{
		++FL_data.transaction_id_MSB_evt_log;
	}
	
	log_data->transaction_id = ++FL_data.transaction_id_evt_log;		// Start transaction with 1
	log_data->transaction_id_MSB = FL_data.transaction_id_MSB_evt_log;		//Anand 02-07-15	
	
	if(FL_data.curr_data_write_addr_evt_log >= FL_EVT_MAX_ADDR)		//Anand 08.10.2014
	{
		FL_data.curr_upload_addr_evt_log = SECTOR_SIZE + FL_EVT_START_ADDR;
		
		if(FL_data.curr_upload_addr_evt_log >= FL_EVT_MAX_ADDR)		//Anand 20-07-15 For one sector size.
		{
			FL_data.curr_upload_addr_evt_log = FL_EVT_START_ADDR;
		}
		
		FL_data.curr_data_write_addr_evt_log = FL_EVT_START_ADDR;		//Anand 02.05.2014
		flashEraseSector(FL_EVT_START_ADDR, FL_EVT_MAX_ADDR);		//Anand 20-07-15
		
		FL_data.curr_upload_transaction_id_evt_log_avail = 0;		//Anand 20-7-15 We did a skip so new upload tx id is required.
		//configRoutines(ERROR_FLASH_FLUSH_REQUIRED);		//Anand 16-7-15 Don't need any message!
	}
	else  //Anand 16-7-15
	{
		if(FL_data.curr_upload_addr_evt_log == FL_data.curr_data_write_addr_evt_log)  //Anand 16-7-15
		{
			if(((FL_data.curr_data_write_addr_evt_log % SECTOR_SIZE) == 0) && (FL_data.curr_data_write_addr_evt_log != FL_EVT_START_ADDR))		//Anand 18-07-15
			{
				flashEraseSector(FL_data.curr_data_write_addr_evt_log, FL_EVT_MAX_ADDR);		//Anand 20-07-15
				FL_data.curr_upload_addr_evt_log += SECTOR_SIZE;
				if(FL_data.curr_upload_addr_evt_log >= FL_EVT_MAX_ADDR)
				{
					FL_data.curr_upload_addr_evt_log = FL_EVT_START_ADDR;
				}
			
				FL_data.curr_upload_transaction_id_evt_log_avail = 0;		//Anand 20-7-15 We did a skip so new upload tx id is required.
			}
		}
	}
	//log_data->chksum = getChecksum((unsigned char*)log_data, (FL_EVT_LOG_LEN - 1 - EVT_UNUSED_LEN));
	log_data->chksum = getChecksum((unsigned char*)log_data, (FL_EVT_LOG_LEN - 1));
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	flashPacketProgram((char*)log_data, FL_EVT_LOG_LEN, FL_data.curr_data_write_addr_evt_log);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif


	FL_data.curr_data_write_addr_evt_log += (FL_EVT_LOG_LEN);
	
}
#endif
void flashSwReset(void)
{
	CE_Low();
	Send_Byte(0x66);
	Send_Byte(0x99);
	CE_High();
}

unsigned long flashGetTransID(char type)		//Anand 07-04-15
{
	unsigned long retVal = 0;
	
	switch(type)
	{
		case RTU :
			retVal = (FL_data.transaction_id_rtu_log + (((unsigned long)FL_data.transaction_id_MSB_rtu_log)*0x10000UL));		//Anand 06-07-15
		break;
#if 0		
		case EVENT :
			retVal = (FL_data.transaction_id_evt_log + (((unsigned long)FL_data.transaction_id_MSB_evt_log)*0x10000UL));		//Anand 06-07-15
		break;
#endif	
	}	

	return retVal;
}

unsigned long flashGetCurrUploadTransactionId(char type)		//Anand 06-07-15
{
	unsigned long retVal = 0;
	
	switch(type)
	{
		case RTU :
			if(FL_data.curr_upload_transaction_id_rtu_log_avail)
				retVal = FL_data.curr_upload_transaction_id_rtu_log;
			else
			{
				if(flashGetCurrDataWriteAddr(RTU) != flashGetCurrUploadAddr(RTU))
				{
					FL_data.curr_upload_transaction_id_rtu_log_avail = 1;
					FL_rtu_log_t curr_upload_log_data;
					WREN();
					readContToBuff(FL_data.curr_upload_addr_rtu_log, FL_RTU_LOG_LEN, (char*)&curr_upload_log_data);
					WRDI();				// HJ 29-12-2015    // Write Disable 
					
					retVal = FL_data.curr_upload_transaction_id_rtu_log = curr_upload_log_data.transaction_id_rtu_log + (((unsigned long)curr_upload_log_data.transaction_id_MSB_rtu_log)*0x10000UL);
				}
				else
					retVal = FL_data.curr_upload_transaction_id_rtu_log_avail = FL_data.transaction_id_rtu_log + (((unsigned long)FL_data.transaction_id_MSB_rtu_log)*0x10000);		//Anand 20-7-15
			}
		break;
#if 0		
		case EVENT :
		
			if(FL_data.curr_upload_transaction_id_evt_log_avail)
				retVal = FL_data.curr_upload_transaction_id_evt_log;
			else
			{
				if(flashGetCurrDataWriteAddr(EVENT) != flashGetCurrUploadAddr(EVENT))
				{
					FL_data.curr_upload_transaction_id_evt_log_avail = 1;
					FL_evt_log_t curr_upload_log_data_event;
					WREN();
					readContToBuff(FL_data.curr_upload_addr_evt_log, FL_EVT_LOG_LEN, (char*)&curr_upload_log_data_event);
					WRDI();				// HJ 29-12-2015    // Write Disable 
					
					retVal = FL_data.curr_upload_transaction_id_evt_log = curr_upload_log_data_event.transaction_id + (((unsigned long)curr_upload_log_data_event.transaction_id_MSB)*0x10000UL);
				}
				else
					retVal = FL_data.curr_upload_transaction_id_evt_log_avail = FL_data.transaction_id_evt_log + (((unsigned long)FL_data.transaction_id_MSB_evt_log)*0x10000);		//Anand 20-7-15
			}
			
		break;
#endif
	}
	
	return retVal;	
}

void flashSetTransID(char type, unsigned long curr_id)		//Anand 04-07-15
{
	switch(type)
	{
		case RTU :
			FL_data.transaction_id_rtu_log = curr_id & 0xFFFF;
			FL_data.transaction_id_MSB_rtu_log = 0xFF & ((curr_id & 0xFF0000)>>16);		//Anand 10-7-15
		break;
#if 0		
		case EVENT :
		
			FL_data.transaction_id_evt_log = curr_id & 0xFFFF;
			FL_data.transaction_id_MSB_evt_log = 0xFF & ((curr_id & 0xFF0000)>>16);		//Anand 10-7-15
		
		break;
#endif
	}
	
	flashWriteMR_BR();
}

void flashClrTransID(void)
{
	FL_data.transaction_id_rtu_log = 0;
	FL_data.transaction_id_MSB_rtu_log = 0;		//Anand 02-07-15
#if 0	
	FL_data.transaction_id_evt_log = 0;
	FL_data.transaction_id_MSB_evt_log = 0;
#endif
}
void flashClrRTULogs(void)	
{
	unsigned int sectors_to_erase = ((FL_RTU_MAX_ADDR - FL_RTU_START_ADDR) / SECTOR_SIZE) + 1;
	unsigned long addr = FL_RTU_START_ADDR;
	
	while(sectors_to_erase--)
	{
		wdt_reset();
#ifdef FLASH_WP_ENABLE
		remove_block_protection();
#endif
		WREN();
		Sector_Erase(addr);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
		WBPR(0);
#endif		
		addr += SECTOR_SIZE;
	}
}
#if 0
void flashClrEventLogs(void)
{
	unsigned int sectors_to_erase = ((FL_EVT_MAX_ADDR - FL_EVT_START_ADDR) / SECTOR_SIZE) + 1;
	unsigned long addr = FL_EVT_START_ADDR;
	
	while(sectors_to_erase--)
	{
#ifdef FLASH_WP_ENABLE
		remove_block_protection();
#endif
		WREN();
		Sector_Erase(addr);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
		WBPR(0);
#endif
		addr += SECTOR_SIZE;
	}
}
#endif
void flashClrMRBR(void)
{
#ifdef FLASH_WP_ENABLE
		remove_block_protection();
#endif
		WREN();
		Sector_Erase(FL_MR_START_ADDR);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
		WBPR(0);
#endif
		
#ifdef FLASH_WP_ENABLE
		remove_block_protection();
#endif
		WREN();
		Sector_Erase(FL_MR_BKP_ADDR);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
		WBPR(0);
#endif
}
int validate_FL_addr(char type, unsigned long addr)
{
	if(type == RTU)
	{
		if((addr >= FL_RTU_START_ADDR) && (addr < FL_RTU_MAX_ADDR))
		{
			return TRUE;
		}
	}
#if 0
	else if(type == EVENT)
	{
		if((addr >= FL_EVT_START_ADDR) && (addr < FL_EVT_MAX_ADDR))
		{
			return TRUE;
		}
	}
#endif
	return FALSE;
}
void flashClearFreqUpdatedConfig(void)
{
	cli();
	
	// Lets clear the entire circular buffer
	flashEraseFreqUpdDataSector();
	freq_updated_data.cnt = 1;
	//Lets copy latest data on first row
	freq_updated_data.chksum = getChecksum((unsigned char*)(&freq_updated_data), (sizeof(freq_updated_data_t) - 1));		//22.05.2014
	//eeprom_write_block(&bowser_cfg_data_freq_updated_data, (void*)addr, sizeof(freq_updated_data_t));		//R
	flashWriteFreqUpdData(&freq_updated_data, FL_FREQ_UPD_DATA_START_ADDR );		
#ifdef _DEBUG_UNSENT_LOGS
UWriteString("freq_clear:",UART_PC);
#endif
	sei();
}

void flashWriteFreqUpdatedConfig(void)
{
#ifdef _DEBUG_UNSENT_LOGS
	UWriteString((char*)"unsent:", UART_PC);		//Debug
#endif
	cli();
	++freq_updated_data.cnt;
	unsigned long addr = FL_FREQ_UPD_DATA_START_ADDR + ((freq_updated_data.cnt - 1) * sizeof(freq_updated_data_t));
	if((addr < FL_FREQ_UPD_DATA_MAX_ADDR) && freq_updated_data.cnt)
	{
		freq_updated_data.chksum = getChecksum((unsigned char*)(&freq_updated_data), (sizeof(freq_updated_data_t) - 1/* - BD_FREQ_UNUSED_LEN*/));
		flashWriteFreqUpdData(&freq_updated_data, addr );		//R
#ifdef _DEBUG_UNSENT_LOGS
		UWriteString((char*)"LOGs:", UART_PC);		//Debug
		UWriteInt(freq_updated_data.unsent_rtu_logs_GPRS,UART_PC);
		UWriteString((char*)"CNT:", UART_PC);		//Debug
		UWriteInt(freq_updated_data.cnt,UART_PC);
#endif
	}

	else
	{
		freq_updated_data.cnt = 1;			//Lets keep cnt within 1 to 10, otherwise it will create problem after 255 transactions.
		flashClearFreqUpdatedConfig();
#ifdef _DEBUG_UNSENT_LOGS
		UWriteString((char*)"clrW:", UART_PC);		//Debug
#endif
	}

	sei();
}

void flashEraseFreqUpdDataSector(void)		//KP 20-4-2018
{
	unsigned long addr = FL_FREQ_UPD_DATA_START_ADDR;
	

#ifdef FLASH_WP_ENABLE
		remove_block_protection();
#endif
		WREN();
		Sector_Erase(addr/*FL_RR_START_ADDR*/);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
		WBPR(0);
#endif
}

void flashWriteFreqUpdData(freq_updated_data_t *log_data, unsigned long addr)
{	
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	//flashPacketProgram((char*)log_data, FL_DR_LEN, FL_data.curr_data_write_addr_dispense_log);
	flashPacketProgram((char*)log_data, FL_FREQ_UPD_DATA_SIZE, addr);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write DisableWRDI();
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif

}

void flashReadFreqUpdData(freq_updated_data_t *freq_updated_data, unsigned long addr,int size)
{
	WREN();
	readContToBuff(addr, size, (char *)freq_updated_data);
	WRDI();				// HJ 29-12-2015    // Write Disable
}
char readFreqUpdData_flash(void)
{
	char read_ok = 1;
	int i = 0;
	unsigned long addr;
	freq_updated_data_t freq_updated_data_;
	unsigned char j=0;
	j = 1;
	cli();		//Anand 20.06.2014
	for(i=0 ; i<3 ; i++)
	{
		do{
			addr = (FL_FREQ_UPD_DATA_START_ADDR + (j++ * FL_FREQ_UPD_DATA_SIZE));
			if(addr < FL_FREQ_UPD_DATA_MAX_ADDR)
			{
				flashReadFreqUpdData(&freq_updated_data, (addr - FL_FREQ_UPD_DATA_SIZE), FL_FREQ_UPD_DATA_SIZE);
				flashReadFreqUpdData(&freq_updated_data_, addr, FL_FREQ_UPD_DATA_SIZE);
			
				if(freq_updated_data_.cnt == 0xFF)
				{
					break;
				}
			}
			else
			{
				memcpy(&freq_updated_data, &freq_updated_data_, FL_FREQ_UPD_DATA_SIZE);		//At the end the second one is holding the Gold
				//lets erase the sector and put this on first location
				freq_updated_data.cnt = 1;
#ifdef _DEBUG_UNSENT_LOGS
UWriteString((char*)"clrR:", UART_PC);		//Debug
#endif
				flashClearFreqUpdatedConfig();
				break;
			}
		}while((freq_updated_data.cnt + 1) == freq_updated_data_.cnt);		//Anand 02-04-16
		
		if(freq_updated_data.chksum == getChecksum((unsigned char*)(&freq_updated_data), (sizeof(freq_updated_data_t) - 1/* - BD_FREQ_UNUSED_LEN*/)))	//Anand 02-04-16 check how your wrote in writecfg
		{
			break;
		}
		else
		{
			read_ok = 0;
		}
	}
	sei();		//Anand 20.06.2014
	if(i>=3)
	{
		read_ok = 0;
		//		write_defaults();		//Writes Default Values to EEPROM
		
		//Log Sys error and go to config mode
		
		SYS_ERR_LED_PORT|=(1<<SYS_ERR_LED);
	}
	if (!read_ok)
	{
		freq_updated_data.unsent_rtu_logs_GPRS = 0;
	}
	return read_ok;
}

#ifdef DEBUG_BOOT_HEX
void init_hex_write_data(void)
{
	hex_file_address = FL_HEX_FILE_START_ADDR;
	#ifdef FLASH_WP_ENABLE
	remove_block_protection();
	#endif
	WREN();
	Sector_Erase(hex_file_address);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable
	#ifdef FLASH_WP_ENABLE
	WBPR(0);
	#endif
}
unsigned char write_hex_file_data(unsigned char *data, unsigned int len)
{
	unsigned int i = 0;
	unsigned char chksum = 0;
	unsigned char value = 0;
	unsigned char hex_data[32] = {0};
	unsigned char ack = 0;
	#ifdef DEBUG_HEX_FILE_STORE
	UWriteString("Receive data = ", UART_PC);
	UWriteBytes((char *)data, len, UART_PC);
	#endif
	if(data[0] == ':')
	{
		#ifdef DEBUG_HEX_FILE_STORE
		UWriteString("pack_init:k", UART_PC);
		#endif
		hex_data[i] = data[i];
		for (i = 1; i < len; i++)
		{
			if ((data[i] >= '0') && (data[i] <= '9'))
			{
				data[i] -= '0';
			}
			else if ((data[i] >= 'A') && (data[i] <= 'F'))
			{
				data[i] = ((data[i] - 'A') + 10);
			}
			else
			{
				#ifdef DEBUG_HEX_FILE_STORE
				UWriteString("Invalid Bytes", UART_PC);
				#endif
				break;
			}
			if (i % 2 == 0)
			{
				value = (value << 4) | (data[i]);
				hex_data[i / 2] = value;
				//UWriteData(value, UART_PC);
				if (i <= (len - 3))
				{
					chksum += value;
				}
				else
				{
					//UWriteData(chksum, UART_PC);
					chksum = ~chksum + 1;
					//UWriteData(chksum, UART_PC);
					if (value == chksum)
					{
						//UWriteString("Flash Packet is = ", UART_PC);
						//UWriteBytes(hex_data, 32, UART_PC);
						#ifdef FLASH_WP_ENABLE
						remove_block_protection();
						#endif
						WREN();
						flashPacketProgram((char*)hex_data, 32, hex_file_address);
						Wait_Busy();
						WRDI();				// HJ 29-12-2015    // Write DisableWRDI();
						#ifdef FLASH_WP_ENABLE
						WBPR(0);
						#endif
						#ifdef DEBUG_FLASH
						UWriteString("\natg_add:",UART_PC);
						UWriteInt(FL_data.curr_data_write_addr_ATG_log,UART_PC);

						#endif
						ack = 1;
						if (hex_data[1] == 0)
						{
							//eeprom_write_byte((uint8_t *)E2P_FLASH_HEX_FILE_UPDATE, TRUE);
							//hex_file_ready(TRUE);
							break;
						}
						else
						{
							hex_file_address += 32;
							if (hex_file_address % SECTOR_SIZE == 0)
							{
								#ifdef FLASH_WP_ENABLE
								remove_block_protection();
								#endif
								WREN();
								Sector_Erase(hex_file_address);
								Wait_Busy();
								WRDI();				// HJ 29-12-2015    // Write Disable
								#ifdef FLASH_WP_ENABLE
								WBPR(0);
								#endif
							}
						}
					}
					else
					{
						#ifdef DEBUG_HEX_FILE_STORE
						UWriteString("Checksum_fail", UART_PC);
						#endif
					}
				}
			}
			else
			{
				value = data[i];
			}
		}
	}
	else
	{
		#ifdef DEBUG_HEX_FILE_STORE
		UWriteString("Packet Initial Fail", UART_PC);
		#endif
	}
	return ack;
}
unsigned char read_hex_file_data(void)
{
	unsigned long flash_file_address = FL_HEX_FILE_START_ADDR;
	unsigned char ack = 0;
	unsigned char temp_data[32] = {0}, chksum = 0;
	int i = 0;
	do
	{
		#ifdef DEBUG_FLASH
		//UWriteString((char *)"\naddr:",UART_PC);
		//UWriteInt(flash_file_address,UART_PC);
		#endif
		WREN();
		readContToBuff(flash_file_address, 32, (char *)temp_data);		//KP 28-03-2015
		Wait_Busy();	//??				// HJ 12-08-2016 Uncomment
		WRDI();
		for (i = 1; (i <= (temp_data[1] + 5)); i++)
		{
			chksum += temp_data[i];
		}
		chksum = ~chksum + 1;
		if (chksum == temp_data[i])
		{
			UWriteString((char *)"OK", UART_PC);
		}
		else
		{
			_delay_ms(3000);
			UWriteString((char *)"NAK", UART_PC);
		}
		UWriteData('\n', UART_PC);
		UWriteBytes((char *)temp_data, 32, UART_PC);
		flash_file_address += 32;
	} while ((temp_data[1] != 0) && (temp_data[1] != 0xFF));
	if ((temp_data[1] == 0) || (temp_data[1] == 0xFF))
	{
		ack = 1;
	}
	return ack;
}
#endif
void flashWriteHex(unsigned char *hex_data, unsigned long *addr)
{
	#ifdef DEBUG_FLASH
	//UWriteString((char *)"\naddr:",UART_PC);
	//UWriteInt(*addr,UART_PC);
	#endif
	#ifdef FLASH_WP_ENABLE
	remove_block_protection();
	#endif
	WREN();
	flashPacketProgram((char *)hex_data, 32, *addr);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write DisableWRDI();
	#ifdef FLASH_WP_ENABLE
	WBPR(0);
	#endif

	if (hex_data[1] == 0)
	{
		//eeprom_write_byte((uint8_t *)E2P_FLASH_HEX_FILE_UPDATE, TRUE);
		hex_file_ready(TRUE);
	}
	else
	{
		*addr += 32;
	}
}
void read_flash_file_row(unsigned long addr, unsigned char *buff)
{
	WREN();
	readContToBuff(addr, 32, (char *)buff);		//KP 28-03-2015
	Wait_Busy();	//??				// HJ 12-08-2016 Uncomment
	WRDI();
}
void flash_hex_file_verify(void)
{
#ifdef DEBUG_HEX_FILE_STORE
UWriteString((char *)"hex_verify:",UART_PC);
#endif
	unsigned long flash_file_address = FL_HEX_FILE_START_ADDR;
	//unsigned char ack = 0;
	unsigned int cnt = 0;
	unsigned char temp_data[32] = {0}, chksum = 0;
	int i = 0;
	do
	{
		
		WREN();
		readContToBuff(flash_file_address, 32, (char *)temp_data);		//KP 28-03-2015
		Wait_Busy();	//??				// HJ 12-08-2016 Uncomment
		WRDI();
#ifdef DEBUG_HEX_FILE_STORE
		UWriteInt(flash_file_address,UART_PC);
		UWriteData('.',UART_PC);
		UWriteBytes((char*)&temp_data,32,UART_PC);
#endif
		for (i = 1; (i <= (temp_data[1] + 5)); i++)
		{
			chksum += temp_data[i];
		}
		chksum = ~chksum + 1;
		if (chksum == temp_data[i])
		{
			wdt_reset();
#ifdef DEBUG_HEX_FILE_STORE
			UWriteString((char *)"OK", UART_PC);
#endif
		}
		else
		{
			//set_hex_file_updt_state(UPDATE_FAIL);
			//set_flash_file_rqst_type(HEX_NO_RQST);
			//hex_file_ready(FALSE);
			UWriteString((char *)"NAK", UART_PC);
			break;
			
		}
		flash_file_address += 32;
		cnt++;
		UWriteInt(cnt, UART_PC);
	} while ((temp_data[1] != 0) && (temp_data[1] != 0xFF));
	if (temp_data[1] == 0)
	{
		hex_file_ready(TRUE);
		set_hex_file_updt_state(DWNLD_CMPLT);
		//ack = 1;
	}
	else
	{
		set_hex_file_updt_state(UPDATE_FAIL);
	}
	set_frmwr_update_state(HEX_FILE_UPDATE_RELEASE);
	//return ack;
}

void flashClrHexFile(void)
{
	unsigned int sectors_to_erase = ((FL_MAX_HEX_FILE_ADDR - FL_HEX_FILE_START_ADDR) / SECTOR_SIZE) + 1;
	unsigned long addr = FL_HEX_FILE_START_ADDR;
	
	while(sectors_to_erase--)
	{
		#ifdef FLASH_WP_ENABLE
		remove_block_protection();
		#endif
		WREN();
		Sector_Erase(addr);
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable
		#ifdef FLASH_WP_ENABLE
		WBPR(0);
		#endif
		addr += SECTOR_SIZE;
	}
}