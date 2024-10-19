
//LCD DISPLAY MESSAGES
//enter not more than 20 char in a line

//POWER ON DISPLAY SCREEN
#include "sys_info.h"
#include <avr/pgmspace.h>
//#define  POWER_ON_LCD_LINE1								"      FUEL EYE      "
//#define  POWER_ON_LCD_LINE2								"     E4ENGINEER     "
//#define  POWER_ON_LCD_LINE3								FIRMWARE_VER

//BOOTING UP MESSAGES
#define BOOTING_LCD_LINE1								"     BOOTING UP     "
#define BOOTING_LCD_LINE2								"%"
#define BOOTING_LCD_LINE3								"  BOOTING COMPLETE  "
/*
#define LOADING_LCD_LINE1								"     LOADING....    "
#define BOOTING_LCD_LINE4								"    INIT. KEYPAD    "
#define BOOTING_LCD_LINE5								"    INIT. PULSAR    "
#define BOOTING_LCD_LINE6								"    INIT.  UART     "
#define BOOTING_LCD_LINE6_								"   INIT. UART OK    "
#define BOOTING_LCD_LINE7								"    INIT.  FLASH    "
#define BOOTING_LCD_LINE8								"    INIT.  TIMER    "
#define BOOTING_LCD_LINE9								"    UPDT.  TIME     "
#define BOOTING_LCD_LINE10								"UPDT. TOTAL DISPENSE"
#define BOOTING_LCD_LINE11								"     INIT. WIFI     "		//Anand 17-4-15

//ADMIN MODE MESSAGES
//#define ADMIN_MODE_LCD_LINE1							"   WELCOME ADMIN    "	
//#define ADMIN_MODE_LCD_LINE2							"   ENTER PASSWORD   "
//#define ADMIN_MODE_LCD_LINE3							"PRESS ENTER FOR LOG "
//#define ADMIN_MODE_LCD_LINE4							"     END OF LOG     "

//GLOBAL MESSAGES
#define	 AUTHENTICATION_MENU_LCD_LINE1					"  INSERT NOZZLE IN  "
#define	 AUTHENTICATION_MENU_LCD_LINE2					"        TANK        "
#define	 AUTHENTICATION_MENU_LCD_LINE3					" PRESS NOZZLE BUTTON"	/ *Anand 18.06.2014* /
#define	 AUTHENTICATED_LCD_LINE							"    AUTHENTICATED   "


#define	 CONFIRM_DISPENSE_LCD_LINE1					"VEHICLE : "
#define	 CONFIRM_DISPENSE_LCD_LINE2					"QTY     : "
// HJ 01-06-2016
//#define	 CONFIRM_DISPENSE_LCD_LINE3					"      CONTINUE?     "
#define	 CONFIRM_DISPENSE_LCD_LINE3					"ODO     : "
//#define	 CONFIRM_DISPENSE_LCD_LINE4					"ENTER(YES)   ESC(NO)"
#define	 CONFIRM_DISPENSE_LCD_LINE4					"    ENTER / ESC     "

#if FD_TYPE == COMMERCIAL
#define  DISPENSING_MENU_LCD_LINE1						"TARGET   :"
#define  DISPENSING_MENU_LCD_LINE2						"DISPENSED:"
#define DISPENSE_MENU_LCD_DATA_START_POS				(10)
#endif
#if FD_TYPE == RETAIL
#define  DISPENSING_MENU_LCD_LINE1						"TARGET :"
#define  DISPENSING_MENU_LCD_LINE2						"AMOUNT :"
#define  DISPENSING_MENU_LCD_LINE3						"Rs     :"
#define DISPENSE_MENU_LCD_DATA_START_POS				(8)
#endif

//#define  DISPENSING_MENU_LCD_LINE3						"					 "
#ifdef _DEBUG_LCD
	#define  DISPENSING_MENU_LCD_LINE4						"FUEL REMAIN:"
#else
	#define  DISPENSING_MENU_LCD_LINE4					" DISPENSE  COMPLETE "
	//#define  DISPENSING_ABORT_MENU_LCD_LINE4			"DISPENSE ABORTED"
#endif
*/

