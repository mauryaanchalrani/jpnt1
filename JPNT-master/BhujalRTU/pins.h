#ifndef __PINS_H__
#define __PINS_H__

#include <avr/io.h>
#include "_debug.h"
#include "config.h"

//#define LCD_PORT (PORTD)
//#define LCD_DDR  (DDRD)
/*
#define LED5	(PA0)		//Not Used
#define LED6	(PA1)		//Not Used
*/
/*

//Added by kamlesh 18-5-2017
#define SOFT_MFRC522_IRQ_RST_PORT		(PORTH)
#define SOFT_MFRC522_IRQ_RST_DDR		(DDRH)
#define SOFT_MFRC522_IRQ_RST_PIN		(PINH)
#define SOFT_MFRC522_IRQ				(PH6)
#define SOFT_MFRC522_RST				(PH7)

#define SOFT_SPI_PORT			(PORTB)
#define SOFT_SPI_PIN			(PINB)
#define SOFT_SPI_DDR			(DDRB)
#define SOFT_MOSI				(PB2)
#define SOFT_MISO				(PB3)
#define SOFT_SCK				(PB1)
#define SOFT_CS					(PB5)
//End here 18-5-2017*/

#define SPIPORT (PORTB)
#define SPIPIN  (PINB)
#define SPIDDR  (DDRB)
/*

#define RFID_RST_DDR		(DDRB)
#define RFID_RST_PIN		(PINB)
#define RFID_RST_PORT		(PORTB)
#define RFID_CS_DDR			(DDRA)
#define RFID_CS_PIN			(PINA)
#define RFID_CS_PORT		(PORTA)
#define _resetPowerDownPin	(PB4)	//1
#define _chipSelectPin		(PA6)*/
//#define CE1		(PB0)	/*FLASH 1*/
#define SCK		(PB1)
#define SI		(PB2)
#define SO		(PB3)
//#define RST		(PB5)
//#define WP1		(PB5)
/*
#define FLASH_CNTRL_DDR	(DDRB)
#define FLASH_CNTRL_PORT	(PORTB)
*/
#define FLASH_CNTRL_DDR	(DDRB)
#define FLASH_CNTRL_PORT	(PORTB)
#define CE1		(PB0)	/*FLASH 2*/

#define FLASH_WP_DDR	(DDRB)
#define FLASH_WP_PORT	(PORTB)
#define WP1		(PB4)		/*Anand 19.08.2014*/
// #define WP1		(PK6)		

#define FLASH_RST_DDR	(DDRB)
#define FLASH_RST_PORT	(PORTB)
//#define WP1		(PB4)		/*Anand 19.08.2014*/
#define RST		(PB5)


#define MOSI		(PB2)	//4
#define MISO		(PB3)	//3

/*
#define SPIPORT 	(PORTB)
#define SPIDDR  	(DDRB)
*/
/*
#define LTC_SS      (PA7)		/ *LTC2418* /
#define LTC_SS_PORT (PORTA)
#define LTC_SS_DDR  (DDRA)

#define E2P_1_WP_PORT (PORTA)
#define E2P_1_WP_DDR (DDRA)
#define E2P_1_WP 	  (PA4)

#define E2P_2_WP_PORT (PORTA)
#define E2P_2_WP_DDR (DDRA)
#define E2P_2_WP 	  (PA5)*/

/*
#define LCD_RS_PORT (PORTD)
#define LCD_RS_DDR  (DDRD)
#define LCD_RS_PIN PG0

#define LCD_EN_PORT (PORTD)
#define LCD_EN_DDR  (DDRD)
#define LCD_EN_PIN PG1
*/


#define GPRS_PWRKEY_DDR		(DDRE)
#define GPRS_PWRKEY_PORT	(PORTE)
#define GPRS_PWRKEY_PIN		(PE2)

/*#define GPRS_STATUS_DDR		(DDRL)
#define GPRS_STATUS_PIN		(PINL)
#define GPRS_STATS			(PL7)*/

#define PULSER_DDR	(DDRE)
#define PULSER_PIN	(PINE)
#define PULSER		(INT4) //04/12/18 VC: need to change because mb rtu i/p is taken on this pin 

#define PULSER1_DDR	(DDRE)
#define PULSER1_PIN	(PINE)
#define PULSER1		(PE5) //04/12/18 VC: need to change because mb rtu i/p is taken on this pin


#define POWER_DOWN_DDR	(DDRK)	//11-03-2019 SK: has to be change according to the power down pin
#define POWER_DOWN_PIN	(PINK)
#if BOARD_VER == v2_0
	#define POWER_DOWN		(PK6)
#endif
#if BOARD_VER == v1_0
	#define POWER_DOWN		(PK0)
