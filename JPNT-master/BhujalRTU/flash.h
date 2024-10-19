
#ifndef FLASH_H_
#define FLASH_H_

/*
Device Communication Protocol(pinout related) functions:

Functions                    		Function
------------------------------------------------------------------
init					Initializes clock to set up mode 0.
Send_Byte				Sends one byte using SI pin to send and
shift out 1-bit per clock rising edge
Send_Double_Byte			Sends one byte using SI pin to send and
shift out 2-bit per clock rising edge
Get_Byte				Receives one byte using SO pin to receive and shift
in 1-bit per clock falling edge
Get_Double_Byte				Receives one byte using SO pin to receive and shift
in 2-bit per clock falling edge
CE_High					Sets Chip Enable pin of the serial flash to high
CE_Low					Clears Chip Enable of the serial flash to low
Reset_Hold_Low				Clears Hold pin to make serial flash hold or Reset the device
Reset_Hold_High				Unholds the serial flash
WP_Low					Clears WP pin to make serial flash write protected
WP_High					Disables write protection pin




Device Operation Instruction functions:

Functions                    		Function
------------------------------------------------------------------
Read_Status_Register			Reads the status register of the serial flash
EWSR							Enables the Write Status Register
WRSR							Performs a write to the status register
WREN							Write enables the serial flash
WRDI							Write disables the serial flash
Read_ID							Reads the manufacturer ID and device ID
Jedec_ID_Read					Reads the Jedec ID
Fast_Read_Dual_IO	 			Reads Memory with Dual Address Input and Data Output
Fast_Read_Dual_Output			Reads Memory with Dual Output
Read							Reads one byte from the serial flash and returns byte(max of 25 MHz CLK frequency)
Read_Cont						Reads multiple bytes(max of 25 MHz CLK frequency)
HighSpeed_Read					Reads one byte from the serial flash and returns byte(max of 66 MHz CLK frequency)
HighSpeed_Read_Cont				Reads multiple bytes(max of 66 MHz CLK frequency)
Page_Program					Programs 1 to 256 data bytes
Dual_Input_Page_Program			Programs 1 to 256 data bytes using dual input
Chip_Erase						Erases entire serial flash
Sector_Erase					Erases one sector (4 KB) of the serial flash
Block_Erase_32K					Erases 32 KByte block memory of the serial flash
Block_Erase_64K					Erases 64 KByte block memory of the serial flash
Wait_Busy						Polls status register until busy bit is low
EHLD							Enable Hold pin functionality

*/


//"C" LANGUAGE DRIVERS

/********************************************************************/
/* Copyright Silicon Storage Technology, Inc. (SST), 1994-2009	    */
/* Example "C" language Driver of SST25VF064C Serial Flash	    */
/* Hardik Patel, Silicon Storage Technology, Inc.                   */
/*                                                                  */
/* Revision 1.0, July 25, 2011				    */
/*								    */
/********************************************************************/

// include files
#include <avr/io.h>
#include "pins.h"

#include "events.h"
#if 0
typedef struct
{
	unsigned char  MR_BR_ser;								// For Master/Backup Record Serial Number
	
	unsigned long curr_data_write_addr_lvl_log;		//for Data
	unsigned long curr_upload_addr_lvl_log;
	unsigned int transaction_id_lvl_log;
	unsigned char transaction_id_MSB_lvl_log;
		
	unsigned long curr_data_write_addr_evt_log;		//for Data
	unsigned long curr_upload_addr_evt_log;
	unsigned int transaction_id_evt_log;
	unsigned char transaction_id_MSB_evt_log;
	
	unsigned long curr_upload_transaction_id_evt_log;				
	char curr_upload_transaction_id_evt_log_avail;					

	unsigned long curr_upload_transaction_id_lvl_log;				
	char curr_upload_transaction_id_lvl_log_avail;					
}FL_data_t;		
#endif