/*
#define  CONFIG_MODE_LCD_LINE1							"    CONFIGURATION   "
#define  CONFIG_MODE_LCD_LINE2							"        MODE        "
#define  REFUEL_MODE_LCD_LINE1							"      ENTER QTY.    "
#define  REFUEL_MODE_LCD_LINE2							" REFUELLING STARTED "
#define  REFUEL_MODE_LCD_LINE3							"   MEASURING FUEL   "
#define  REFUEL_MODE_LCD_LINE4							"  PRESS REFUEL KEY  "
#define  REFUEL_MODE_LCD_LINE5							"      WHEN DONE     "
#define  REFUEL_MODE_LCD_LINE6							"  ENTER QTY. AGAIN  "
#define  FUEL_AMT_UNIT_LCD_LINE							"Ltr"
#define	 ODO_READING_UNIT_LCD_LINE						"Km/Hr"
#define  FUEL_AMT_PRICE_UNIT_LCD_LINE					"Rs."
#define  FAILSAFE_LINE1									" Loading Defaults "
#define  FAILSAFE_LINE2									" Press Any Key to "
#define  FAILSAFE_LINE3									"     Continue     "
#define  DISPLAY_LOGS_TRANS_LCD_LINE					"TRANS #: "		/ *Anand 07-07-2014* /
#define  DISPLAY_LOGS_MAIN_LCD_LINE1					"1. SUMMARY"
#define  DISPLAY_LOGS_MAIN_LCD_LINE2					"2. DISPENSE LOGS"
#define  DISPLAY_LOGS_MAIN_LCD_LINE3					"3. REFUEL LOGS"
#define  DISPLAY_LOGS_MAIN_LCD_LINE4					"Press Respective KEY"
#define  DISPLAY_LOGS_DISPENSE_LCD_LINE1				"DISPENSE LOGS"
#define  DISPLAY_LOGS_DISPENSE_LCD_LINE2				"Press ENTER Key to"
#define  DISPLAY_LOGS_DISPENSE_LCD_LINE3				"CONTINUE"
#define  DISPLAY_LOGS_DISPENSE_NO_LOGS_LCD_LINE1		"  No Dispenses Yet! "		//Anand 07-07-15
#define  DISPLAY_LOGS_DISPENSE_NO_LOGS_LCD_LINE2		"Press Summary/ESC to"		//Anand 07-07-15
#define  DISPLAY_LOGS_DISPENSE_NO_LOGS_LCD_LINE3		"      GO Back       "		//Anand 07-07-15
#define	 DISPLAY_LOGS_REFUEL_LCD_LINE1					"REFUEL LOGS"
#define  DISPLAY_LOGS_REFUEL_LCD_LINE2					"Press ENTER Key to"
#define  DISPLAY_LOGS_REFUEL_LCD_LINE3					"CONTINUE"
#define  DISPLAY_LOGS_REFUEL_NO_LOGS_LCD_LINE1			"   No Refuels Yet!  "		//Anand 07-07-15
#define  DISPLAY_LOGS_REFUEL_NO_LOGS_LCD_LINE2			"Press Summary/ESC to"		//Anand 07-07-15
#define  DISPLAY_LOGS_REFUEL_NO_LOGS_LCD_LINE3			"      GO Back       "		//Anand 07-07-15
#define  DISPLAY_LOGS_END_LCD_LINE						"     END OF LOG     "		/ *Anand 2.05.2014* /
#define  DISPLAY_LOGS_REFUEL_LCD_LINE4					"Fuel Amt.:"
#define  DISPLAY_LOGS_SUMMARY_LCD_LINE1					"DISPENSER SUMMARY"
#define  DISPLAY_LOGS_SUMMARY_LCD_LINE2					"Curr.  : "
#define  DISPLAY_LOGS_SUMMARY_LCD_LINE3					"Dispd. : "
#define  DISPLAY_LOGS_SUMMARY_LCD_LINE4					"To be Synced : "//"Dispenser No. : "
#define  POWER_KEY_LCD_LINE1							" Continue Shutdown? "
#define  POWER_KEY_LCD_LINE2							"YES(ENTER)   NO(ESC)"
#define  POWER_KEY_LCD_LINE3							"   SHUTTING DOWN    "
#define  POWER_KEY_LCD_LINE4							"      COMPLETE      "
#define  POWER_CUTOFF_LCD_LINE							" EMERGENCY POWER OFF"
#define  CALLIBRATION_MODE_LCD_LINE1					"  CALIBRATION  MODE "
#define  CALLIBRATION_MODE_LCD_LINE2					"   ENTER  PASSWORD  "
#define  CALLIBRATION_MODE_LCD_LINE3					"  ENTER FUEL IN AMT "
#define  CALLIBRATION_MODE_LCD_LINE4					"FILL "
#define  CALLIBRATION_MODE_LCD_LINE5					" Ltr. FUEL"
#define  CALLIBRATION_MODE_LCD_LINE6					"   AND PRESS ENTER  "
#define  CALLIBRATION_MODE_LCD_LINE7					"TOTAL FUEL : "
#define  CALLIBRATION_MODE_LCD_LINE8					"  CALCULATING....   "
//#define  BOWSER_STATUS_LCD_LINE1						"DISPENSER No : "
#define  BOWSER_STATUS_LCD_LINE1						"SYS STS : "
#define  BOWSER_STATUS_LCD_LINE2						"CURR. AMT. : "
#define  BOWSER_STATUS_LCD_LINE3						" PRESS DISPENSE KEY "
#define  SEND_UNSENT_MSGS_LCD_LINE1						"   SENDING UNSENT   "
#define  SEND_UNSENT_MSGS_LCD_LINE2						" TRANSACTION MSG(S) "
#define  SEND_UNSENT_MSGS_LCD_LINE3						"     PLEASE WAIT...."
#define  INVALID_QTY_LCD_LINE							" INVALID QUANTITY ! "		//Change by HJ 12-01-2017
#define  OPERATOR_TAG_MESSAGE							"SWIPE OPERATOR CARD "
#define  BATTERY_PERC_LCD_LINE							"NOZZLE BATT.: 		 "
#define  MEDIUM_BATTERY_LCD_LINE						"RECHARGE NOZZLE BATT"
#define  CRITICAL_BATTERY_LCD_LINE						"NOZZLE BATT CRITICAL"
#define  DATE_SEPERATOR									'/'
#define  TIME_SEPERATOR									':'
#define  DISPLAY_CONFIG_LCD_AUTH						" ENTER THE PASSWORD "	
#define  DISPLAY_CONFIG_LCD_LINE1						"1. CONFIGRATION"							//HJ 27-01-2016
#define  DISPLAY_CONFIG_LCD_LINE2						"2. CLEAR DATA"
#define  DISPLAY_CONFIG_LCD_LINE3						"3. CHANGE PASSWORD"
#define  DISPLAY_CONFIG_LCD_CONFIG_LINE1				"1. SET PRICE"
#define  DISPLAY_CONFIG_LCD_CONFIG_LINE2				"2. CAL. CONSTANT"
#define  DISPLAY_CONFIG_LCD_CONFIG_LINE3				"3. SET RTC"
#define  DISPLAY_CONFIG_LCD_DATA_LINE1					"1. CLEAR LOGS"
#define  DISPLAY_CONFIG_LCD_DATA_LINE2					"2. RESET TANK"
#define  DISPLAY_CONFIG_LCD_CHNG_PASS_LINE				" ENTER NEW PASSWORD "
//HJ 16-02-2016
#define  DISPLAY_CONFIG_LCD_CURR_RATE_LINE				"CURR. RATE:"
#define  DISPLAY_CONFIG_LCD_NEW_RATE_LINE				"NEW RATE:"
#define  DISPLAY_CONFIG_LCD_ENTER_NEW_RATE_LINE			"   ENTER NEW RATE   "
#define  DISPLAY_CONFIG_LCD_CURR_CONSTANT_LINE			"CURR. CONSTANT:"
#define  DISPLAY_CONFIG_LCD_NEW_CONSTANT_LINE			"NEW CONSTANT:"
#define  DISPLAY_CONFIG_LCD_ENTER_NEW_CONSTANT_LINE		" ENTER NEW CONSTANT "
#define  DISPLAY_CONFIG_LCD_CLEAR_LOGS_LINE				"   CLEAR ALL LOGS   "
#define  DISPLAY_CONFIG_LCD_CLEAR_LEVEL_LINE			"  CLEAR TANK LEVEL  "
#define  DISPLAY_CONFIG_LCD_CNFRM_PASS_LINE				"  CONFIRM PASSWORD  "
#define  DISPLAY_CONFIG_LCD_PWSD_LEN_LINE				"  ENTER FOUR DIGIT  "

#define GLCD_SUMMARY_HEADER_1							"SYSTEM LOGS"
#define GLCD_SUMMARY_HEADER_2							"1 SUMMARY"
#define GLCD_SUMMARY_HEADER_3							"2 DISPENSE LOGS"
#define GLCD_SUMMARY_HEADER_4							"3 REFUEL LOGS"*/
/*

#define GLCD_SUMMARY_SCREEN_LINE_1						BOWSER_STATUS_LCD_LINE1
#define	GLCD_SUMMARY_SCREEN_LINE_2						BOWSER_STATUS_LCD_LINE2//Available fuel*/