#endif
/*

#ifdef USE_LCD_RW_PIN
#define LCD_RW_PORT (PORTB)
#define LCD_RW_DDR  (DDRB)
#define LCD_RW_PIN PB7
#endif

#ifdef _DEBUG_PULSER
#define LCD_D0 PA0		/ *Anand 30.08.2014* /
#define LCD_D1 PA1
#endif

#define SWITCHING_SWITCH PA0		//Anand 03.10.2014
#define SWITCHING_PIN PINA		//Anand 03.10.2014*/

//#define AUX_RELAY_PORT  (PORTG)		//Assigning tamperorly to pump for Gorahi
//#define AUX_RELAY_DDR  (DDRG)
//#define AUX_RELAY  		(PG3)
/*
#define AUX_RELAY_PORT  (PORTG)
#define AUX_RELAY_DDR  (DDRG)
#define AUX_RELAY  		(PG3)*/		//Anand 20-7-15

/*****************************  PORTD ****************************************/

/*
#define RTC_SCL    (PD0)
#define RTC_SDA    (PD1)
#define USART1_RXD (PD2)
#define USART1_TXD (PD3)*/

/*
#define PUMP_DDR 		   (DDRG)
#define PUMP_PORT 		   (PORTG)

#if PUMP_SUPPLY_TYPE == DC_PUMP
	#define PUMP	  		   (PG4)		//Anand 20-7-15
#else
	#define PUMP	  		   (PG3) //temp for Gorahi
#endif

#define ORIFICE_DDR    (DDRE)
#define ORIFICE_PORT   (PORTE)		//Anand 16.04.2014
#define BIG_ORIFICE		   (PE2)

//#define SMALL_ORIFICE_DDR  (DDRE)
//#define SMALL_ORIFICE_PORT (PORTE)
#define SMALL_ORIFICE 	   (PE3)


/ *****************************  PORTE **************************************** /

#define USART0_RXD 		 (PE0)
#define USART0_TXD 		 (PE1)

*/

#define ACTIVITY_LED_DDR (DDRC)
#define ACTIVITY_LED_PORT (PORTC)
#define ACTIVITY_LED	 (PC0)

#define SYS_ERR_LED_DDR		(DDRC)
#define SYS_ERR_LED_PORT 	(PORTC)
#define SYS_ERR_LED			(PC1)

#define POWER_ON_DDR	(DDRG)		//Anand 08-07-15
#define POWER_ON_PORT	(PORTG)		//Anand 08-07-15
#define POWER_ON		(PG5)		//Anand 08-07-15
/*
#define MAIN_BAT_SIG_DDR (DDRK)		//Anand 08-07-15
#define MAIN_BAT_SIG_PIN (PINK)		//Anand 08-07-15
#define MAIN_BAT_SIG	 (PK7)		//Anand 08-07-15*/

/*
#define BACKUP_BAT_SIG_DDR (DDRF)		//Anand 08-07-15
#define BACKUP_BAT_SIG_PIN (PINF)		//Anand 08-07-15
#define BACKUP_BAT_SIG	 (PF0)		//Anand 08-07-15*/
//#define ADC_PORT 		  (PORTK)
//#define ADC_DDR	 		  (DDRK)

/*
#define IGNITION_DDR		(DDRE)
#define IGNITION_PIN		(PINE)
#define IGNITION			(PE4)

#define INPUT_1_DDR		(DDRC)
#define INPUT_1_PIN		(PINC)
#define INPUT_1			(PC6)

#define INPUT_2_DDR		(DDRC)
#define INPUT_2_PIN		(PINC)
#define INPUT_2			(PC5)

#define OUTPUT_1_DDR		(DDRC)
#define OUTPUT_1_PORT		(PORTC)
#define OUTPUT_1			(PC4)

#define OUTPUT_2_DDR		(DDRC)
#define OUTPUT_2_PORT		(PORTC)
#define OUTPUT_2			(PC3)*/
/*****************************  PORTL ****************************************/

#if 0
#define KP_DDR_R 		  (DDRL)		/ *ROW* /
#define KP_PORT_R         (PORTL)
#define KP_PIN_R 		  (PINL)
#ifdef ALPHANUMERIC_KEYPAD
#define KP_TRISMASK_R     (0xF8)
#else
#define KP_TRISMASK_R     (0xFF)
#endif
//#define ADC_PORT 		  (PORTF)
//#define ADC_DDR	 		  (DDRF)
#ifdef ALPHANUMERIC_KEYPAD
#define KP_R_0			  (3)
#else
#define KP_R_0			  (0)
#endif
/*****************************  PORTC ****************************************/