typedef struct
{
	unsigned char  MR_BR_ser;								// For Master/Backup Record Serial Number
	
	unsigned long curr_data_write_addr_rtu_log;		//for Data
	unsigned long curr_upload_addr_rtu_log;
	unsigned int transaction_id_rtu_log;
	unsigned char transaction_id_MSB_rtu_log;
	
	/*unsigned long curr_data_write_addr_evt_log;		//for Data
	unsigned long curr_upload_addr_evt_log;
	unsigned int transaction_id_evt_log;
	unsigned char transaction_id_MSB_evt_log;
	
	unsigned long curr_upload_transaction_id_evt_log;
	char curr_upload_transaction_id_evt_log_avail;
*/

	unsigned long curr_upload_transaction_id_rtu_log;
	char curr_upload_transaction_id_rtu_log_avail;
}FL_data_t;

//#define SST25VF064C	//Remove comment if you are using old flash

#ifndef SST25VF064C 
#define SST26VF064B
#endif	

#ifdef _DEBUG_ADD_DUMMY_DISPENSE_LOGS_ON_KEY_PRESS		//HJ 27-07-2015
	#define MAX_NUM_OF_RTU_LOGS	(644)					//Change with care = Need many alteration in code regarding communication and all.
#else
	#define MAX_NUM_OF_RTU_LOGS	(65535)	//(65535)					//Change with care = Need many alteration in code regarding communication and all.		//HJ 27-07-2015														//Change max store logs from 65535 to 32767 because packet is just doubled				//HJ 11-07-2017 
#endif

#define FL_RTU_LOG_LEN ((unsigned long)(32))		//TT 17 Sep 17 

#define FL_MR_DATA_LEN (18)		/* Master/Backup Packet Data Length	= 3bytes */		//Anand 04-07-15
#define MR_BR_UNUSED_LEN (12)	//Anand 04-07-15		//To make a packet of 16
#define FL_MR_BR_LEN	(32)		/* Master/Backup Packet Length */

#define FL_MR_START_ADDR ((unsigned long)0x00)
#define FL_MR_BKP_ADDR ((unsigned long)0x1000)

#define FL_RTU_START_ADDR ((unsigned long)0x2000)
#define FL_RTU_MAX_ADDR	 (((unsigned long)MAX_NUM_OF_RTU_LOGS * FL_RTU_LOG_LEN) + FL_RTU_START_ADDR)

#define FL_MAX_MR_ENTRIES (100)		/*Anand 02.07.2015*/		//It is of 24 Bytes so care is needed for greater than 170 Entries! 4096/24 = 170.67
#define SECTOR_SIZE		  ((unsigned long)0x1000)		//Anand 31-10-15

#define FLASH_READ_TIMEOUT		(100)

//Frequently updated data moved from eeprom to flash // KP 20-4-2018

#define FL_FREQ_UPD_DATA_SIZE			((unsigned long)16)
#define FL_FREQ_UPD_DATA_MAX_ENTRIES	(100)
#define FL_FREQ_UPD_DATA_START_ADDR ((unsigned long)0x408000)		//Reserve one sector fto store circular buffer for frequently updated data
#define FL_FREQ_UPD_DATA_MAX_ADDR ((unsigned long)(FL_FREQ_UPD_DATA_START_ADDR + (FL_FREQ_UPD_DATA_SIZE * FL_FREQ_UPD_DATA_MAX_ENTRIES)))

#define FL_HEX_PKT_LEN					(32)
#define FL_HEX_PKT_MAX_NUM				(unsigned long)(0x2000)
//#define	FL_HEX_FILE_START_ADDR			((unsigned long)0x600000)
#define	FL_HEX_FILE_START_ADDR			((unsigned long)0x600000)
#define FL_MAX_HEX_FILE_ADDR			((unsigned long)(FL_HEX_FILE_START_ADDR + (FL_HEX_PKT_LEN * FL_HEX_PKT_MAX_NUM)))

enum		//Anand 04-07-15
{
	RTU /*LEVEL*/ = 0,
	//EVENT
};