/*
#define	GLCD_DISP_LOG_LINE_1							"Trans #:"//Transaction id
#define	GLCD_DISP_LOG_LINE_2							"Vehi  #: "//Vehicle reg. no.
#define GLCD_DISP_LOG_LINE_3							"Odo Mtr: "//odometer reading
#define GLCD_DISP_LOG_LINE_4							"Litres : "//fuel Amount
#define GLCD_DISP_LOG_LINE_5							"Date   : "//date
#define GLCD_DISP_LOG_LINE_6							"Time   : "//time

#define GLCD_REFUEL_LOG_LINE_1							GLCD_DISP_LOG_LINE_1//Transaction id
#define GLCD_REFUEL_LOG_LINE_2							"Litres : "//fuel amount
#define GLCD_REFUEL_LOG_LINE_3							"Date   : "//data
#define GLCD_REFUEL_LOG_LINE_4							"Time   : "//time

#define USR_MSG_LOGS_MENU_ARROW_ESC						" PRESS ESC or ARROW "
*/


/*
//INPUT MESSAGES
#define  FUEL_AMOUNT_UPDATE_LCD_LINE1		    		"CALCULATING  CURRENT"
#define  FUEL_AMOUNT_UPDATE_LCD_LINE2		    		"  AVL. FUEL AMOUNT  "
#define  FUEL_AMOUNT_UPDATE_LCD_LINE3		    		"   PLEASE WAIT....  "
#define  ENTER_TRUCK_REGI_NUM_LCD_LINE					" VEHICLE REGTD. NO. "
#define	 ENTER_FUEL_AMT_LCD_LINE						"      QUANTITY      "
#define	 ENTER_FUEL_AMT_PRICE_LCD_LINE					"       AMOUNT       "
#define	 ENTER_ODO_READ_LCD_LINE1						"  ODOMETER READING  "
#define	 PRESS_ENTER_LCD_LINE						    "  PRESS ENTER KEY   "

//For Printing

#define  PRINTING_LCD_LINE1								"Do You Want To Print"	
#define  PRINTING_LCD_LINE2								"YES(ENTER)   NO(ESC)"
#define  PRINTING_START_LCD_LINE						"   PRINTING START   "
#define  AFTER_PRINTING_LCD_LINE						" PRESS DISPENSE KEY "
#define	 NUM_OF_RECEIT_LCD_LINE1						"Enter Num of Receit"
#define	 NUM_OF_RECEIT_LCD_LINE2						"          1         "

#define	 MAX_RECEIT_CHOICE_LCD_LINE1					"   PLEASE RE-ENTER  "

// For DAignostic Messages
#define MAIN_BAT_CRITICAL_MSG							" MAIN BATT CRITICAL "
#define MAIN_BAT_CRITICAL_ACTION						"   REPLACE BATTERY  "
#define MAIN_BAT_LOW_MSG								"  MAIN BATTERY LOW  "
#define MAIN_BAT_LOW_ACTION								"   CHARGE BATTERY   "
#define BACKUP_BAT_LOW_MSG								"   BACKUP BATT LOW  "
#define BACKUP_BAT_LOW_ACTION							" REPLACE BACKUP BATT"
#define RTC_FAULT_MSG									"   DATE/TIME ERROR  "
#define RTC_FAULT_ACTION								"SET SYSTEM DATE/TIME"

#define DISPLAY_ERR_DIAG_LCD_LINE1						"ERR : "
#define DISPLAY_NO_ERR_DIAG_LCD							"      SYSTEM OK     "

#define PUMP_COOLING_OFF_MSG_LINE1						"  PUMP COOLING OFF  "
#define PUMP_COOLING_OFF_MSG_LINE2						"     PLEASE WAIT    "*/