#define KP_DDR_C 	  (DDRC)		/*COLUMN*/
#define KP_PORT_C 	  (PORTC)
#define KP_PIN_C 	  (PINC)
#ifdef ALPHANUMERIC_KEYPAD
#define KP_TRISMASK_C (0x1F)
#else
#define KP_TRISMASK_C (0x7F)
#endif
#define KP_C_0		  (0)

#endif
/*
#define WIFI_CHPD_DDR	(DDRK)		
#define WIFI_CHPD_PORT 	(PORTK)
#define WIFI_CHPD 		(PK0)

#define WIFI_CHPD_DDR	(DDRJ)
#define WIFI_CHPD_PORT 	(PORTJ)
#define WIFI_CHPD 		(PJ6)*/

// HJ 28-09-2016
/*
#define BACKUP_BATTERY_CHARGER_STOP_DDR		(DDRF)
#define BACKUP_BATTERY_CHARGER_STOP_PORT	(PORTF)
#define BACKUP_BATTERY_CHARGER_STOP			(PF1)*/
#define GENERAL_IO_DDR			(DDRL) 
#define GENERAL_IO_PORT			(PORTL)
#define GENERAL_IO_PIN			(PINL)


#define CFG_SW_DDR			(DDRC) //DDRC
#define CFG_SW_PIN			(PINC)
#define CONFIG_SW			(PC4)

#define MODBUS_DDR			(DDRB) //DDRC
#define MODBUS_PORT			(PORTB)
#define MOD_TXEN_PIN		(PB6)   //0

#define MODBUS_INPUT_DDR	(DDRK)
#define MODBUS_OUTPUT_DDR	(DDRH)

#define SW_LED_DDR			(DDRC)
#define SW_LED_PORT			(PORTC)
#define SW_LED_PIN			(PINC)

#define SW_LED_SW1			(PC5)
#define SW_LED_SW2			(PC6)
#define SW_LED_SW3			(PC7)
//-------------------------------------------------------------------------------------------------
//	KS0108 Pin Config
//-------------------------------------------------------------------------------------------------
#define KS0108_DATA_DIR		DDRA
#define KS0108_DATA_PORT	PORTA
#define KS0108_DATA_PIN		PINA

#if BOARD_VER == v2_0
	#define KS0108_CTRL_DIR		DDRD
	#define KS0108_CTRL_PORT	PORTD

	#define KS0108_CS_CTRL_DIR		DDRD
	#define KS0108_CS_CTRL_PORT		PORTD

	#define KS0108_RS			(4)
	#define KS0108_EN			(5)
	//#define KS0108_RW			(2)
#endif

#if BOARD_VER == v1_0
	#define KS0108_CTRL_DIR		DDRJ
	#define KS0108_CTRL_PORT	PORTJ

	#define KS0108_CS_CTRL_DIR		DDRJ
	#define KS0108_CS_CTRL_PORT		PORTJ

	#define KS0108_RS			(2)
	#define KS0108_EN			(3)
//#define KS0108_RW			(2)
#endif


#define KS0108_CS1			(7)
#define KS0108_CS2			(6)

/*
#define KS0108_DATA_DIR		DDRA
#define KS0108_DATA_PORT	PORTA
#define KS0108_DATA_PIN		PINA

#define KS0108_CTRL_DIR		DDRJ
#define KS0108_CTRL_PORT	PORTJ

#define KS0108_CS_CTRL_DIR		DDRJ
#define KS0108_CS_CTRL_PORT		PORTJ

#define KS0108_RS			(2)
#define KS0108_EN			(3)

#define KS0108_CS1			(7)
#define KS0108_CS2			(6)
*/

//	I/O PORT
#if BOARD_VER == v2_0
	#define MOTOR_DDR	(DDRH)
	#define MOTOR_PORT	(PORTH)
	#define MOTOR		(PH2)

	#define ALARM_DDR	(DDRH)
	#define ALARM_PORT	(PORTH)
	#define ALARM		(PH3)
	
	#define SNOOZE_SW_DDR	(DDRK)
	#define SNOOZE_SW_PIN	(PINK)
	#define SNOOZE_SW		(PK0)

	#define MOTOR_AC_DDR	(DDRJ)
	#define MOTOR_AC_PIN	(PINJ)
	#define MOTOR_AC		(PJ2)

#endif
#if BOARD_VER == v1_0
	#define MOTOR_DDR	(DDRH)
	#define MOTOR_PORT	(PORTH)
	#define MOTOR		(PH6)

	#define ALARM_DDR	(DDRB)
	#define ALARM_PORT	(PORTB)
	#define ALARM		(PB7)
	
	#define SNOOZE_SW_DDR	(DDRE)
	#define SNOOZE_SW_PIN	(PINE)
	#define SNOOZE_SW		(PE7)
#endif

#endif