#define RTU_WRITE_ADDR_INDEX 0		//321	3=MSB
#define RTU_WRITE_ADDR_BYTE2 1
#define RTU_WRITE_ADDR_BYTE3 2
	
#define RTU_UPLOAD_ADDR_INDEX 3
#define RTU_UPLOAD_ADDR_BYTE2 4
#define RTU_UPLOAD_ADDR_BYTE3 5

#define RTU_TRANS_ID_INDEX 6
#define RTU_TRANS_ID_BYTE2 7
#define RTU_TRANS_ID_BYTE3 8
/*	
#define EVT_WRITE_ADDR_INDEX 9
#define EVT_WRITE_ADDR_BYTE2 10
#define EVT_WRITE_ADDR_BYTE3 11
	
#define EVT_UPLOAD_ADDR_INDEX 12
#define EVT_UPLOAD_ADDR_BYTE2 13
#define EVT_UPLOAD_ADDR_BYTE3 14

#define EVT_TRANS_ID_INDEX 15
#define EVT_TRANS_ID_BYTE2 16
#define EVT_TRANS_ID_BYTE3 17
*/
#define FLASH_WP_ENABLE
//typedefs

typedef struct
{
	unsigned char pkt_ser;
	unsigned char pkt_data[FL_MR_DATA_LEN];	
	char dummy[MR_BR_UNUSED_LEN];
	unsigned char pkt_data_chksm;		//Anand 18-07-15
} __attribute__((packed)) MR_BR_t;		//Anand 04-07-2015


/* Function Prototypes */

void Portassign();
void DisplayLED(char Led_Stat);

void SetSI_Output();
void SetSI_Input();
void SetSO_Output();
void SetSO_Input();

/* Function Prototypes */

void flashInit();
void Send_Byte(unsigned char out);
void Send_Double_Byte(unsigned char out);
unsigned char Get_Byte();
unsigned char Get_Double_Byte();
void SCK_High();
void SCK_Low();
void SI_High();
void SI_Low();
void SO_High();
void SO_Low();
void CE_High();
void CE_Low();
void Reset_Hold_Low();
void Reset_Hold_High();
void WP_Low();
void WP_High();
unsigned char Read_Status_Register();
void EWSR();
void WRSR(char byte);
void WREN();
void WRDI();


//HJ 26-7-2016
unsigned char Read_Configration_Register();
void RBPR(unsigned char *buff);			//Read Block-Protection Register
void WBPR(char state);				// HJ 27-7-2016
void WRCR(char byte);
void remove_block_protection(void);
void enable_wp_pin(void);
/////////////
unsigned long Read_ID(char ID_addr);
unsigned long Jedec_ID_Read();
//unsigned char Read(unsigned long Dst);
//void Read_Cont(unsigned long Dst, unsigned long no_bytes);
void readContToBuff(unsigned long Dst, unsigned long no_bytes, char*);		//Anand
//unsigned char HighSpeed_Read(unsigned long Dst);
//void HighSpeed_Read_Cont(unsigned long Dst, unsigned long no_bytes);
void Chip_Erase();
void Sector_Erase(unsigned long Dst);
void Block_Erase_32K(unsigned long Dst);
void Block_Erase_64K(unsigned long Dst);
void Wait_Busy();
//void Fast_Read_Dual_IO(unsigned long Dst, unsigned long no_bytes);
//void Fast_Read_Dual_Output(unsigned long Dst, unsigned long no_bytes);
//void Page_Program(unsigned long Dst);
//void Dual_Input_Page_Program(unsigned long Dst);
void EHLD();
//void ReadSID(unsigned char Dst, unsigned char security_length);
//void ProgSID();
//void LockSID();
void flashPacketProgram(char*, uint16_t, unsigned long);		//Added by Anand
void flashWriteMR_BR();		//Anand
//unsigned int flashGetCurrUploadPktNum(void);		//Anand 03-07-15
void updateFlashCurrAddr(void);		//Anand
unsigned long flashGetCurrDataWriteAddr(char);		//Anand 04-07-15
unsigned long flashGetCurrUploadAddr(char);		//Anand 04-07-15
//unsigned long flashGetlastEraseAddr(void);
//long flashGetCurrRefuelWriteLogSerNo();		//Anand 28.04.2014
int8_t flashGetSerMR_BR(void);				//Anand
void flashWriteLR(FL_rtu_log_t *);			//Anand	28.03.14
void flashEraseBackup(void);				//Anand
void flashEraseMaster(void);				//Anand
void flashEraseRefuelSector(void);		//Anand 28.04.2014
void flashEraseDRSector(void);		//Anand 02.05.2014
void flashEraseSector(unsigned long, unsigned long);		//Anand 20-07-15
char flashCompareMR_BR(char*, char*);	//Anand 28.03.14
void flashCopyMR(char*, char*);			//Anand 28.03.14
void flashWriteER(FL_evt_log_t*);		//Anand 28.04.2014
void flashSwReset(void);
unsigned long flashGetTransID(char);		//Kamlesh 30-03-2015		//Anand 02-07-15
unsigned long flashGetCurrUploadTransactionId(char);		//Anand 06-07-15
void flashSetTransID(char, unsigned long);		//Anand 04-07-15
void flashClrTransID(void);					//TT 13-5-15
void flashReadAuthorizedTruck(unsigned int);
void flashWriteTruckConfig(void);
void flashClrTruckConfig(unsigned int);	//Anand 31-10-15
void flashClrRTULogs(void);		//HJ 29-12-2015
//void flashClrEventLogs(void);			//HJ 29-12-2015
void flashClrMRBR(void);				//HJ 29-12-2015