//IDLE DISPLAY SCREEN
//#define  IDLE_DISP_LCD_LINE1							"  Idle Display  "
//#define  IDLE_DISP_LCD_LINE2							"     Screen     "

//REFUELLING DISPLAY SCREEN

//#define  REFUEL_ENTER_LCD_LINE1							"  Refuel Mode   "
//#define  REFUEL_WAIT_LCD_LINE2							"  Waiting......."
//#define  REFUEL_PUMP_CODE_LCD_LINE1						"Enter Pump Code "
//#define  REFUEL_PUMP_PASS_LCD_LINE1						"Pump Code:"

//DISPENSE DISPLAY SCREEN
//#define  DISPENSE_ENTER_LCD_LINE1						"Entering  Dispense"
//#define  DISPENSE_ENTER_LCD_LINE2						"        Mode     "
//#define  DISPENSE_SCAN_RFID_LCD_LINE1					"Please authenticate"
//#define  DISPENSE_RFID_WAIT_LCD_LINE2					"Waiting for Scan"
//#define  DISPENSE_MODE_LCD_LINE1						"Select Mode "
//#define  DISPENSE_MODE_LCD_LINE2						"1: MANUAL"				 //dont change number
//#define  DISPENSE_MODE_LCD_LINE3						"2: AUTO  "				 //dont change number
//#define  DISPENSE_ODO_READING_LCD_LINE1					"Please enter "
//#define  DISPENSE_ODO_READING_LCD_LINE					"Odometer Reading"
//#define  DISPENSE_ODO_PASS_LCD_LINE2					"Enter Success"
//#define  DISPENSE_FUEL_OUT_LCD_LINE1					"Dispensing Fuel"



//CONFIG MODE DISPLAY SCREEN

//#define  CONFIG_ENTER_LCD_LINE1							"Entering    Config."
//#define  CONFIG_ENTER_LCD_LINE2							"        Mode       "
//#define  CONFIG_PASSWD_LCD_LINE1						"  Enter Password  "
//#define  CONFIG_PASS_CHANGE_LCD_LINE1					"1: Change Password"    //dont change number
//#define  CONFIG_NEW_LCD_LINE2							"2: New RFID Entry "	//dont change number
//#define  CONFIG_UPLOAD_LCD_LINE3						"3: Upload Data    "	//dont change number
//#define  CONFIG_NEXT_LCD_LINE4							"4: Next menu "			//dont change number
//#define  CONFIG_CLEAR_LCD_LINE1							"5: Clear Master Data"	//dont change number
//#define  CONFIG_DATE_LCD_LINE2							"6: Set Date "			//dont change number
//#define  CONFIG_TIME_LCD_LINE3							"7: Set Time "			//dont change number
//#define  CONFIG_BACK_LCD_LINE4							"8: Previous Menu"		//dont change number
//#define  CONFIG_NEWPASSWD_LCD_LINE1						"Enter New  password  "
//#define  CONFIG_PASS_CHANGED_LCD_LINE1					" Password Changed!! "
//#define  CONFIG_SCAN_RFID_LCD_LINE1						"Scan New RFID Nozzle"
//#define  CONFIG_RFID_WAIT_LCD_LINE2						"Waiting for Scan"
//#define  CONFIG_SETDATE_LCD_LINE1						"DD/MM/YY/Day Format"
//#define  CONFIG_SETDATE_LCD_LINE2						"SUN=1,MON=2....SAT=7"
//#define  CONFIG_DATEPASS_LCD_LINE1						"Date change Success"
//#define  CONFIG_SETTIME_LCD_LINE1						"HH:MM:SS:AM/PM"
//#define  CONFIG_SETTIME_LCD_LINE2						"AM=0 , PM=1"
//#define  CONFIG_TIMEPASS_LCD_LINE1						"Time change Success"


//SERIAL RESPONSE LCD DISPLAY

//#define  FUEL_LIMIT_CHANGE_LINE3						"New fuel Disp.limit:"
//#define  NEW_TRUCK_ID_ADD_LINE3							"New ID Added:       "
//#define  DEL_TRUCK_ID_LINE3								"ID Deleted:         "
//#define  TRUCK_CONFIG_CLEAR_LINE4						"Truck Data Cleared  "


//ERROR MESSAGES		//Anand 1.04.2014