int validate_FL_addr(char, unsigned long); // HJ 12-09-2016

void flashClearFreqUpdatedConfig(void);
void flashWriteFreqUpdatedConfig(void);
void flashEraseFreqUpdDataSector(void);
void flashWriteFreqUpdData(freq_updated_data_t *log_data, unsigned long addr);
void flashReadFreqUpdData(freq_updated_data_t *,unsigned long addr,int size);
//HJ 26-04-2018
char readFreqUpdData_flash(void);

#ifdef DEBUG_BOOT_HEX
void init_hex_write_data(void);
unsigned char write_hex_file_data(unsigned char *, unsigned int);
unsigned char read_hex_file_data(void);
#endif
void flashWriteHex(unsigned char *, unsigned long *);
void read_flash_file_row(unsigned long, unsigned char *);
void flash_hex_file_verify(void);
void flashClrHexFile(void);
#endif



/*
#ifndef FLASH_H_
#define FLASH_H_

/ *
Device Communication Protocol(pinout related) functions:

Functions                    		Function
------------------------------------------------------------------
init					Initializes clock to set up mode 0.
Send_Byte				Sends one byte using SI pin to send and
shift out 1-bit per clock rising edge
Send_Double_Byte			Sends one byte using SI pin to send and
shift out 2-bit per clock rising edge
Get_Byte				Receives one byte using SO pin to receive and shift
in 1-bit per clock falling edge
Get_Double_Byte				Receives one byte using SO pin to receive and shift
in 2-bit per clock falling edge
CE_High					Sets Chip Enable pin of the serial flash to high
CE_Low					Clears Chip Enable of the serial flash to low
Reset_Hold_Low				Clears Hold pin to make serial flash hold or Reset the device
Reset_Hold_High				Unholds the serial flash
WP_Low					Clears WP pin to make serial flash write protected
WP_High					Disables write protection pin




Device Operation Instruction functions:

Functions                    		Function
------------------------------------------------------------------
Read_Status_Register			Reads the status register of the serial flash
EWSR							Enables the Write Status Register
WRSR							Performs a write to the status register
WREN							Write enables the serial flash
WRDI							Write disables the serial flash
Read_ID							Reads the manufacturer ID and device ID
Jedec_ID_Read					Reads the Jedec ID
Fast_Read_Dual_IO	 			Reads Memory with Dual Address Input and Data Output
Fast_Read_Dual_Output			Reads Memory with Dual Output
Read							Reads one byte from the serial flash and returns byte(max of 25 MHz CLK frequency)
Read_Cont						Reads multiple bytes(max of 25 MHz CLK frequency)
HighSpeed_Read					Reads one byte from the serial flash and returns byte(max of 66 MHz CLK frequency)
HighSpeed_Read_Cont				Reads multiple bytes(max of 66 MHz CLK frequency)
Page_Program					Programs 1 to 256 data bytes
Dual_Input_Page_Program			Programs 1 to 256 data bytes using dual input
Chip_Erase						Erases entire serial flash
Sector_Erase					Erases one sector (4 KB) of the serial flash
Block_Erase_32K					Erases 32 KByte block memory of the serial flash
Block_Erase_64K					Erases 64 KByte block memory of the serial flash
Wait_Busy						Polls status register until busy bit is low
EHLD							Enable Hold pin functionality

* /


//"C" LANGUAGE DRIVERS

/ ******************************************************************** /
/ * Copyright Silicon Storage Technology, Inc. (SST), 1994-2009	    * /
/ * Example "C" language Driver of SST25VF064C Serial Flash	    * /
/ * Hardik Patel, Silicon Storage Technology, Inc.                   * /
/ *                                                                  * /
/ * Revision 1.0, July 25, 2011				    * /
/ *								    * /
/ ******************************************************************** /

// include files
#include <avr/io.h>

#define HIGHEST_ADDRESS (0x7FFFFF)		//Anand
#define MANUFACTURER_ID (0xBF)
#define DEVICE_ID (0x4F)
#define MANUFACTURER_ID_ADDRESS (0x00)
#define DEVICE_ID_ADDR (0x01)
// defines
//#define SPIPORT (PORTF)
//#define SPIDDR  (DDRF)
//#define SI		(PF7)
//#define SO		(PF6)
//#define CE		(PF5)
//#define RST		(PF4)
//#define WP		(PF3)
//#define SCK		(PF2)


/ * Function Prototypes * /

void Portassign();
void DisplayLED(char Led_Stat);

void SetSI_Output();
void SetSI_Input();
void SetSO_Output();
void SetSO_Input();

/ * Function Prototypes * /

void flash_init();
void Send_Byte(unsigned char out);
void Send_Double_Byte(unsigned char out);
unsigned char Get_Byte();
unsigned char Get_Double_Byte();
void SCK_High();
void SCK_Low();
void SI_High();
void SI_Low();
void SO_High();
void SO_Low();
void CE_High();
void CE_Low();
void Reset_Hold_Low();
void Reset_Hold_High();
void WP_Low();
void WP_High();
unsigned char Read_Status_Register();
void EWSR();
void WRSR(char byte);
void flash_WREN();
void WRDI();

unsigned long Read_ID(char ID_addr);
unsigned long Jedec_ID_Read();
unsigned char Read(unsigned long Dst);
void Read_Cont(unsigned long Dst, unsigned long no_bytes);
unsigned char HighSpeed_Read(unsigned long Dst);
void HighSpeed_Read_Cont(unsigned long Dst, unsigned long no_bytes);
void Chip_Erase();
void Sector_Erase(unsigned long Dst);
void Block_Erase_32K(unsigned long Dst);
void Block_Erase_64K(unsigned long Dst);
void Wait_Busy();
void Fast_Read_Dual_IO(unsigned long Dst, unsigned long no_bytes);
void Fast_Read_Dual_Output(unsigned long Dst, unsigned long no_bytes);
void Page_Program(unsigned long Dst);
void Dual_Input_Page_Program(unsigned long Dst);
void EHLD();
void ReadSID(unsigned char Dst, unsigned char security_length);
void ProgSID();
void LockSID();
void flash_byte_Program(char data,unsigned long Dst);  //ADD BY ROHIT
char upper_256[256];	/ * global array to store read data * /
unsigned char security_id_32[32];	/ * global array to store security_id data * /


#endif */