/*
#define ERR_NOT_ENOUGH_FUEL_LCD_LINE						"   NOT ENOUGH FUEL  "
#define ERR_MAX_LIMIT_LCD_LINE								" MAX LIMIT EXCEEDED "
#define ERR_INVALID_TRUCK_REGI_NUM							"INVALID TRUCK NUMBER"		//Anand 18.04.2014
#define ERR_INVALID_PASS_LCD_LINE							"  INVALID PASSWORD  "
#define ERR_INVALID_TRUCK_ID_LCD_LINE						"  INVALID TRUCK ID  "		//Anand 29-9-15
#define ERR_HOLD_NOZZLE_STILL_LCD_LINE						" HOLD NOZZLE STILL  "		//Anand 29-9-15
#define ERR_INVALID_ENTRY_LCD_LINE							"    INVALID ENTRY   "
#define ERR_TRUCK_ID_MISMATCH_LCD_LINE						"TRUCK ID MISMATCHED "
#define ERR_FLASH_FLUSH_LCD_LINE							"   FLUSHING FLASH   "
#define ERR_FLASH_DR_MISMATCH_LCD_LINE						"    DR MISMATCH     "
#define ERR_FLASH_RR_MISMATCH_LCD_LINE						"    RR MISMATCH     "
#define ERR_E2P_DATA_MISMATCH_LCD_LINE						"  E2P DATA MISMATCH "
#define ERR_E2P_PRODUCT_SERIAL_NUMBER_INVALID_LCD_LINE		" INVALID PRODUCT ID "
#define ERR_CALLIBRATION_DATA_MISMATCH_LCD_LINE				" CALIBRATION NEEDED "
#define ERR_ADC_READ_LCD_LINE								"   ADC READ ERROR   "
#define ERR_GSM_INIT_LCD_LINE								"   GSM INIT ERROR   "
#define ERR_SYS_HANG_LCD_LINE								"    SYSTEM HANG     "
#define ERR_SYS_HANG_LCD_LINE_FDM_ON						"    SYS HANG : ON   "
#define ERR_SYS_HANG_LCD_LINE_FDM_CURR_FUEL_AMT_UPDATE		" SYS HANG : LVL E2P "
#define ERR_SYS_HANG_LCD_LINE_FDM_TRUCK_REGI_NUM			" SYS HANG : REGI NUM"
#define ERR_SYS_HANG_LCD_LINE_FDM_AUTHENTICATE				" SYS HANG : AUTHENT "
#define ERR_SYS_HANG_LCD_LINE_FDM_DISPENSE_FUEL				" SYS HANG : DISPENSE"
#define ERR_SYS_HANG_LCD_LINE_FDM_CONFIG					" SYS HANG : CONFIG  "
#define ERR_SYS_HANG_LCD_LINE_FDM_CALLIBRATE_LEVEL_SENSOR	" SYS HANG : CALIBRAT"
#define ERR_SYS_HANG_LCD_LINE_FDM_DISPLAY_LOG				" SYS HANG : DISP LOG"


//
#define EMPTY_STRING										"                    "
#define SYSTEM_REBOOT_LCD_LINE							    "SYSTEM REBOOTING"

#define  WIFI_UPLOADING								"WiFi Uploading"

#define RTC_FAILURE_STRING							"      RTC FAIL      "*/


////////////////////////////////////////////////////////////////////////////////////
// TT 11 Sep 2017
// Tank Monitoring

//LCD DISPLAY MESSAGES
//enter not more than 20 char in a line

//POWER ON DISPLAY SCREEN
#define  POWER_ON_LCD_LINE1								"       \"FlinK\"        "
#define  POWER_ON_LCD_LINE2								"     E4ENGINEER     "
#define  POWER_ON_LCD_LINE3								FIRMWARE_VER

#define WELCOME_STRING								"      Hi Tiya       "

const char hex_file_init_update_line1[] PROGMEM =	"    CHECKING    ";
const char hex_file_update_line2[] PROGMEM =		"   DOWNLOADING  ";
const char hex_file_pending_line[] PROGMEM =		"PENDING DOWNLOAD";
const char hex_file_resume_line[] PROGMEM =			"1. RESUME       ";
const char hex_file_restart_line[] PROGMEM =		"2. RESTART      ";
const char cnfrm_dwnload_line1[] PROGMEM =			"DOWNLOAD UPDATE?";
const char no_update_line[] PROGMEM =				"   NO  UPDATE   ";
const char download_complete_line[] PROGMEM =		" DOWNLOAD  DONE ";
const char file_not_found_line[] PROGMEM =			" FILE NOT FOUND ";
const char conn_error_line[] PROGMEM =				"CONNECTION ERROR";
const char conn_verify_line[] PROGMEM =				"    VERIFYING   ";
