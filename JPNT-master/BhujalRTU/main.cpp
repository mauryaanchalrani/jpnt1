#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "schedular.h"
#include "timer.h"
#include "uart.h"
#include "app_uart.h"
#include "main.h"
#include "graphic_lcd_ks0108.h"

#include "language.h"
#include "ModbusRtu.h"
#include "common.h"
#include "app_eeprom.h"
/*
#include "I2C.h"
#include "ds1307.h"*/
#include "clock.h"
#include "system_sts.h"
#include "web_comm.h"

#include "flash.h"
#include "error.h"
#include "adc.h"		//Anand 02.05.2014

//#include "display.h"
#include "sys_info.h"
#include "RS_485_device.h"
#include "ext_interrupt.h"
#ifdef GPRS_ENABLE
#include "gprs.h"
#endif

//#include "calibration.h"

#include "always.h"
// #include "pulser.h"
#include "sensor.h"
#include "panel.h"
#include "modbus_addr.h"
//#include "graphic_lcd_ks0108.h"
#include "app_lcd.h"
#include "gprs_config.h"
#include "flow_meter.h"
#include "_debug.h"
#include "pulser.h"
#include <avr/wdt.h>
#include "WDT.h"
#include "daily_limit.h"
#include "Time_Diff.h"

extern unsigned char color;

extern volatile schedular_flg_t schedular_flg;
extern volatile Rx_Buff_t Rx_Buff[3];
extern volatile Tx_Buff_t Tx_Buff[3];
//volatile gprs_temp_rx_buff_t gprs_temp_rx_buff;
extern gprs_rx_isr_handler_t gprs_rx_isr_handler;
rtu_control_data_t rtu_control_data;
extern timeDiff_t timeDiff;
extern gprs_date_time_t gprs_date_time;
sensor_data_t sensor_data;
ram_data_t ram_data;
FL_rtu_log_t FL_rtu_log;		//should be local vaiable
FL_rtu_log_t last_record;			//24-07-21 AM
extern time_main_t time_main;
WEB_data_u WEB_data;

extern RS485_device RS485_device_handler;

//rtu_config_t rtu_config;

freq_updated_data_t freq_updated_data;

//sim_info_t sim_info;

//extern sys_info_t sys_info;
extern product_info_t product_info;		//HJ 0-08-2016
#ifdef ONLINE_CONFIG_EN
extern product_info_config_t product_info_config;
#endif
//extern gsm_cfg_data_t gsm_cfg_data;
extern gprs_config_data_t gprs_config_data;

extern volatile unsigned long int pulser_raw_cnt;
//extern fm_data_t fm_data;
extern calbration_data_t calbration_data;
//extern mylcd mylcd_obj,mylcd_obj2;
extern mbtcp_payload_data_t  mbtcp_payload_data;   // 03/10/2018
//extern unsigned int tcp_data_send_status ;
extern gprs_t gprs; // VC:08/10/2018
extern gprs_conn_status _gprs_conn_status; // 24/10/2018

unsigned int day_change_status = FALSE;
unsigned int month_change_status = FALSE;
unsigned int db_save_status = TRUE;
unsigned char sms_alert_state = CHECK_ALERT;

em_data_t em_data; // 20-11-2018 VC: taking for save the old kwh and kwah
extern gprs_rx_data_buff_t uart_to_gprs_temp_buff;
RTU_state_t RTU_state = RTU_NORMAL;				// 21-2-19 VC:

extern volatile unsigned int curr_pulse_cnt;	// 22-2-19 VC:
//extern sim_info_t sim_info;						// 22-2-19 VC:

static manage_frmwr_updt_t manage_frmwr_updt;
extern flash_hexfile_t flash_hexfile;
rtu_data_t rtu_data;

extern flow_meter_t flow_meter;
alarm_state_t alarm_state = ALARM_OFF;
extern e2p_log_data_t e2p_log_data;

extern e2p_log_y_tot_t e2p_log_y_tot;	//abdul majeed
unsigned long int yest_total;

#ifdef ONLINE_CONFIG_EN
extern online_cfg_t online_cfg;
extern e2p_online_cnfg_t e2p_online_cnfg;
#endif

rtc_state_t rtc_state = IDLE;//AM

int main(void)
{
	WDT_off();
   //unsigned char timebuff[] = {19, 03, 05, 01, 11, 51, 00 };
	static unsigned int rtu_upload_time = 0, rtu_save_logs_time = 0, db_save_time = 0/*, schedular_30ms_cnt = 0*/,schedular_5s_cnt = 0;
	//ram_data.pump_data.system_status = 0; // 22/09/2018 VC: initial zero for noting any error.
   
   
   
	_delay_ms(100);		//Imp delay to avoid self restart after shutdown
	
	gpioInit();
//#if 0
	GLCD_Initalize();			//25-02-19 SK: Initialize graphics lcd 
	_delay_ms(100);		//Lets Settle down a bit
	 GLCD_ClearScreen();
	initUart(UART_PC);
	UWriteString("** RTU **\n\r", UART_PC);
	UWriteString(FIRMWARE_VER,UART_PC);
	bootingUp();

	/*UWriteString("** RTU **\n\r", UART_PC);*/
	#ifdef _DEBUG_LCD
	UWriteString("** RTU **\n\r", UART_PC);
	#endif

	//#define NEW_BOARD
#ifdef NEW_BOARD
	factory_defaults();
	UWriteString("Factory Default Done.",UART_PC);   
#endif

	
	
	//set_rtc(timebuff);
	
	

   getAllCfgData();				// 22-2-19 VC: to do here for this project
#ifdef ONLINE_CONFIG_EN
#ifdef DEBUG_ONLINE_CFG
	UWriteString("curr_ver_cnt:",UART_PC);
	UWriteInt(get_online_cfg_ver_cnt(),UART_PC);
#endif
#endif
   setGPRSConnSts(NOT_AVBL);
   setGPRSSrvrSts(NOT_AVBL);
   
#ifdef DEBUG_UNSENT_LOGS
   UWriteString("unsent:",UART_PC);
   UWriteInt(freq_updated_data.unsent_rtu_logs_GPRS,UART_PC);
#endif

   _delay_ms(1000);
   init_rtu_ctrl_data(); // taking for keep alive second rtu
  

	welcome_screen();
	_delay_ms(2000);
	GLCD_ClearScreen();
	
	//Lets get date and time in Ram data. This is mendatory for day change and month change logic	
	get_present_time(&ram_data.time);
	ram_data.status |= (1 << POWER_ON_BIT);
	//This should be done afer time read
	get_last_rtu_log();
//#endif
	//initUart(UART_PC);
	//initUart(UART_485);
#ifdef DEBUG_SIMULATED_DATA
	//set_dummy_values();
#endif
	_delay_ms(1000);
	WDT_Init();
	 sei(); //:VC :14/07/2018 need to be enable
   while(true)
   {
		wdt_reset();
//#if 0


	  // _delay(10);
	   //readButton();
	   /*check_for_power_down();*/
	   
	   //schedule_without_timer();
	   //Free Running Task for checking data in request
	   checkforUARTFrameTimeout(UART_485);		// 30 ms frame timeout
	   checkforUARTFrameTimeout(GPRS_UART);	// 30 ms frame timeout

	   //checkforpendingdata();
	   
	   if(schedular_flg.flg_10ms == TRUE)
	   {
		   schedular_flg.flg_10ms = FALSE;		   
		   check_for_power_down();
		   check_for_MOTOR_POWER();
	   }
	   if(schedular_flg.flg_50ms == TRUE)
	   {
		   schedular_flg.flg_50ms = FALSE;
		   sw_led_cntrl();
			   /*manage_gps_gprs();*/
	   }	   
	   // ----- Perform 100ms Tasks -----
	   if (schedular_flg.flg_100ms == TRUE)
	   {
		   schedular_flg.flg_100ms = FALSE;
		if(getRtuState() != RTU_UART_CONFIG)
		{
			manage_gps_gprs();
		}
		   decodeMsgUart0();
#if FM_TYPE == ABB_UART
		   //readFMSensor();
#endif
		   controlRTU();
		   
		}
	   
	   // * ----- Perform 1sec Tasks ----- * /
		if (schedular_flg.flg_1sec == TRUE)
		{
			schedular_flg.flg_1sec = FALSE;
			ACTIVITY_LED_PORT ^= (1<<ACTIVITY_LED);
			if((getRtuState() != RTU_FRMWR_UPDATE))
			{
#if FM_TYPE == KROHNE_MAR_50_485
				//RS485_device_handler.slave_handler();
#endif
			}
			/*updateRamData();*/
			if(getRtuState() == RTU_NORMAL) 
			{
				readFMSensorADC();
				updateRamData();
				data_screen();							//26-02-2019 SK
				check_sms_threshold();					//AM
#ifdef DEBUG_PULSER
				UWriteString("pulse:",UART_PC);
				UWriteInt(getPulserCnt(),UART_PC);
				clrPulserCnt();
#endif
			}
			
			
			checkForAlert();		//04-01-21 SK: this is empty function
			//updateLocalDesplay();					//23-2-19 SK: this is to be changed according to lcd library.
			readButton();
			//Store in flash in every 60sec
			
		   if(++schedular_5s_cnt >= 5)
		   {
				schedular_5s_cnt = 0;
		   }
		   if((getRtuState() != RTU_FRMWR_UPDATE))
		   {
			   if( ++rtu_save_logs_time >= RTU_SAVE_LOG_TIME  ) // save data is time is 60 sec
			   {
				   rtu_save_logs_time = 0;
#ifdef FLASH_LOG_DATA
				   prepare_FL_LogData();
				   save_FL_rtu_data();
#else
					prepare_e2p_logdata();
					e2p_write_log_data();
#endif
				   //if((getGPRSSrvrSts() == NOT_AVBL)) // if gprs is not avilable then save the ram data into flash
				   if(getGPRSConnSts() == NOT_AVBL	||	check_unsent_log())
				   {
#ifdef DEBUG_ONLINE_DATA
UWriteString("upload_back:",UART_PC);
#endif
					   ++freq_updated_data.unsent_rtu_logs_GPRS;
					   flashWriteFreqUpdatedConfig();
					   // 21/09/2018 need to change to upload on flash
				   }
			   }
			   
			   //if( ++rtu_upload_time >= RTU_UPLOAD_TIME  ) // upload data in every 60 sec
			   if(	++rtu_upload_time >= RTU_UPLOAD_TIME	
			   #ifdef SUPERCAP
			    ||	check_mains_cut()	
				#endif
				)
			   {				
 #ifdef SUPERCAP
					if(check_mains_cut())
					{
						//save_mains_off_logs();//already handled in check_for_power_down() 10ms
						db_save_status = TRUE;
					}
					else
#endif
					{
					   rtu_upload_time = 0;
#ifdef DEBUG_ONLINE_DATA
UWriteString("upload_online:",UART_PC);
#endif
					   if(++db_save_time >= DB_SAVE_TIME)
					   {
						   db_save_time = 0;
						   db_save_status = TRUE;
					   }
					   else//if(db_save_time == (product_info.industry_id % (DB_SAVE_TIME)))
					   {
						  // db_save_status = FALSE;
					   }
					}
				   
				   update_online_data();
			   }
			   online_firmware_download();
			   /*if(++db_save_time >= DB_SAVE_TIME)
			   {
				   db_save_time = 0;
			   }
			   if(db_save_time == (product_info.industry_id % (DB_SAVE_TIME)))
			   {
				   db_save_status = TRUE;
			   }*/
		   }
		   
	   } // END OF 1 SEC SCHEDULAR
	   
	   if((Tx_Buff[UART_PC].pending_command))
	   {

		   
		  /* #ifdef DEBUG_TIME
		   UWriteString("test_cnt:",UART_PC);
		   UWriteInt(test_cnt++,UART_PC);
		   #endif*/
		   completePendingCommand();
		   Tx_Buff[UART_PC].curr_pck_num++;
		   
	   }
	   
	   if(Tx_Buff[UART_PC].tx_ready == TRUE)
	   {
		   Tx_Buff[UART_PC].tx_ready = FALSE;
		   
		   Usendbuffer(UART_PC);		//Send data

		   flushTxBuffer(UART_PC);	//Flush buffer
	   }
	   if(Tx_Buff[UART_485].tx_ready == TRUE)
	   {
		   Tx_Buff[UART_485].tx_ready = FALSE;
		   Usendbuffer(UART_485);		//Send data
#ifdef DEBUG_FM
UWriteString((char *)"\nfm_tx:",UART_PC);
UWriteBytes((char *)&Tx_Buff[UART_485].tx_buffer,(unsigned int)Tx_Buff[UART_485].tx_indx,UART_PC);
#endif
		   flushTxBuffer(UART_485);		//Flush buffer
	   }
//#endif	   
   }//while end

	return 0;
}

void gpioInit (void)
{
	// SET I/O HERE
	DDRA = DDRB = DDRC = DDRD = DDRE = DDRF = DDRG = DDRH = DDRK = 0x00;

	//DDRA &= (~((1<<SWITCHING_PIN) | (1<<PA1) | (1<<PA2) | (1<<PA3)));		//Remaining are LCD Data pins which are directed in respective Functions.
	/*LTC_SS_DDR |= (1<<LTC_SS);
	E2P_1_WP_DDR |= (1<<E2P_1_WP);
	E2P_2_WP_DDR |= (1<<E2P_2_WP);*/
	//POWER_CUTOFF_DDR &= ~(1<<POWER_CUTOFF);		//Added by KP 5-2-2015
	
	SYS_ERR_LED_DDR |= (1<<SYS_ERR_LED);
	SYS_ERR_LED_PORT |= (1<<SYS_ERR_LED);
		
	FLASH_CNTRL_DDR |= (1<<CE1) /*| (1<<WP1)*/;
	
	ACTIVITY_LED_DDR |= (1<<ACTIVITY_LED);
	
	SYS_ERR_LED_PORT &= (~(1<<SYS_ERR_LED));//|(1<<SYS_ERR)));
	ACTIVITY_LED_PORT &= (~(1<<ACTIVITY_LED));
	GENERAL_IO_DDR = 0xF0;

	// for modbus RTU pins setup 04/10/2018 :VC
	MODBUS_INPUT_DDR = 0x00;
	MODBUS_OUTPUT_DDR = 0x3C;
	// modbus RTU io pins
	DDRK = 0x00;
	DDRH = 0xFF;
	
	CFG_SW_DDR &= ~(1<<CONFIG_SW);
	
	SW_LED_DDR &= ~((1 << SW_LED_SW1) | (1 << SW_LED_SW2) | (1 << SW_LED_SW3));
	POWER_DOWN_DDR &= ~(1 << POWER_DOWN);
	/*MODBUS_DDR |= (1 << MOD_TXEN_PIN);
	MODBUS_PORT &= ~(1 << MOD_TXEN_PIN);*/
	MOTOR_DDR |= (1 << MOTOR);
	MOTOR_PORT |= (1 << MOTOR);
	ALARM_DDR |= (1 << ALARM);
	
	SNOOZE_SW_DDR &= ~(1 << SNOOZE_SW);
	MOTOR_AC_DDR &= ~(1 << MOTOR_AC);//AM 19-07-21

	
	/*DDRL &= ~(1 << PL7);*/
	/*PORTL |= (1 << PL7);*/
	//GPRS_STATUS_DDR &= ~(1 << GPRS_STATS);
	GPRS_PWRKEY_DDR |= (1 << GPRS_PWRKEY_PIN);
	gprsPowerUpInit();
}
void gprsPowerUpInit()
{
	/*GPRS_RESET_DDR	|= (1<<PE2);
	
	GPRS_RESET_PORT |= (1<<PE2);*/
	//DDRG |= (1 << PG0);
	/*GPRS_PWRKEY_PORT &= ~(1 << GPRS_PWRKEY_PIN);
	_delay_ms(2000);*/
	GPRS_PWRKEY_PORT |= (1<<GPRS_PWRKEY_PIN);
	/*PORTG &= ~(1 << PG0);
	_delay_ms(2000);
	PORTG |= (1 << PG0);*/
}

void prepare_FL_LogData(void)
{
	//FL_log_data.transaction_id_lvl_log;			//2
	//FL_log_data.transaction_id_MSB_lvl_log;		//1
	//get_present_time(&ram_data.time);
	//get_location();
	memcpy(&FL_rtu_log.ram_data,&ram_data,sizeof(ram_data_t));
	
	FL_rtu_log.chksum = 0;		//This is caculated in nxt fun
}

void prepare_e2p_logdata(void)
{
	e2p_log_data.t_total = ram_data.todays_total;
	e2p_log_data.m_total = ram_data.months_total;
	e2p_log_data.c_total = ram_data.cuml_reading;
	e2p_log_data.date = ram_data.time.dd;
	e2p_log_data.month = ram_data.time.mm;
	e2p_log_data.checksum = 0;
}

void prepare_e2p_y_total(void)	//abdul majeed
{
	//e2p_log_y_tot.y_total = yest_total;
	memcpy(&e2p_log_y_tot.y_total,&yest_total,sizeof(e2p_log_y_tot_t));
}

void prepare_GPRS_LogData(void)
{
#if 0
	static long cum_cnt = 6556;
	
//ram_data.pump_data.system_status = 50;


//ram_data.pump_data.inst_flow = 3555 /*+ (++flw_cnt)*/;

//ram_data.pump_data.cum_flow = (cum_cnt + 50)  ;
//ram_data.pump_data.pressure_in = 5222 /*+ (++pre_in)*/;

//ram_data.pump_data.pressure_out = 4900;

/*
ram_data.EM_data.V_R = 230;

ram_data.EM_data.V_R_dp = 8;

ram_data.EM_data.V_Y = 240;

ram_data.EM_data.V_Y_dp = 6;

ram_data.EM_data.V_B = 250;

ram_data.EM_data.V_B_dp = 60;
ram_data.EM_data.I_R = 7;
ram_data.EM_data.I_R_DP = 4;
ram_data.EM_data.I_Y =5;
ram_data.EM_data.I_Y_DP = 50;
ram_data.EM_data.I_B = 8;
ram_data.EM_data.I_B_DP = 60;
ram_data.EM_data.PF_R = 1;
ram_data.EM_data.PF_R_dp = 70;
ram_data.EM_data.PF_Y = 0;
ram_data.EM_data.PF_Y_dp = 90;
ram_data.EM_data.PF_B = 1;
ram_data.EM_data.PF_B_dp = 90;
ram_data.EM_data.PF_AVG  = 1;
ram_data.EM_data.PF_AVG_dp = 10;

ram_data.EM_data.KWh = 176;

ram_data.EM_data.KWh_dp = 11;
ram_data.EM_data.KVah = 185;

ram_data.EM_data.KVah_dp = 12;
*/

 /*ram_data.time.yr = 119;

 ram_data.time.mm = 120;

 ram_data.time.dd = 121;

 ram_data.time.hr = 122;

 ram_data.time.min = 123;

 ram_data.time.sec = 124;*/

#endif
 #ifdef DEBUG_ONLINE_DATA
 UWriteString("prep_:",UART_PC);
 #endif
	memset(&WEB_data.GPRS_rtu_data, 0, sizeof(GPRS_rtu_data_t));
	WEB_data.GPRS_rtu_data.pkt_id = GPRS_RTU_DATA_RAM;
	memcpy(&WEB_data.GPRS_rtu_data.ram_data, &ram_data,sizeof(ram_data_t));
	WEB_data.GPRS_rtu_data.pkt_ready = TRUE;
}

#ifdef GPRS_ENABLE
gprs_upload_data_pkt_id_t controlRTUGprsUpload(void)		//Anand 29-04-2016
{
	gprs_upload_data_pkt_id_t upload_code = GPRS_NO_DATA;
	if ((get_frmwr_update_state() == HEX_FILE_UPDATE_PRG) && (get_flash_file_rqst_type() != HEX_NO_RQST) && (!check_unsent_log()))
	{
		upload_code = GPRS_HEX_FILE_DATA;
	}
	
#ifdef ONLINE_CONFIG_EN
//#if 0
	else if(online_cfg.cfg_pending)
	{
		upload_code = GPRS_ONLINE_CFG;
	} 
	
//#endif
#endif
	else if((getRtuState() == RTU_GPRS_CONFIG))
	{
		upload_code = GPRS_RTU_CONFIG;
	}
	/************************************************************************/
	/*02-01-21 SK: commented because currently we dont have server to recieve these requests*/
	/************************************************************************/
	else if(freq_updated_data.unsent_rtu_logs_GPRS)
	{
#ifdef _DEBUG_UNSENT_LOGS
UWriteString("A1", UART_PC);
		UWriteString("UNSENT:",UART_PC);
		UWriteInt(freq_updated_data.unsent_rtu_logs_GPRS,UART_PC);
#endif
		unsigned long addr = 0;
		FL_rtu_log_t temp_data;
		if((flashGetCurrDataWriteAddr(RTU) - (freq_updated_data.unsent_rtu_logs_GPRS * FL_RTU_LOG_LEN)) >= FL_RTU_START_ADDR)		//Subtraction must not result negative coz the variable is unsigned!
		{
			addr = flashGetCurrDataWriteAddr(RTU) - (freq_updated_data.unsent_rtu_logs_GPRS * FL_RTU_LOG_LEN);
		}
		else
		{
			addr = FL_RTU_MAX_ADDR + flashGetCurrDataWriteAddr(RTU) - FL_RTU_START_ADDR - ((freq_updated_data.unsent_rtu_logs_GPRS)*FL_RTU_LOG_LEN) ;		//Anand 20-7-15
		}//AM?
		
		if(gprs_rx_isr_handler.state == GPRS_RX_IDLE)
		{
			#ifdef DEBUG_SPCL
			UWriteString(" flash read: ",UART_PC);
			#endif
			memset(&temp_data, 0, sizeof(FL_rtu_log_t));
			memset(&WEB_data.GPRS_rtu_data, 0, sizeof(WEB_data_u));
			cli();
			WREN();
			readContToBuff(addr, FL_RTU_LOG_LEN, (char *)&temp_data);
			//Wait_Busy();
			WRDI();				// HJ 29-12-2015    // Write Disable
			sei();
			if (temp_data.chksum == getChecksum((unsigned char*)&temp_data, (sizeof(FL_rtu_log_t) - 1)))
			{
				//WEB_data.GPRS_lvl_data.pkt_id = (0xA0 | 1);
				WEB_data.GPRS_rtu_data.pkt_id = GPRS_RTU_DATA_FLASH;
				memcpy(&WEB_data.GPRS_rtu_data.ram_data, &temp_data.ram_data, sizeof(ram_data_t));
				upload_code = GPRS_RTU_DATA_FLASH;
				//break;
			}
			else
			{
				upload_code = GPRS_NO_DATA;
				#ifdef DEBUG_SPCL
				UWriteString("\nFLASH Read Error", UART_PC);
				#endif
				//Read previous data
				if(freq_updated_data.unsent_rtu_logs_GPRS>0)
				{
					--freq_updated_data.unsent_rtu_logs_GPRS;
					flashWriteFreqUpdatedConfig();
				}
			}
		}
		else
		{
			upload_code = GPRS_NO_DATA;
			#ifdef DEBUG_SPCL
			UWriteString("\nchannel busy", UART_PC);
			#endif
		}
#ifdef DEBUG_ONLINE_DATA

#endif
	}
	else if (WEB_data.GPRS_rtu_data.pkt_ready)
	{
#ifdef GPRS_DEBUG
UWriteString("A0", UART_PC);
#endif
		upload_code = GPRS_RTU_DATA_RAM;
		//WEB_data.GPRS_rtu_data.pkt_ready = FALSE;
		/*if(db_save_status == TRUE)
		{
			db_save_status = FALSE;
			
		}*/
	}
	/*02-01-21 SK: commented because currently we dont have server to recieve these requests*/
	return upload_code;		
}
void decrement_unsent_log_cnt(char upload_code)
{
	if (upload_code == GPRS_RTU_DATA_FLASH)
	{
		if(freq_updated_data.unsent_rtu_logs_GPRS>0)
			--freq_updated_data.unsent_rtu_logs_GPRS;
		
		flashWriteFreqUpdatedConfig();
		//eepromWriteFreqUpdatedConfig();
	}
	/*else if (upload_code == GPRS_EVT_DATA)
	{
		--tm_freq_updated_data.unsent_evt_logs_GPRS;
	}*/
	
	//memset(&WEB_data, 0, sizeof(WEB_data_u));
	
#ifdef DEBUG_SPCL
UWriteString("Dcr:", UART_PC);
#endif
}
char check_unsent_log(void)
{
	
		if (freq_updated_data.unsent_rtu_logs_GPRS || WEB_data.GPRS_rtu_data.pkt_ready 
#ifdef ONLINE_CONFIG_EN
//#if 0
		|| online_cfg.cfg_pending
//#endif
#endif
		 /*|| tm_freq_updated_data.unsent_evt_logs_GPRS*/)		//19-12-2019 SK: updated

	{
/*
#ifdef GPRS_DEBUG
UWriteString("chk_un:t",UART_PC);
#endif*/
		return 1;
	}
	else
	{
/*
#ifdef GPRS_DEBUG
UWriteString("chk_un:f",UART_PC);
#endif*/

		return 0;
	}
}
#endif



void bootingUp(void)		
{
	init_ext_interrupt();	// 22-2-19 VC: need to init before do any thing
	
	timer0Init();	
#ifdef FLASH_LOG_DATA	
	flashInit();
#else
	read_e2p_log_data();
#endif
	//getDateTime();
	/*initUart(UART_PC);*/ //,BAUD_RATE_FMS);
	//initUart(UART_485); //,BAUD_RATE_FMS);
#ifdef GPRS_ENABLE
	initUart(GPRS_UART);				//For GPRS
	gprsPowerUpInit();
#ifdef ONLINE_CONFIG_EN
#ifdef DEBUG_SIMULATED_CONFIG			//disable this before release
	/*set_online_cfg_ver_cnt(1);			//set ver_cnt to 1 forcefully to enable config download online
	eeprom_write_online_cfg_cnt();*/
#endif
	init_online_cfg();
#endif
#endif
	
// 	initPulser(); // VC:23/08/2018
// 	pulserON(); // VC:23/08/2018
	
	initADC();
	
/*
	adxl345_init();
	l3g4200_init();*/
	init_hex_file_data();
	updt_sys_config();
	initFMSensor();
	_delay_ms(100);		//Anand 30.04.2014
}


#ifdef GPRS_ENABLE
#ifdef HTTP_ENABLE
char rtuPrepareGPRSData(char *buff, gprs_upload_data_pkt_id_t upload_code)
{
	char buffFilled = FALSE;		//whether we filled the buff or not
	unsigned int index = 0,i;
	//unsigned char temp_status[8];
	strcpy(buff, "AT+HTTPPARA=\"URL\",\"http://");
	index = strlen(buff);
	//strcpy(&buff[index], (char *)&gprs_config_data.websiteUrl);
	strcpy(&buff[index], (char *)"3.111.149.2/");
	index = strlen(buff);
	
	if(upload_code == GPRS_HEX_FILE_DATA)
	{
		//prepareHexFileServerPkt(&buff[index]);
		switch(flash_hexfile.state)
		{
			case HEX_CHK_FRMWR_UPDT:
			/*strcpy(&buff[index], "3.81.95.143/");
			index = strlen(buff);*/
			sprintf(&buff[index], "bhujal/api/v1/devices/chkforfrmwrupd?data=%s%s%04X\"\r", PRODUCT_CODE, FIRMWARE_VER, get_rtu_cnfg());
			break;
			
			case HEX_RQST_NEW_REC:
			/*strcpy(&buff[index], "3.81.95.143/");
			index = strlen(buff);*/
			sprintf(&buff[index], "bhujal/api/v1/devices/sendnextrecords?data=%s%s%04X%04u%u\"\r", PRODUCT_CODE, get_e2p_frm_ver(), get_rtu_cnfg(), (flash_hexfile.pkt_no + 1), NUM_OF_PKT_PER_QUERY);
			break;
			default:
			break;
		}
	}
	else if (upload_code == GPRS_RTU_CONFIG)
	{
		strcpy(&buff[index], "bhujal/api/v1/devices/config?data=");
		index = strlen(buff);
		memcpy(&buff[index], readImei(), IMEI_NUM_LEN);
		sprintf(&buff[index + IMEI_NUM_LEN], "\"\r");
		
		
		/*
				just for testing purpose after testing comment down the below packet and uncomment the upper packet.
		*/
		
		//sprintf(&buff[index],"jpnt/api/v1/devices/config?data=%s",readImei());
	}
	
#ifdef ONLINE_CONFIG_EN
	else if (upload_code == GPRS_ONLINE_CFG)
	{
//#if 0
		switch(online_cfg.online_cfg_state)
		{
			case CFG_VER_CNT:
			sprintf(&buff[index], "bhujal/api/v1/devices/ver_cnt_req?data=%04d%s",product_info.industry_id,FIRMWARE_VER);
			//strcpy(&buff[index],"ver_cnt_req?data=");
			index = strlen(buff);
			/*for (int i = 0; i < IMEI_NUM_LEN; i++, index++)
			{
				buff[index] = gprs.imei[i];
			}*/
			
			sprintf(&buff[index], "\"\r");
			break;
			
			case CFG_DW_DATA:
			//strcpy(&buff[index],"config_req?data=");
			//sprintf(&buff[index], "jpnt/api/v1/devices/config?data=");
			sprintf(&buff[index], "bhujal/api/v1/devices/config?data=%04d%s",product_info.industry_id,FIRMWARE_VER);
			index = strlen(buff);
#ifdef DEBUG_SIMULATED_CONFIG
			//set_imei((char *)"123456789012345");
#endif
			/*for (int i=0; i < IMEI_NUM_LEN; i++, index++)
			{
				buff[index] = gprs.imei[i];
			}*/
			sprintf(&buff[index], "\"\r");
			break;
			default:
			break;
		}
//#endif
		buffFilled = TRUE;
	}
	
#endif

	//else/* if(upload_code == )*/
	else if(upload_code == GPRS_RTU_DATA_RAM)
	{
		/*strcpy(&buff[index], "www.mindfield.co.in/jpnt/api/v1/logs/insert?data=");
		index = strlen(buff);*/
		sprintf(&buff[index],"bhujal/api/v1/logs/insert?data=%04d%d%04lu.%02lu%07lu.%02lu%07lu.%02lu%07lu.%02lu",
		product_info.industry_id, db_save_status, (WEB_data.GPRS_rtu_data.ram_data.inst_reading / 1000), ((WEB_data.GPRS_rtu_data.ram_data.inst_reading % 1000)/10), (WEB_data.GPRS_rtu_data.ram_data.todays_total / 1000), ((WEB_data.GPRS_rtu_data.ram_data.todays_total % 1000)/10),
		(WEB_data.GPRS_rtu_data.ram_data.months_total / 1000), ((WEB_data.GPRS_rtu_data.ram_data.months_total % 1000)/10), (((WEB_data.GPRS_rtu_data.ram_data.cuml_reading)/ 1000) %10000000), ((WEB_data.GPRS_rtu_data.ram_data.cuml_reading % 1000)/10));
	
		index = strlen(buff);
		for (i = 0; i < 8; i++, index++)
		{
			buff[index] = ((WEB_data.GPRS_rtu_data.ram_data.status & (1 << i)) >> i) + '0';
		}
		sprintf(&buff[index], "20%02d%02d%02d%02d%02d%02d\"\r",WEB_data.GPRS_rtu_data.ram_data.time.yr, WEB_data.GPRS_rtu_data.ram_data.time.mm, WEB_data.GPRS_rtu_data.ram_data.time.dd,
		WEB_data.GPRS_rtu_data.ram_data.time.hr, WEB_data.GPRS_rtu_data.ram_data.time.min, WEB_data.GPRS_rtu_data.ram_data.time.sec);
	}
	
	else if(upload_code == GPRS_RTU_DATA_FLASH)//AM 27-05-21
	{
		/*strcpy(&buff[index], "www.mindfield.co.in/jpnt/api/v1/logs/insert?data=");
		index = strlen(buff);*/
#if 0
		db_save_status=TRUE;
#endif
		sprintf(&buff[index],"bhujal/api/v1/logs/insert?data=%04d%d%04lu.%02lu%07lu.%02lu%07lu.%02lu%07lu.%02lu",
		product_info.industry_id, db_save_status, (WEB_data.GPRS_rtu_data.ram_data.inst_reading / 1000), ((WEB_data.GPRS_rtu_data.ram_data.inst_reading % 1000)/10), (WEB_data.GPRS_rtu_data.ram_data.todays_total / 1000), ((WEB_data.GPRS_rtu_data.ram_data.todays_total % 1000)/10),
		(WEB_data.GPRS_rtu_data.ram_data.months_total / 1000), ((WEB_data.GPRS_rtu_data.ram_data.months_total % 1000)/10), (((WEB_data.GPRS_rtu_data.ram_data.cuml_reading)/1000) %10000000), ((WEB_data.GPRS_rtu_data.ram_data.cuml_reading % 1000)/10));
	
		index = strlen(buff);
		for (i = 0; i < 8; i++, index++)
		{
			buff[index] = ((WEB_data.GPRS_rtu_data.ram_data.status & (1 << i)) >> i) + '0';
		}
		sprintf(&buff[index], "20%02d%02d%02d%02d%02d%02d\"\r",WEB_data.GPRS_rtu_data.ram_data.time.yr, WEB_data.GPRS_rtu_data.ram_data.time.mm, WEB_data.GPRS_rtu_data.ram_data.time.dd,
		WEB_data.GPRS_rtu_data.ram_data.time.hr, WEB_data.GPRS_rtu_data.ram_data.time.min, WEB_data.GPRS_rtu_data.ram_data.time.sec);
	}
	
	//*len = strlen(buff);
	buffFilled = TRUE;

#if 0
	else if (upload_code == GPRS_EVT_DATA)
	{
		#ifdef GPRS_DEBUG
		UWriteString("DISP PKT", UART_PC);
		#endif
		sprintf(&buff[index], "%02X", WEB_data.GPRS_evt_data.pkt_id);
		index = strlen(buff);
	
		sprintf(&buff[index], "%c%02d%05u.%01d%05u.%01d%04X20%02d-%02d-%02d%02d:%02d:%02d",WEB_data.GPRS_evt_data.evt_type,
		WEB_data.GPRS_evt_data.evt_code, WEB_data.GPRS_evt_data.ram_data.raw_lvl, WEB_data.GPRS_evt_data.ram_data.raw_lvl_dp,
		WEB_data.GPRS_evt_data.ram_data.compensated_lvl, WEB_data.GPRS_evt_data.ram_data.compensated_lvl_dp, WEB_data.GPRS_evt_data.ram_data.Status,
		WEB_data.GPRS_evt_data.ram_data.time.yr, WEB_data.GPRS_evt_data.ram_data.time.mm, WEB_data.GPRS_evt_data.ram_data.time.dd,
		WEB_data.GPRS_evt_data.ram_data.time.hr, WEB_data.GPRS_evt_data.ram_data.time.min, WEB_data.GPRS_evt_data.ram_data.time.sec);
		index = strlen(buff);
	
		for(i = 0 ; i < LAT_LEN; ++i, ++index)
		{
			buff[index] = WEB_data.GPRS_evt_data.ram_data.Latitude[i];
		}
		for(i = 0 ; i < LONG_LEN; ++i, ++index)
		{
			buff[index] = WEB_data.GPRS_evt_data.ram_data.Longitude[i];
		}
	
	sprintf(&buff[index], "%03d.%01d%03d.%02d}",WEB_data.GPRS_evt_data.ram_data.speed, WEB_data.GPRS_evt_data.ram_data.speed_dp,
	WEB_data.GPRS_evt_data.ram_data.orientation, WEB_data.GPRS_evt_data.ram_data.orientation_dp);

	buffFilled = TRUE;
}
#endif
return buffFilled;
}
#endif

#ifdef TCP_ENABLE
char rtuPrepareGPRSData(char *buff,int *len, gprs_upload_data_pkt_id_t upload_code)
{

	char buffFilled = FALSE;		//whether we filled the buff or not
	
	buff[0] = upload_code;
		
	memcpy(&buff[1],&WEB_data.GPRS_rtu_data.ram_data,sizeof(ram_data_t)); //VC: 2/08/2018 copy the all web ram data into send buff
	
	*len = sizeof(ram_data_t)  + 1;
	
	buffFilled = TRUE;

#if 0	
	else if (upload_code == GPRS_EVT_DATA)
	{
#ifdef GPRS_DEBUG
		UWriteString("DISP PKT", UART_PC);
#endif
		sprintf(&buff[index], "%02X", WEB_data.GPRS_evt_data.pkt_id);
		index = strlen(buff);
		
		sprintf(&buff[index], "%c%02d%05u.%01d%05u.%01d%04X20%02d-%02d-%02d%02d:%02d:%02d",WEB_data.GPRS_evt_data.evt_type,
		WEB_data.GPRS_evt_data.evt_code, WEB_data.GPRS_evt_data.ram_data.raw_lvl, WEB_data.GPRS_evt_data.ram_data.raw_lvl_dp,
		WEB_data.GPRS_evt_data.ram_data.compensated_lvl, WEB_data.GPRS_evt_data.ram_data.compensated_lvl_dp, WEB_data.GPRS_evt_data.ram_data.Status,
		WEB_data.GPRS_evt_data.ram_data.time.yr, WEB_data.GPRS_evt_data.ram_data.time.mm, WEB_data.GPRS_evt_data.ram_data.time.dd,
		WEB_data.GPRS_evt_data.ram_data.time.hr, WEB_data.GPRS_evt_data.ram_data.time.min, WEB_data.GPRS_evt_data.ram_data.time.sec);
		index = strlen(buff);
		
		for(i = 0 ; i < LAT_LEN; ++i, ++index)
		{
			buff[index] = WEB_data.GPRS_evt_data.ram_data.Latitude[i];
		}
		for(i = 0 ; i < LONG_LEN; ++i, ++index)
		{
			buff[index] = WEB_data.GPRS_evt_data.ram_data.Longitude[i];
		}
	
		sprintf(&buff[index], "%03d.%01d%03d.%02d}",WEB_data.GPRS_evt_data.ram_data.speed, WEB_data.GPRS_evt_data.ram_data.speed_dp,
		WEB_data.GPRS_evt_data.ram_data.orientation, WEB_data.GPRS_evt_data.ram_data.orientation_dp);
		
		buffFilled = TRUE;
	}
#endif
	return buffFilled;
}
char rtuPrepareGPRSData_Mb(char *buff,int *len, gprs_upload_data_pkt_id_t upload_code)
{

	char buffFilled = FALSE;		//whether we filled the buff or not
	float temp;	
/*
	buff[0] = upload_code;
	
	memcpy(&buff[1],&WEB_data.GPRS_rtu_data.ram_data,sizeof(ram_data_t)); //VC: 2/08/2018 copy the all web ram data into send buff
	
	*len = sizeof(ram_data_t)  + 1;*/
	memset(buff,0,sizeof(*buff));







	buff[0] = (mbtcp_payload_data.transction_id >>8);
	buff[1] = (mbtcp_payload_data.transction_id );
	buff[2] = (mbtcp_payload_data.protocol_id >>8);
	buff[3] = (mbtcp_payload_data.protocol_id);
	buff[4] = (mbtcp_payload_data.total_pkt_lgth >>8);
	buff[5] = (mbtcp_payload_data.total_pkt_lgth);
	buff[6] = (mbtcp_payload_data.slave_id);
	buff[7] = (mbtcp_payload_data.func_code);
	buff[8] = (mbtcp_payload_data.length_of_data);
	buff[9] = 0;
	buff[10] = upload_code;
	buff[11] = 0;
	buff[12] = (ram_data.pump_data.system_status);
	buff[13] = 0;
	buff[14] = ram_data.pump_data.ip_status;
	buff[15] = 0;
	buff[16] = ram_data.pump_data.op_status;
	buff[17] = (ram_data.pump_data.inst_flow >> 8);
	buff[18] = (ram_data.pump_data.inst_flow );
	buff[19] = (ram_data.pump_data.cum_flow >> 24);
	buff[20] = (ram_data.pump_data.cum_flow >> 16);
	buff[21] = (ram_data.pump_data.cum_flow >> 8);
	buff[22] = (ram_data.pump_data.cum_flow);
	buff[23] = (ram_data.pump_data.pressure_in >> 8);
	buff[24] = (ram_data.pump_data.pressure_in);
	buff[25] = (ram_data.pump_data.pressure_out >> 8);
	buff[26] = (ram_data.pump_data.pressure_out);

	temp = (ram_data.EM_data.V_R +(ram_data.EM_data.V_R_dp/100.0));
	convertFloatToChar((char *)&temp,&buff[27],4);

	temp = 0;
	temp = (ram_data.EM_data.V_Y +(ram_data.EM_data.V_Y_dp/100.0));
	convertFloatToChar((char *)&temp,&buff[31],4);
 
	temp = 0;
	temp = (ram_data.EM_data.V_B +(ram_data.EM_data.V_B_dp/100.0));
	convertFloatToChar((char *)&temp,&buff[35],4);
 
	temp = 0;
	temp = (ram_data.EM_data.I_R +(ram_data.EM_data.I_R_DP/100.0));
	convertFloatToChar((char *)&temp,&buff[39],2);

	temp = 0;
	temp = (ram_data.EM_data.I_Y +(ram_data.EM_data.I_Y_DP/100.0));
	convertFloatToChar((char *)&temp,&buff[41],2);

	temp = 0;
	temp = (ram_data.EM_data.I_B +(ram_data.EM_data.I_B_DP/100.0));
	convertFloatToChar((char *)&temp,&buff[43],2);
 
	temp = 0;
	temp = (ram_data.EM_data.PF_R +(ram_data.EM_data.PF_R_dp/100.0));
	convertFloatToChar((char *)&temp,&buff[45],2);
 
	temp = 0;
	temp = (ram_data.EM_data.PF_Y +(ram_data.EM_data.PF_Y_dp/100.0));
	convertFloatToChar((char *)&temp,&buff[47],2);

	temp = 0;
	temp = (ram_data.EM_data.PF_B +(ram_data.EM_data.PF_B_dp/100.0));
	convertFloatToChar((char *)&temp,&buff[49],2);
 
	temp = 0;
	temp = (ram_data.EM_data.PF_AVG +(ram_data.EM_data.PF_AVG_dp/100.0));
	convertFloatToChar((char *)&temp,&buff[51],2);
 
	temp = 0;
	temp = (ram_data.EM_data.KWh +(ram_data.EM_data.KWh_dp/100.0));
	convertFloatToChar((char *)&temp,&buff[53],4);
 
	temp = 0;
	temp = (ram_data.EM_data.KVah +(ram_data.EM_data.KVah_dp/100.0));
	convertFloatToChar((char *)&temp,&buff[57],4);

	/*buff[27] = (ram_data.EM_data.V_R >> 8);
	buff[28] = (ram_data.EM_data.V_R);
	buff[29] = (ram_data.EM_data.V_R_dp >> 8);
	buff[30] = (ram_data.EM_data.V_R_dp);
	buff[31] = (ram_data.EM_data.V_Y >> 8);
	buff[32] = (ram_data.EM_data.V_Y);
	buff[33] = (ram_data.EM_data.V_Y_dp >> 8);
	buff[34] = (ram_data.EM_data.V_Y_dp);
	buff[35] = (ram_data.EM_data.V_B >> 8);
	buff[36] = (ram_data.EM_data.V_B);
	buff[37] = (ram_data.EM_data.V_B_dp >> 8);
	buff[38] = (ram_data.EM_data.V_B_dp);
	buff[39] = (ram_data.EM_data.I_R );
	buff[40] = (ram_data.EM_data.I_R_DP);
	buff[41] = (ram_data.EM_data.I_Y );
	buff[42] = (ram_data.EM_data.I_Y_DP);
	buff[43] = (ram_data.EM_data.I_B );
	buff[44] = (ram_data.EM_data.I_B_DP);
	buff[45] = (ram_data.EM_data.PF_R );
	buff[46] = (ram_data.EM_data.PF_R_dp);
	buff[47] = (ram_data.EM_data.PF_Y );
	buff[48] = (ram_data.EM_data.PF_Y_dp);
	buff[49] = (ram_data.EM_data.PF_B );
	buff[50] = (ram_data.EM_data.PF_B_dp);
	buff[51] = (ram_data.EM_data.PF_AVG );
	buff[52] = (ram_data.EM_data.PF_AVG_dp);
	buff[53] = (ram_data.EM_data.KWh >> 24);
	buff[54] = (ram_data.EM_data.KWh >> 16);
	buff[55] = (ram_data.EM_data.KWh >> 8);
	buff[56] = (ram_data.EM_data.KWh);
	buff[57] = (ram_data.EM_data.KWh_dp >> 8);
	buff[58] = (ram_data.EM_data.KWh_dp);
	buff[59] = (ram_data.EM_data.KVah >> 24);
	buff[60] = (ram_data.EM_data.KVah >> 16);
	buff[61] = (ram_data.EM_data.KVah >> 8);
	buff[62] = (ram_data.EM_data.KVah);
	buff[63] = (ram_data.EM_data.KVah_dp >> 8);
	buff[64] = (ram_data.EM_data.KVah_dp);
	*/
	buff[61] = 0;
	buff[62] = ram_data.time.yr;
	buff[63] = 0;
	buff[64] = ram_data.time.mm;
	buff[65] = 0;
	buff[66] = ram_data.time.dd;
	buff[67] = 0;
	buff[68] = ram_data.time.hr;
	buff[69] = 0;
	buff[70] = ram_data.time.min;
	buff[71] = 0;
	buff[72] = ram_data.time.sec;

	*len = DATA_PKT_LGTH;
	
	buffFilled = TRUE;

	
return buffFilled;
}
#endif
#endif

char prepareConnectPayload(char *buff, int* len)
{
	char buffFilled = FALSE;		//whether we filled the buff or not
	int index = 0;
	
	sprintf(buff, "{");
		
	index = strlen(buff);
	//for(i = 0 ; i < SIM_NUM_LEN; ++i, ++index)
//	for(i = 3 ; i < SIM_NUM_LEN; ++i, ++index) //06/10/2018 VC:
	{
		//product_info.product_id = 200;
		buff[index] = (product_info.industry_id & 0xff);//sim_info.sim_no[i];			//??
	}
	
	#ifdef GPRS_DEBUG
	UWriteString("DISP PKT", UART_PC);
	#endif
	
	//sprintf(&buff[index], "%02X", WEB_data.GPRS_lvl_data.pkt_id);
	index = strlen(buff);
/*			
	sprintf(&buff[index], "%05u.%01d%05u.%01d%04X20%02d-%02d-%02d%02d:%02d:%02d",WEB_data.GPRS_lvl_data.ram_data.raw_lvl,
	WEB_data.GPRS_lvl_data.ram_data.raw_lvl_dp,	WEB_data.GPRS_lvl_data.ram_data.compensated_lvl, WEB_data.GPRS_lvl_data.ram_data.compensated_lvl_dp,
	WEB_data.GPRS_lvl_data.ram_data.Status, WEB_data.GPRS_lvl_data.ram_data.time.yr, WEB_data.GPRS_lvl_data.ram_data.time.mm,
	WEB_data.GPRS_lvl_data.ram_data.time.dd, WEB_data.GPRS_lvl_data.ram_data.time.hr, WEB_data.GPRS_lvl_data.ram_data.time.min,
	WEB_data.GPRS_lvl_data.ram_data.time.sec);
	index = strlen(buff);
			
	for(i = 0 ; i < LAT_LEN; ++i, ++index)
	{
		buff[index] = WEB_data.GPRS_lvl_data.ram_data.Latitude[i];
	}
	for(i = 0 ; i < LONG_LEN; ++i, ++index)
	{
		buff[index] = WEB_data.GPRS_lvl_data.ram_data.Longitude[i];
	}
	sprintf(&buff[index], "%03d.%01d%03d.%02d}",WEB_data.GPRS_lvl_data.ram_data.speed, WEB_data.GPRS_lvl_data.ram_data.speed_dp,
	WEB_data.GPRS_lvl_data.ram_data.orientation, WEB_data.GPRS_lvl_data.ram_data.orientation_dp);
*/	
	sprintf(&buff[index], "}");
	//sprintf(buff, "}");
	index = strlen(buff);
	*len = index;	
	buffFilled = TRUE;	
	
	return buffFilled;		//Anand 05-05-16
}


void save_FL_rtu_data()
{	
	flashWriteLR(&FL_rtu_log);
	flashWriteMR_BR();
}

/*void calculate_spd(void);				//Get vehicle speed based on accelerometer

void update_status_bits(void);			//Get various status bits

void get_location(void);
void get_time(void);
*/

void prepare_xyz_raw_data(unsigned int *pkt, uint8_t *temp, unsigned char no_of_bytes)
{
	for (int i = 0; i < no_of_bytes; i++)
	{
		pkt[i] = (((unsigned int)temp[(i * 2) + 1] << 8) | ((unsigned int)temp[i * 2]));
	}
}


/*
void write_default_rtu_config(rtu_config_t *rtu_config)
{	
	memset((void *)rtu_config, 0, sizeof(rtu_config_t));
	
	rtu_config->rtu_MB_slav_Addr = DEFAULT_RTU_SLV_ADDR;
	
	eepromWriteRTUConfig(rtu_config);

}*/

void writeDefaults(void)
{	
	//freq_updated_data.cnt = 0;			// HJ 31-08-2016
	
	//write_default_rtu_config(&rtu_config);	
//need to update as for new struct 11-12-18
	//memset((void *)&sim_info, 0, sizeof(sim_info_t));		//Anand 05-04-16
	//memcpy(sim_info.sim_no, "+910987654321", (strlen("+910987654321")));//9529545685
	//memcpy(sim_info.sim_no, DEFAULT_SIM_NUM, (strlen("+911234567890")+1));
	//eepromWriteSIMConfig();
	
	freq_updated_data.unsent_rtu_logs_GPRS = 0;
	//tm_freq_updated_data.unsent_evt_logs_GPRS = 0;
	
#ifdef FLASH_LOG_DATA
	flashClearFreqUpdatedConfig();
#endif
	/*eeprom_write_block(/ *(void*)* /&bowser_cfg_data,/ *(void*)* /&addr,sizeof(bowser_cfg_data_t));		// R*/

	FL_rtu_log.transaction_id_rtu_log = 0;	//Added by KP 9-5-15		Moved Here by HJ 10.05.2015		//Anand 04-07-15 0 Beacause we first increment and then save the dispense logs.
	FL_rtu_log.transaction_id_MSB_rtu_log = 0;	//Added by Anand 2-7-15
	
	//write_default_GSM_config();
	write_default_GPRS_config();
	memset(&product_info,0,sizeof(product_info_t));
	//eepromWriteProductInfo();
	write_default_product_info();
#ifdef GPRS_ENABLE
	/*memset((void *)&GPRSCfgData, 0, sizeof(GPRSCfgData_t));		//Anand 05-04-16
	memcpy(GPRSCfgData.apn, gsm_cfg_data.apn, APN_LEN);//(strlen("airtelgprs.com")+1))*/;
	//memcpy(GPRSCfgData.websiteUrl, "www.mindfield.co.in/fd/", (strlen("www.mindfield.co.in/fd/")+1));
	//eepromWriteGPRSConfig();
#endif
	//strcpy(&buff[index], "115.248.195.049:8082/index.htm?data=");
	//update_url();
}

void read_IO_status()
{
	//ram_data.pump_data.op_status = 0;
	//ram_data.pump_data.ip_status = 0;
	
	for(uint8_t i = 0; i < (NUM_OF_MB_IO / 2); i++)
	{
		//ram_data.pump_data.ip_status |= (digitalReadRtu(MB_FC_READ_DISCRETE_INPUTS_OFFSET_ADDR + i)<<i);	
	}
	for(uint8_t i = 0; i < (NUM_OF_MB_IO / 2); i++)
	{
		//ram_data.pump_data.op_status |= (digitalReadRtu(MB_FC_READ_COILS_OFFSET_ADDR + i)<<i);
	}

/*
#ifdef DEBUG_TIME
	UWriteString("op_status:",UART_PC);
	UWriteInt((int)(ram_data.pump_data.op_status),UART_PC);
	UWriteString("ip_status:",UART_PC);
	UWriteInt((int)(ram_data.pump_data.ip_status),UART_PC);
	UWriteInt(digitalReadRtu(MB_FC_READ_COILS_OFFSET_ADDR ),UART_PC);
#endif	*/

}
void read_flow_data()
{
	//ram_data.inst_reading = flow_meter.fm_instant_data.curr_flow; //sensor_data.ml_s/CONVERT_TO_L;			//Inst reading in Lier/sec		
	//ram_data.inst_reading = sensor_data.ml_s/CONVERT_TO_L;
	ram_data.inst_reading = sensor_data.l_hr;	//inst_reading will be litre per hr
	sensor_data.temp += (sensor_data.l_hr*1000/CONVERT_HR_TO_SEC);		//ml_s
	if(sensor_data.temp >= 1000)
	{
		ram_data.todays_total += sensor_data.temp/1000;
		ram_data.months_total += sensor_data.temp/1000;
		ram_data.cuml_reading += sensor_data.temp/1000;
		sensor_data.temp %= 1000;
	}
	//ram_data.cuml_reading = flow_meter.fm_instant_data.net_flow;
#ifdef DEBUG_RAM_DATA
	UWriteString("temp:",UART_PC);
	UWriteInt(sensor_data.temp,UART_PC);
	UWriteString("ram_data:",UART_PC);
	UWriteInt(ram_data.inst_reading,UART_PC);
	UWriteData(',',UART_PC);
	UWriteInt(ram_data.todays_total,UART_PC);
	UWriteData(',',UART_PC);
	UWriteInt(ram_data.cuml_reading,UART_PC);	
#endif
/*
#ifdef _DEBUG_RAM_DATA
UWriteString("temp:",UART_PC);
UWriteInt(sensor_data.temp,UART_PC);
#endif*/
	/*ram_data.inst_dp = sensor_data.avg%CONVERT_TO_L;*/
	
	/*temp_val = sensor_data.avg/CONVERT_TO_ML;				//reading in KL
	temp_dp = sensor_data.avg%CONVERT_TO_ML;*/
	
	/*ram_data.todays_total += temp_val;
	ram_data.todays_total_dp += temp_dp;*/
	
	//ram_data.todays_total += ram_data.inst_reading/1000;
	//ram_data.todays_total = (ram_data.todays_total + ram_data.inst_reading)/1000;
	//ram_data.todays_total /= 1000;
	/*if(ram_data.todays_total_dp >= CONVERT_TO_KL)
	{
		ram_data.todays_total += ram_data.todays_total_dp/CONVERT_TO_KL;
		ram_data.todays_total_dp %= CONVERT_TO_KL;
		
	}*/
	
	/*ram_data.months_total += temp_val;
	ram_data.months_total_dp += temp_dp;*/
	
	//ram_data.months_total += ram_data.inst_reading/1000;
	//ram_data.months_total /= 1000;	
	/*if(ram_data.months_total_dp >= CONVERT_TO_KL)
	{
		ram_data.months_total += ram_data.months_total_dp/CONVERT_TO_KL;
		ram_data.months_total_dp %= CONVERT_TO_KL;
		
	}*/
	//ram_data.cuml_reading += ram_data.inst_reading/1000;
	//ram_data.cuml_reading /= 1000;
	//sensor_data.avg = 0;
/*
	
#ifdef _DEBUG_RS_485
	UWriteString(" ins:",UART_PC);
	UWriteInt(ram_data.inst_reading,UART_PC);
	/ *UWriteString(" ins_dp:",UART_PC);
	UWriteInt(ram_data.inst_dp,UART_PC);* /
	UWriteString(" td:",UART_PC);
	UWriteInt(ram_data.todays_total,UART_PC);
/ *
	UWriteString(" td_dp:",UART_PC);
	UWriteInt(ram_data.todays_total_dp,UART_PC);* /
	UWriteString(" mnt:",UART_PC);
	UWriteInt(ram_data.months_total,UART_PC);
	/ *UWriteString(" mnt_dp:",UART_PC);
	UWriteInt(ram_data.months_total_dp,UART_PC);* /
#endif*/
}
void read_pressure_data()
{
#ifdef SIMULATED_DATA
	
	static unsigned int pre_in = 10, pre_out = 5 ;
	//static char kw_dp = 9, Kv_dp = 8;
	//ram_data.pump_data.pressure_in = pre_in;
	//ram_data.pump_data.pressure_out = pre_out;
	pre_out++;
	pre_in++;
/*
	ram_data.EM_data.KWh = pre_in;
	ram_data.EM_data.KWh_dp = kw_dp;
	ram_data.EM_data.KVah = pre_in - 2;
	ram_data.EM_data.KVah_dp = Kv_dp;
	ram_data.EM_data.PF_AVG = 1;
	ram_data.EM_data.PF_AVG_dp = 0;
*/
#endif

}

void updateRamData()
{
	read_flow_data();
	check_for_day_month_change();
	
#ifdef DEBUG_TODAY_TOT
	char buff[20];
	UWriteData('\n', UART_PC);
	sprintf(buff,"%02d:%02d:%02d,%02d/%02d/%02d,",gprs_date_time.hr,gprs_date_time.min,gprs_date_time.sec,gprs_date_time.dd,gprs_date_time.mm,gprs_date_time.yy);
	UWriteString(buff,UART_PC);
	memset(&buff,0,sizeof(buff));
	sprintf(buff,"%02d:%02d:%02d,%02d/%02d/%02d,",ram_data.time.hr,ram_data.time.min,ram_data.time.sec,ram_data.time.dd,ram_data.time.mm,ram_data.time.yr);
	UWriteString(buff,UART_PC);
	UWriteInt(ram_data.todays_total,UART_PC);
	UWriteData(',', UART_PC);
	UWriteInt(yest_total,UART_PC);
#endif
}

#if 0
void update_time_from_log()
{
	char temp_time[6];
	int indx = 0;
	if(last_record.ram_data.time.yr >= DEFAULT_YEAR)
	{
		temp_time[indx++] = last_record.ram_data.time.yr;
		temp_time[indx++] = last_record.ram_data.time.mm;
		temp_time[indx++] = last_record.ram_data.time.dd;
		temp_time[indx++] = 1;//Day: 1-7, 1=Sunday
		temp_time[indx++] = last_record.ram_data.time.hr;
		temp_time[indx]   = last_record.ram_data.time.min;
		set_rtc((unsigned char *)&temp_time);
	}
	
}
#endif
/*
void compare_time_from_log()
{
	bool today_reset_flag = false;
	timeDiff = calcTimeDiff(ram_data.time, last_record.ram_data.time);

	if (timeDiff.diffDays == 1)
	{
		yest_total = ram_data.todays_total;
	}
	else
	{
		yest_total = 0xffffffff;
	}
	if (timeDiff.diffDays > 0 || timeDiff.diffHrs > 0 || timeDiff.diffMins > 0)
	{
		if (today_reset_flag)
		{
			// Reset today's total if today_reset_flag is true
			ram_data.todays_total = 0;
			today_reset_flag = false;
		}
		else
		{
			ram_data.todays_total += yest_total; //if day didn't change
		}
		// Update month's total if needed
		if (timeDiff.diffDays > 0 && ram_data.time.dd == 1)
		{
			ram_data.months_total = 0;
		}
	}
}
*/

void compare_time_from_log()
{
	if(last_record.ram_data.time.yr >= DEFAULT_YEAR)
	{
		if(last_record.ram_data.time.yr == ram_data.time.yr	&&	
		last_record.ram_data.time.mm == ram_data.time.mm	&&	
		last_record.ram_data.time.dd != ram_data.time.dd)
		{
			if(ram_data.time.dd==last_record.ram_data.time.dd+1)
			{
				yest_total = ram_data.todays_total;
#ifdef DEBUG_TOT_DATA
				UWriteString("<yupd_D:",UART_PC);
#endif
			}
			else
			{
				yest_total=0xffffffff;
			}
			ram_data.todays_total = 0;
			//save fl log
#ifdef FLASH_LOG_DATA
			prepare_FL_LogData();
			save_FL_rtu_data();
			//++freq_updated_data.unsent_rtu_logs_GPRS;
			//flashWriteFreqUpdatedConfig();
#else
			prepare_e2p_logdata();
			e2p_write_log_data();
#endif
			prepare_e2p_y_total();
			e2p_write_y_total();
			sms_alert_state = CHECK_ALERT;
		}
		else if(last_record.ram_data.time.yr == ram_data.time.yr	&&	last_record.ram_data.time.mm != ram_data.time.mm)
		{
			if(ram_data.time.dd==1	&&	
			((last_record.ram_data.time.dd==29 && last_record.ram_data.time.mm==2)	||
			(last_record.ram_data.time.dd==28 && last_record.ram_data.time.mm==2) ||
			(last_record.ram_data.time.dd==30	&& (last_record.ram_data.time.mm==4||last_record.ram_data.time.mm==6||
			last_record.ram_data.time.mm==9||last_record.ram_data.time.mm==11))||
			(last_record.ram_data.time.dd==31 && (last_record.ram_data.time.mm==1||last_record.ram_data.time.mm==3||last_record.ram_data.time.mm==5||
			last_record.ram_data.time.mm==7||last_record.ram_data.time.mm==8||last_record.ram_data.time.mm==10||last_record.ram_data.time.mm==12))))
			{
				yest_total = ram_data.todays_total;
#ifdef DEBUG_TOT_DATA
				UWriteString("<yupd_M:",UART_PC);
#endif
			}
			else
			{
				yest_total=0xffffffff;
			}
			ram_data.todays_total = 0;
			ram_data.months_total = 0;
			//save fl log
#ifdef FLASH_LOG_DATA
			prepare_FL_LogData();
			save_FL_rtu_data();
			//++freq_updated_data.unsent_rtu_logs_GPRS;
			//flashWriteFreqUpdatedConfig();
#else
			prepare_e2p_logdata();
			e2p_write_log_data();
#endif
			prepare_e2p_y_total();
			e2p_write_y_total();
			sms_alert_state = CHECK_ALERT;
		}
		else if(last_record.ram_data.time.yr != ram_data.time.yr)
		{
			if(ram_data.time.mm==1	&&	ram_data.time.dd==1	&& ram_data.time.yr==last_record.ram_data.time.yr+1	&&	
			last_record.ram_data.time.dd==31 && last_record.ram_data.time.mm==12)
			{
				yest_total = ram_data.todays_total;
#ifdef DEBUG_TOT_DATA
				UWriteInt(yest_total,UART_PC);
				UWriteString("<Yupd_Y:",UART_PC);
#endif
			}
			else
			{
				yest_total=0xffffffff;
			}
			ram_data.todays_total = 0;
			ram_data.months_total = 0;
			//save fl log
#ifdef FLASH_LOG_DATA
			prepare_FL_LogData();
			save_FL_rtu_data();
			//++freq_updated_data.unsent_rtu_logs_GPRS;
			//flashWriteFreqUpdatedConfig();
#else
			prepare_e2p_logdata();
			e2p_write_log_data();
#endif
			prepare_e2p_y_total();
			e2p_write_y_total();
			sms_alert_state = CHECK_ALERT;
		}
	}
}

void check_for_day_month_change()
{
	char date, mon, year;
	date = ram_data.time.dd;
	mon = ram_data.time.mm;
	year = ram_data.time.yr;
	
#ifdef DEBUG_TOT_DATA
UWriteString("O_tm:",UART_PC);
UWriteInt(ram_data.time.dd,UART_PC);
#endif
	get_present_time(&ram_data.time);
#ifdef DEBUG_TOT_DATA
UWriteString("N_tm:",UART_PC);
UWriteInt(ram_data.time.dd,UART_PC);
#endif
	if((ram_data.time.yr >= DEFAULT_YEAR && (year >= DEFAULT_YEAR)))//27-0y_err_yt4-2021 SK: In case if RTC is currupted then we will not check day/month change until time is updated from gprs
	{
		if((mon != 0) && (date != 0))	//abdul majeed
		{
			if(year == ram_data.time.yr	&&	mon == ram_data.time.mm	&&	date != ram_data.time.dd)
			{
				if(ram_data.time.dd==date+1)
				{
					yest_total = ram_data.todays_total;
#ifdef DEBUG_TOT_DATA
					UWriteInt(yest_total,UART_PC);
					UWriteString("<Y_Tupd D:",UART_PC);
#endif
				}
				else
				{
					yest_total=0xffffffff;
				}
				prepare_e2p_y_total();
				e2p_write_y_total();
				//reset todays_total after the day change
				//ram_data.todays_total = 0;		np
				sms_alert_state = CHECK_ALERT;
			}
			else if(year == ram_data.time.yr	&&	mon != ram_data.time.mm)
			{
				if(ram_data.time.dd==1	&&	((date ==29 && mon==2)	||(date ==28 && mon==2) ||(date==30	&& (mon==4||mon==6||mon==9||mon==11))	||	(date==31 && (mon==1||mon==3||mon==5||mon==7||mon==8||mon==10||mon==12))))
				{
					yest_total = ram_data.todays_total;
#ifdef DEBUG_TOT_DATA
					UWriteInt(yest_total,UART_PC);
					UWriteString("<Y_Tupd M:",UART_PC);
#endif
				}
				else
				{
					yest_total=0xffffffff;
				}
				prepare_e2p_y_total();
				e2p_write_y_total();
				//reset todays_total and months total after the month change 
				//ram_data.todays_total = 0;		np
				//ram_data.months_total = 0;		np
				//today_reset_flag = TRUE ;		np
				sms_alert_state = CHECK_ALERT;
			}
			else  if(year != ram_data.time.yr)
			{
				if(ram_data.time.mm==1	&&	ram_data.time.dd==1	&& ram_data.time.yr==year+1	&&	date ==31 && mon==12)
				{
					yest_total = ram_data.todays_total;
#ifdef DEBUG_TOT_DATA
				UWriteInt(yest_total,UART_PC);
				UWriteString("<Y_Tupd Y:",UART_PC);
#endif	
				}
				else
				{
					yest_total=0xffffffff;
				}
				prepare_e2p_y_total();
			 	e2p_write_y_total();
				//reset todays_total and months total after the year change
				//ram_data.todays_total = 0;		np
				//ram_data.months_total = 0;		np
				// today_reset_flag = TRUE ;		np
				sms_alert_state = CHECK_ALERT;
			}
// 			else
// 			{
// 				today_reset_flag = FALSE ;		np
// 			}
		}
	}
	else if(ram_data.time.yr < DEFAULT_YEAR)
	{
#ifdef	DEBUG_TOT_DATA
		UWriteString("rtc_f1:",UART_PC);
#endif
	}
	else if(year < DEFAULT_YEAR)//(year == 0)
	{
#ifdef	_DEBUG_TOT_DATA
		UWriteString("tm_upd:",UART_PC);//time updated from GPRS
#endif
	}
	else
	{
#ifdef	DEBUG_TOT_DATA
		UWriteString("yr_err_yt",UART_PC);
#endif
	}

//////////////////////////////////////////////////////////////////////////
// time from gprs
//////////////////////////////////////////////////////////////////////////
	//updatetimefromegprs(&ram_data.time);
	//if((mon != 0) && (date != 0))
#ifdef DEBUG_RTC
char buff[80];
sprintf(buff,"ram_time:-%02d/%02d/%02d %02d:%02d:%02d    ",ram_data.time.dd,ram_data.time.mm,ram_data.time.yr,ram_data.time.hr,ram_data.time.min,ram_data.time.sec);
UWriteString(buff,UART_PC);
#endif
	if((ram_data.time.yr >= DEFAULT_YEAR) && (year >= DEFAULT_YEAR))		//27-04-2021 SK: In case if RTC is currupted then we will not check day/month change until time is updated from gprs
	{
		switch(rtc_state)
		{
			case IDLE:
#ifdef	DEBUG_RTC_STATE
UWriteString("rtc_I:",UART_PC);
#endif
				break;
			case GOT_TIME_FROM_GPRS:
#ifdef	DEBUG_RTC_STATE
UWriteString("rtc_gt:",UART_PC);
#endif
				//time updated from GPRS
				compare_time_from_log();
				set_rtc_state(IDLE);
				break;
				default:
#ifdef	DEBUG_RTC_STATE
UWriteString("rtc_def:",UART_PC);
#endif
				break;
		}
		if((year != ram_data.time.yr )	||	(mon != ram_data.time.mm) )
		{
			//save fl log
#ifdef FLASH_LOG_DATA
			prepare_FL_LogData();
			save_FL_rtu_data();
			++freq_updated_data.unsent_rtu_logs_GPRS;
			flashWriteFreqUpdatedConfig();
#else
			prepare_e2p_logdata();
			e2p_write_log_data();
#endif
			ram_data.months_total = 0;
			ram_data.todays_total = 0;
#ifdef DEBUG_TOT_DATA
UWriteString("t_init:1",UART_PC);
#endif
		}
		else if(date != ram_data.time.dd)
		{
		
			//save fl log
#ifdef FLASH_LOG_DATA
			prepare_FL_LogData();
			save_FL_rtu_data();
			++freq_updated_data.unsent_rtu_logs_GPRS;
			flashWriteFreqUpdatedConfig();
#else
			prepare_e2p_logdata();
			e2p_write_log_data();
#endif
			ram_data.todays_total = 0;
#ifdef DEBUG_TOT_DATA
			UWriteString("t_init:2",UART_PC);
#endif
		}
	}
	else if(ram_data.time.yr < DEFAULT_YEAR)
	{
#ifdef DEBUG_TOT_DATA
		UWriteString("rtc_f2:",UART_PC);
#endif
		//update_time_from_log();
		//if(last_record.ram_data.time.yr >= DEFAULT_YEAR)
		//memcpy((void *)&ram_data.time,(void *)&last_record.ram_data.time,sizeof(time_stamp_t));
	}
	else if(year < DEFAULT_YEAR)//(year == 0)
	{
#ifdef _DEBUG_TOT_DATA
		UWriteString("tm_upd2:",UART_PC);//time updated from GPRS
#endif
		//compiler will come here only if RTC is faulty and time updated from GPRS
		set_rtc_state(GOT_TIME_FROM_GPRS);
	}
	else
	{
#ifdef DEBUG_TOT_DATA
		UWriteString("yr_err_tt",UART_PC);
#endif
	}
}

void set_rtc_state(rtc_state_t state)
{
	rtc_state = state;
}

rtc_state_t get_rtc_state()
{
	return rtc_state;
}

void updateLocalDesplay()
{
#if 0
	// display for KWh,KVah,PF
	mylcd_obj.updatePanel1();
	mylcd_obj.bat1();
	mylcd_obj.bat2();
	mylcd_obj.bat3();

	// display for Pressure_data
	mylcd_obj2.updatePanel2();
	mylcd_obj2.bat1();
	mylcd_obj2.bat2();
	mylcd_obj2.bat3();
#endif
}
void getAllCfgData(void)
{
	//unsigned int temp_daily_limit = 0;
	// 21/09/2018 need to take system status.
	//if (!(eepromReadProductInfo()))

	/*if(!(eepromreadSIMConfig()))
	{
		//configRoutines(ERROR_E2P_PRODUCT_SERIAL_NUMBER_INVALID);		//Anand 1.04.2014
		//product_info.product_serial_num = 0x1234;
		memcpy(&sim_info.sim_no, "+919929919936", SIM_NUM_LEN);
		eepromWriteSIMConfig();
#ifdef DEBUG_SPCL
_delay_ms(100);
UWriteString("ERR_SIM_CFG_READ", UART_PC);
#endif
		ram_data.pump_data.system_status |= (1 << SYS_SIM_CONF_ERROR); // 22/09/2018 VC: indicating for system error
		
	}*/
	

/*
	//Lets read config data from eeprom
	if(eepromReadRTUCfgData(&rtu_config))
	{
		flashWriteFreqUpdatedConfig();
		
		write_default_rtu_config(&rtu_config);
		//_delay_ms(5000);
	}
	else
	{
		write_default_rtu_config(&rtu_config);
		ram_data.pump_data.system_status |= (1 << SYS_RTU_CONF_ERROR); // 22/09/2018 VC: indicating for system error
		#ifdef DEBUG_SPCL
		_delay_ms(100);
		UWriteString("ERR_RTU_CFG_READ", UART_PC);
		#endif

	}	
*/

	readFreqUpdData_flash();
	
	if(!(eepromReadProductInfo()))
	{
		product_info.industry_id = 999;
		product_info.daily_limit = 0;
		product_info.daily_limit_dp = 0;
		memcpy((void*)&product_info.ind_name,"E4ENGINEER",INDUS_NAME_LEN);
		eepromWriteProductInfo();
#ifdef _DEBUG_E2P
_delay_ms(100);
UWriteString("ERR_PRODUCT_DATA_READ", UART_PC);
#endif
		//ram_data.pump_data.system_status |= (1 << SYS_PRODUCT_CONF_ERROR);	// 22/09/2018 VC: indicating for system error
	}
	if(product_info.daily_limit_dp >= 100)
	{
		product_info.daily_limit_dp = 0;
		eepromWriteProductInfo();
	}
	//temp_daily_limit = get_daily_limit(product_info.industry_id);
	/*if(temp_daily_limit != product_info.daily_limit)
	{*/
		//product_info.daily_limit = temp_daily_limit;
/*
		eepromWriteProductInfo();
#ifdef DEBUG_DAILY_LIMIT
UWriteString("\ne2p_lim_set:",UART_PC);
#endif
	}*/
#ifdef DEBUG_DAILY_LIMIT
	UWriteString("\nDaily_lim:",UART_PC);
	UWriteInt(product_info.daily_limit,UART_PC);
#endif
	/*if(!(eepromreadGSMConfig()))
	{
	
		//memset(&gsm_cfg_data,0,sizeof(gsm_cfg_data_t));
		//_delay_ms(5000);
		#ifdef DEBUG_TIME
		_delay_ms(100);
		UWriteString("ERR_GSM_CFG_READ", UART_PC);
		#endif
		UWriteString("ERR_GSM_CFG_READ", UART_PC);
	
		//write_default_GSM_config();
		
		
		ram_data.pump_data.system_status |= (1 << SYS_GSM_CONF_ERROR);	// 22/09/2018 VC: indicating for system error
	}*/
	if(!(eepromreadGPRSConfig()))
	{
		memset(&gprs_config_data,0,sizeof(gprs_config_data_t));
		setGPRSConfigErrSts(0);
		write_default_GPRS_config();
		setGPRSConfigErrSts(1);
		#ifdef DEBUG_TIME
		_delay_ms(100);
		UWriteString("ERR_GPRS_DATA_READ", UART_PC);
		#endif
		//ram_data.pump_data.system_status |= (1 << SYS_GPRS_CONF_ERROR);	// 22/09/2018 VC: indicating for system error
	}
	else
	{
		setGPRSConfigErrSts(1);
/*
#ifdef CONNECTION_DEBUG
		UWriteString("PI:",UART_PC);
		UWriteInt((product_info.product_id & 0xFF),UART_PC);
#endif*/
	}
#ifdef ONLINE_CONFIG_EN	
	if(!eepromReadProductInfoConfig())
	{
		product_info_config.time_slot = 2300;
		product_info_config.flow_meter_make = KROHNE_MAR_50;
		product_info_config.connection_type = FM_CONN_ADC420;
		eepromWriteProductInfoConfig();
#ifdef _DEBUG_E2P
		_delay_ms(100);
		UWriteString("ERR_PRODUCT_CFG_DATA_READ", UART_PC);
#endif
	}
#ifdef DEBUG_PRODUCT_COFIGURATION
	else
	{
		UWriteString("t_slot:",UART_PC);
		UWriteInt(product_info_config.time_slot,UART_PC);
		UWriteData(',',UART_PC);
		
		UWriteString("fm_mak:",UART_PC);
		UWriteInt(product_info_config.flow_meter_make,UART_PC);
		UWriteData(',',UART_PC);
		
		UWriteString("contype:",UART_PC);
		UWriteInt(product_info_config.connection_type,UART_PC);
		UWriteData(',',UART_PC);
	}
#endif

	if(!eeprom_read_online_cfg_cnt())
	{
		e2p_online_cnfg.online_cfg_version_count = 1;
		eeprom_write_online_cfg_cnt();
	}
#endif
	if(!readFreqUpdData_flash())
	{
		//freq_updated_data.cnt = 1;
		freq_updated_data.unsent_rtu_logs_GPRS = 0;
		flashClearFreqUpdatedConfig();
	}
	/*if(!(eepromreadGSMConfig()))
	{
	
		//memset(&gsm_cfg_data,0,sizeof(gsm_cfg_data_t));
		//_delay_ms(5000);
		#ifdef DEBUG_TIME
		_delay_ms(100);
		UWriteString("ERR_GSM_CFG_READ", UART_PC);
		#endif
		UWriteString("ERR_GSM_CFG_READ", UART_PC);
	
		//write_default_GSM_config();
		
		
		ram_data.pump_data.system_status |= (1 << SYS_GSM_CONF_ERROR);	// 22/09/2018 VC: indicating for system error
	}*/
	
	if(!(eepromReadCalData()))
	{ // need to update from new struct						

		calbration_data.cal_data_flow1 = 1000;
		calbration_data.cal_data_pressure1 = 1000;
		calbration_data.cal_data_lvl1 = 1000;
		
		eepromWriteCalData();
#ifdef DEBUG_TIME
_delay_ms(100);
UWriteString("ERR_CAL_DATA_READ", UART_PC);
#endif
		//ram_data.pump_data.system_status |= (1 << SYS_CAL_CONF_ERROR);	// 22/09/2018 VC: indicating for system error	
	}

	
	
	/*if(!(eepromReadEmInfo()))
	{
		memset(&em_data,0,sizeof(em_data_t));
		eepromWriteEmInfo();
#ifdef DEBUG_TIME
_delay_ms(100);
UWriteString("ERR_EM_DATA_READ", UART_PC);
UWriteInt((int)em_data.kwh_offset,UART_PC);
#endif
	//	ram_data.pump_data.system_status |= (1 << SYS_EM_CONF_ERROR);	// 22/09/2018 VC: indicating for system error
	}*/
	
}

void update_online_data()
{
	if(rtu_control_data.keep_alive )
	{	
 #ifdef DEBUG_ONLINE_DATA
 UWriteString("aliv:",UART_PC);
 #endif
		//if(getGPRSSrvrSts()  == AVBL)
		if((getGPRSConnSts() == AVBL) && (getRtuState() == RTU_NORMAL))
		{			
			prepare_GPRS_LogData();
		}
	}
}
void init_rtu_ctrl_data()
{
	rtu_control_data.keep_alive = TRUE;
}
void factory_defaults(void)
{
	//if(getTmState()==TM_CONFIG)
	//{
		clear_logs();
		flashClrTransID();
#ifdef FLASH_WP_ENABLE
		remove_block_protection();
#endif
		WREN();
		Chip_Erase();
		Wait_Busy();
		WRDI();				// HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
		WBPR(0);
#endif
		//////////
		writeDefaults();
		//resp[0] = ACK;
	//}
	//else
	//{
		//resp[0] = ERROR_INVALID_SYS_STATE;
	//}
}
RTU_state_t getRtuState( void)
{
	return RTU_state;
}
void  setRtuState( RTU_state_t state)
{
	RTU_state = state;
}
void readButton(void) // VC: 20/08/2018
{
	static int long_press_time = 0;
	static char cs = 1;
//	for( ; digitalReadSwitch(pin); temp++)
	cs = CFG_SW_PIN & (1 << CONFIG_SW);	
	/*if((cs == 0) &&(ls != 0))
	{
		_delay_ms(10);
		if(!(CONFIG_SWITCH_PIN & (1<<CONFIG_SW)))
		{
		}
			
	}*/
	if( cs == 0)
	{
		if(long_press_time++ >= LONG_PRESS_DELAY_CFG)
		{
			long_press_time = 0;
			//GLCD_ClearScreen();
			//GLCD_GoTo(1,0);
			//GLCD_WriteString((char*)"JUMP TO BOOTLOADER");
			//jump_to_bootloader();
			//reset_avr();
			if(getRtuState() == RTU_NORMAL)
			{
				//gprsConfigRoutines();
#ifdef ONLINE_CONFIG_EN
				memset((void*)&online_cfg, 0, sizeof(online_cfg_t));
				online_cfg.cfg_pending = TRUE;
				online_cfg.online_cfg_state = CFG_VER_CNT;
#endif
			}
		}
	}
	else
	{
		long_press_time = 0;
	}
	//ls = cs;


}
void init_lcd(void)
{
#if 0
	// for LCD second
	mylcd_obj.run(LCD_EN_CS1,LCD_EN_WR,LCD_EN_DATA,LCD_EN_LED);
	mylcd_obj.backlightOff();
	mylcd_obj.conf();
	mylcd_obj.clr();	

	// for LCD second
	mylcd_obj2.run(LCD_EN_CS2,LCD_EN_WR,LCD_EN_DATA,LCD_EN_LED);
	mylcd_obj2.backlightOff();
	mylcd_obj2.conf();
	mylcd_obj2.clr();
#endif
}

void convertFloatToChar(char *temp, char *buff, int len)
{
	//char *ptr = (char*)&temp;
	int j = 0;
	
	for(int i = len; i > 0 ; i--)
		buff[j++] = temp[i-1];

}
gprs_conn_status monitorGprsConn()
{
#if 0	
	gprs_conn_status _gprs_conn_status = NORMAL;
	
	if(tcp_data_send_status == GPRS_SOFT_RST_TIME)
	{		
		_gprs_conn_status = SOFT_SHUT;
	}
	else if((tcp_data_send_status > GPRS_SOFT_RST_TIME) && (tcp_data_send_status < GPRS_HARD_RST_TIME))
	{
		_gprs_conn_status =	AFTER_SOFT_SHUT;		
	}
	else if(tcp_data_send_status == GPRS_HARD_RST_TIME) // 10 min
	{		
		_gprs_conn_status = HARD_SHUT;
	}
	else if(tcp_data_send_status > GPRS_HARD_RST_TIME) // after 10 min.
	{
		tcp_data_send_status = 0;
		_gprs_conn_status = AFTER_HARD_SHUT;
	}
#endif	
	return _gprs_conn_status;
}
void update_energy_data(void)
{
	//ram_data.EM_data.KWh = ram_data.EM_data.KWh + em_data.kwh_offset;
	//ram_data.EM_data.KVah = ram_data.EM_data.KVah + em_data.kwah_offset; 
}

void getInstFlow(float val)
{	
	sensor_data.ml_s = 0;
	/*static int indx=0;
	int i;*/
	
	/*if(indx<5)
	{
		sensor_data.avg = (unsigned long) (val*CONVERT_TO_ML);
		indx++;
		
	}
	else*/
	{
		/*for(i=0;i<(NUM_OF_SAMPLES-1);i++)
		{
			sensor_data.sample_1s[NUM_OF_SAMPLES-i-1] = sensor_data.sample_1s[NUM_OF_SAMPLES-i-2];
		}
		sensor_data.sample_1s[0] = (unsigned long)
		sensor_data.sample_1s[0] = (unsigned long) (val*CONVERT_TO_ML);
	
		
		//get avg of raw data in buffer
		for(i=0;i<(NUM_OF_SAMPLES);i++)
		{
			sensor_data.avg += sensor_data.sample_1s[i];
		}*/
		sensor_data.ml_s = (unsigned long) (val*CONVERT_TO_ML);
#ifdef _DEBUG_RS_485
UWriteInt(sensor_data.ml_s,UART_PC);
#endif
		//sensor_data.avg /= NUM_OF_SAMPLES; 
	}
	
		
}

void get_last_y_tot()	//abdul majeed
{
	if(!(e2p_read_y_total()))
	{
		e2p_log_y_tot.y_total = 0xffffffff;
		e2p_write_y_total();
		#ifdef _DEBUG_E2P
		_delay_ms(100);
		UWriteString("ERR_Y_TOTAL_DATA_READ", UART_PC);
		#endif
		//ram_data.pump_data.system_status |= (1 << SYS_PRODUCT_CONF_ERROR);	// 22/09/2018 VC: indicating for system error
	}
}

#if 0
void get_last_time_log(void)
{
#ifdef FLASH_LOG_DATA
	//FL_rtu_log_t temp_data;			//HJ 06-08-2016
	unsigned long addr = 0;

	addr = flashGetCurrDataWriteAddr(RTU) - FL_RTU_LOG_LEN;
#ifdef _DEBUG_FL_ADDR
	UWriteString("fl_addr:",UART_PC);
	UWriteInt(addr,UART_PC);
	UWriteData(':',UART_PC);
	//UWriteBytes((char *)&last_record,sizeof(FL_rtu_log_t),UART_PC);
#endif
	if (addr < FL_RTU_START_ADDR)
	{
		ram_data.cuml_reading = 0;
		ram_data.todays_total = 0;
		ram_data.months_total = 0;
	}
	else
	{
		//cli();
		WREN();
		readContToBuff(addr, FL_RTU_LOG_LEN, (char *)&last_record);
		Wait_Busy();
		WRDI();				   // Write Disable
		//sei();
		
		ram_data.cuml_reading = last_record.ram_data.cuml_reading;
		ram_data.todays_total = last_record.ram_data.todays_total;
		ram_data.months_total = last_record.ram_data.months_total;
		ram_data.time.dd      = last_record.ram_data.time.dd;
		ram_data.time.mm      = last_record.ram_data.time.mm;
		ram_data.time.yr      = last_record.ram_data.time.yr;
#ifdef DEBUG_RTC
		char buff[80];
		sprintf(buff,"ram_time:-%02d/%02d/%02d %02d:%02d:%02d    ",ram_data.time.dd,ram_data.time.mm,ram_data.time.yr,ram_data.time.hr,ram_data.time.min,ram_data.time.sec);
		UWriteString(buff,UART_PC);
#endif
#else	//reading from eeprom
		ram_data.todays_total = e2p_log_data.t_total;
		ram_data.months_total = e2p_log_data.m_total;
		ram_data.cuml_reading = e2p_log_data.c_total;
		memcpy((void *)&last_log,(void *)&e2p_log_data,sizeof(e2p_log_data_t));
		ram_data.time.dd = e2p_log_data.date;
		ram_data.time.mm = e2p_log_data.month;
#endif
	}

}
#endif

void get_last_rtu_log()
{
#ifdef FLASH_LOG_DATA
	//FL_rtu_log_t temp_data;			//HJ 06-08-2016
	unsigned long addr = 0;

	addr = flashGetCurrDataWriteAddr(RTU) - FL_RTU_LOG_LEN;
#ifdef _DEBUG_FL_ADDR
	UWriteString("fl_addr:",UART_PC);
	UWriteInt(addr,UART_PC);
	UWriteData(':',UART_PC);
	//UWriteBytes((char *)&last_record,sizeof(FL_rtu_log_t),UART_PC);
#endif
	if (addr < FL_RTU_START_ADDR)
	{
		ram_data.cuml_reading = 0;
		ram_data.todays_total = 0;
		ram_data.months_total = 0;
	}
	else
	{
	//cli();
		WREN();
		readContToBuff(addr, FL_RTU_LOG_LEN, (char *)&last_record);		
		Wait_Busy();	
		WRDI();				   // Write Disable
		//sei();

		ram_data.cuml_reading = last_record.ram_data.cuml_reading;
		ram_data.todays_total = last_record.ram_data.todays_total;
		ram_data.months_total = last_record.ram_data.months_total;
		
#ifdef DEBUG_RTC
char buff[80];
sprintf(buff,"ram_time:-%02d/%02d/%02d %02d:%02d:%02d    ",ram_data.time.dd,ram_data.time.mm,ram_data.time.yr,ram_data.time.hr,ram_data.time.min,ram_data.time.sec);
UWriteString(buff,UART_PC);
#endif

		check_for_day_month_change_y_tot(&last_record);	//abdul majeed
	
#ifdef _DEBUG_RAM_DATA
		UWriteString("last_record_ram_data:",UART_PC);
		UWriteInt(last_record.ram_data.inst_reading,UART_PC);
		UWriteData(',',UART_PC);
		UWriteInt(last_record.ram_data.todays_total,UART_PC);
		UWriteData(',',UART_PC);
		UWriteInt(last_record.ram_data.cuml_reading,UART_PC);
#endif
		if(ram_data.time.yr >= DEFAULT_YEAR)		//27-04-2021 SK: In case if RTC is currupted then we will not check day/month change until time is updated from gprs
		{
			if(last_record.ram_data.time.mm != ram_data.time.mm)
			{
				ram_data.months_total = 0;
				ram_data.todays_total = 0;
#ifdef DEBUG_TOT_DATA
UWriteString("tot:l_log1",UART_PC);
#endif
			}
			else if (last_record.ram_data.time.dd != ram_data.time.dd)
			{
				ram_data.todays_total = 0;
#ifdef DEBUG_TOT_DATA
UWriteString("tot:l_log2",UART_PC);
#endif
			}
		}
		else //if(ram_data.time.yr < DEFAULT_YEAR)
		{
#ifdef DEBUG_TOT_DATA
			UWriteString("rtc_f:",UART_PC);
#endif
			if(last_record.ram_data.time.yr >= DEFAULT_YEAR)
			{
#ifdef DEBUG_TOT_DATA
UWriteString("get_ll:",UART_PC);
#endif
				/*memcpy((void *)&ram_data.time,(void *)&last_record.ram_data.time,sizeof(time_stamp_t));
				unsigned char temp[8] = {0};
				temp[0] = last_record.ram_data.time.yr; temp[1] = last_record.ram_data.time.mm; temp[2] = last_record.ram_data.time.dd;
				temp[3] = 1;//Day of month
				temp[4] = last_record.ram_data.time.hr; temp[5] = last_record.ram_data.time.min; temp[6] = last_record.ram_data.time.sec;
				memcpy((void *)temp[4],(void *)&last_record.ram_data.time.hr,3);
				set_rtc(temp);*/
			}
		}
	}
#else	//reading from eeprom
	ram_data.todays_total = e2p_log_data.t_total;
	ram_data.months_total = e2p_log_data.m_total;
	ram_data.cuml_reading = e2p_log_data.c_total;
	memcpy((void *)&last_log,(void *)&e2p_log_data,sizeof(e2p_log_data_t));
	ram_data.time.dd = e2p_log_data.date;
	ram_data.time.mm = e2p_log_data.month;
#endif
}

void check_for_day_month_change_y_tot(FL_rtu_log_t *t_data)	//abdul majeed
{
	//FL_rtu_log_t t_data;
	char pre_date, pre_mon, pre_year;
	pre_date = t_data->ram_data.time.dd;
	pre_mon = t_data->ram_data.time.mm;
	pre_year = t_data->ram_data.time.yr;
	get_last_y_tot();
	get_present_time(&ram_data.time);
	if(pre_year>= DEFAULT_YEAR)
	{
		yest_total = e2p_log_y_tot.y_total;
#ifdef DEBUG_TOT_DATA
		UWriteInt(yest_total,UART_PC);
		UWriteString("<Y_LL:",UART_PC);
#endif
	}
	if(ram_data.time.yr >= DEFAULT_YEAR)//27-04-2021 SK: In case if RTC is currupted then we will not check day/month change until time is updated from gprs
	{
		if(pre_year == ram_data.time.yr	&&	pre_mon == ram_data.time.mm	&&	pre_date == ram_data.time.dd)
		{
			yest_total = e2p_log_y_tot.y_total;
#ifdef DEBUG_TOT_DATA
	UWriteInt(yest_total,UART_PC);
	UWriteString("<Y_dsame:",UART_PC);
#endif
		}
		else if(pre_year == ram_data.time.yr	&&	pre_mon == ram_data.time.mm	&&	pre_date != ram_data.time.dd)
		{
			if(pre_date+1 == ram_data.time.dd)
			{
				yest_total = t_data->ram_data.todays_total;
#ifdef DEBUG_TOT_DATA
	UWriteInt(yest_total,UART_PC);
	UWriteString("<Y_dpre:",UART_PC);
#endif
			}
			else
			{
				yest_total = 0xffffffff;
#ifdef DEBUG_TOT_DATA
	UWriteInt(yest_total,UART_PC);
	UWriteString("<Y_2:",UART_PC);
#endif
			}
			prepare_e2p_y_total();
			e2p_write_y_total();
		}
		else if(pre_year == ram_data.time.yr	&&	pre_mon != ram_data.time.mm)
		{
			if(ram_data.time.dd==1	&&	((pre_date ==29 && pre_mon==2 )	||(pre_date ==28 && pre_mon==2 && pre_year%4!=0) ||(pre_date==30	&& (pre_mon==4||pre_mon==6||pre_mon==9||pre_mon==11))	||	(pre_date==31 && (pre_mon==1||pre_mon==3||pre_mon==5||pre_mon==7||pre_mon==8||pre_mon==10||pre_mon==12))))
			{
				yest_total = t_data->ram_data.todays_total;
#ifdef DEBUG_TOT_DATA
UWriteInt(yest_total,UART_PC);
UWriteString("<Y_mpre:",UART_PC);
#endif
			}
			else
			{
				yest_total=0xffffffff;
#ifdef DEBUG_TOT_DATA
	UWriteInt(yest_total,UART_PC);
	UWriteString("<Y_mchange:",UART_PC);
#endif
			}
			prepare_e2p_y_total();
			e2p_write_y_total();
		}
		else  if(pre_year != ram_data.time.yr)
		{
			if(ram_data.time.mm==1	&&	ram_data.time.dd==1	&& ram_data.time.yr==pre_year+1	&&	pre_date ==31 && pre_mon==12)
			{
				yest_total = t_data->ram_data.todays_total;
#ifdef DEBUG_TOT_DATA
	UWriteInt(yest_total,UART_PC);
	UWriteString("<Y_ypre:",UART_PC);
#endif
			}
			else
			{
				yest_total=0xffffffff;
#ifdef DEBUG_TOT_DATA
	UWriteInt(yest_total,UART_PC);
	UWriteString("<Y_ychange:",UART_PC);
#endif
			}
			prepare_e2p_y_total();
			e2p_write_y_total();
		}
		else
		{
			yest_total = 0xffffffff;
			prepare_e2p_y_total();
			e2p_write_y_total();
		}
	}
	else
	{
#ifdef DEBUG_TOT_DATA
		UWriteString("y_err_yt",UART_PC);
#endif
	}
	
}

void check_for_MOTOR_POWER(void)//AM 19-07-21
{
	static power_status_t mot_pow_detect = PD_DETECT;
	static unsigned char mot_down_cnt = 0;
	switch(mot_pow_detect)
	{
		case PD_DETECT:
			if(!(MOTOR_AC_PIN & (1<<MOTOR_AC)))
			{
#ifdef FM_AC_DEBUG
				UWriteString("M_PWON",UART_PC);
#endif
				ram_data.status |= (1 << MOTOR_ON);
				mot_down_cnt = 0;
			}
			else
			{	
				mot_down_cnt++;
				if(mot_down_cnt>10)
					mot_pow_detect = PD_DOWN;
			}
			break;
		case PD_DOWN:
#ifdef FM_AC_DEBUG
			UWriteString("M_PWDN",UART_PC);
#endif
			ram_data.status &= ~(1 << MOTOR_ON);
			mot_down_cnt = 0;
			mot_pow_detect = PD_DETECT;
			break;
	}
}

#ifdef SUPERCAP
unsigned char check_mains_cut(void)
{
	if(ram_data.status & (1<<MAIN_CUTOFF))
		return TRUE;
	else
		return FALSE;
		
}
#endif
void save_mains_off_logs(void)
{
#ifdef FLASH_LOG_DATA
	prepare_FL_LogData();
	save_FL_rtu_data();
	
#ifdef _DEBUG_UNSENT_LOGS
	UWriteString((char*)"unsent:", UART_PC);		//Debug
	UWriteInt(freq_updated_data.unsent_rtu_logs_GPRS,UART_PC);
#endif
#else
	prepare_e2p_logdata();
	e2p_write_log_data();
#endif
	
#ifdef _DEBUG_MAIN_CUTOFF
	UWriteString("Main_OFF",UART_PC);
#endif
}

void check_for_power_down(void)
{
	static power_status_t power_status = PD_DETECT;
	static unsigned char pd_cnt = 0;
	switch(power_status)
	{
		case PD_DETECT:
			
			if(!(POWER_DOWN_PIN & (1<<POWER_DOWN)))
			{
				if(pd_cnt++>= 3)
				{
					
					ram_data.status |= (1 << MAIN_CUTOFF);
#ifdef FLASH_LOG_DATA
					prepare_FL_LogData();
					save_FL_rtu_data();

					//++freq_updated_data.unsent_rtu_logs_GPRS;//no need in case of super cap
					//flashWriteFreqUpdatedConfig();
#ifdef _DEBUG_UNSENT_LOGS
					UWriteString((char*)"unsent:", UART_PC);		//Debug
					UWriteInt(freq_updated_data.unsent_rtu_logs_GPRS,UART_PC);
#endif
#else
					prepare_e2p_logdata();
					e2p_write_log_data();
#endif

#ifdef _DEBUG_MAIN_CUTOFF
					UWriteString("Main_OFF",UART_PC);
#endif
					pd_cnt = 0;
					power_status = PD_DOWN;
				}
			}
			else
			{
				pd_cnt = 0;
				ram_data.status &= ~(1 << MAIN_CUTOFF);
			}
		break;
		case PD_DOWN:
			if((POWER_DOWN_PIN & (1<<POWER_DOWN)))
			{
				if(pd_cnt++>= 3)
				{
					ram_data.status &= ~(1 << MAIN_CUTOFF);
#ifdef	DEBUG_TOT_DATA
UWriteString("c_data:",UART_PC);
UWriteInt(ram_data.cuml_reading,UART_PC);
#endif
					power_status = PD_DETECT;
#ifdef _DEBUG_MAIN_CUTOFF
UWriteString("Main_ON",UART_PC);
#endif
				}
			}
			else
			{
				//Still No mains
				ram_data.status |= (1 << MAIN_CUTOFF);
				pd_cnt =0;
			}
		break;
		
	}
	
}

void sw_led_cntrl(void)
{
	static unsigned int time = 0;
	static unsigned char cs = 1, ls = 1;
	
	cs = SW_LED_PIN & 0xF0;
	
	if (((cs & (1 << SW_LED_SW1)) == 0) && (((ls & (1 << SW_LED_SW1)) >> SW_LED_SW1) == 1))
	{
		//resetSystemIdleTime();
		/*display_pwr_cntrl(HIGH);
		set_display_state(TRUE);
		re_init_lcd();*/
		if (getRtuState() == RTU_FRMWR_UPDATE)
		{
			if (get_frmwr_update_state() == HEX_FILE_UPDATE_CHK)
			{
				if (get_hex_file_updt_state() == RESUME_PENDING)
				{
					set_frmwr_choice(1);
				}
			}
		}
	}
	if (((cs & (1 << SW_LED_SW2)) == 0) && (((ls & (1 << SW_LED_SW2)) >> SW_LED_SW2) == 1))
	{
		_delay_ms(50);
		if (((SW_LED_PIN & (1 << SW_LED_SW2)) == 0))
		{
			//resetSystemIdleTime();
			//display_pwr_cntrl(HIGH);
			//set_display_state(TRUE);
			//re_init_lcd();
			#ifdef DEBUG_BOOT_HEX
			//UWriteString("SW0 Press", UART_PC);
			#endif
			time = 0;
			if ((getRtuState() == RTU_FRMWR_UPDATE) && (get_frmwr_update_state() == HEX_FILE_UPDATE_CHK))
			{
				if (get_hex_file_updt_state() == CNFRM_DWNLD)
				{
					set_frmwr_choice(0);
				}
			}
		}
	}
	else
	{
		if (((cs & (1 << SW_LED_SW2)) == 0) && ((ls & (1 << SW_LED_SW2)) == 0))
		{
			time++;
			if (time >= 100)
			{
				#ifdef DEBUG_BOOT_HEX
				//UWriteString("SW Done", UART_PC);
				#endif
				if (getRtuState() == RTU_NORMAL /*&& (!check_unsent_log())*/)
				{
					time = 0;
					if (getGPRSConnSts() == AVBL)
					{
						//re_init_lcd();
						set_hex_file_updt_mode(HEX_FILE_UPDT_MANUAL);
						hexFileUpdateRoutines();
					}
				}
			}
		}
	}
	if (((cs & (1 << SW_LED_SW3)) == 0) && (((ls & (1 << SW_LED_SW3)) >> SW_LED_SW3) == 1))
	{
		//resetSystemIdleTime();
		//display_pwr_cntrl(HIGH);
		//set_display_state(TRUE);
		//re_init_lcd();
		if (getRtuState() == RTU_FRMWR_UPDATE)
		{
			if (get_frmwr_update_state() == HEX_FILE_UPDATE_CHK)
			{
				if (get_hex_file_updt_state() == RESUME_PENDING)
				{
					set_frmwr_choice(2);
				}
				else if (get_hex_file_updt_state() == CNFRM_DWNLD)
				{
					setRtuState(RTU_NORMAL);
				}
			}
		}
#ifdef DEBUG_WDT
		else
		{
			char buff[21],i = 1;
			GLCD_ClearScreen();
			GLCD_GoTo(0,4);
			GLCD_WriteString((char*)"      SYS HANG      ");
			
			
			
			
			while(1)
			{
				_delay_ms(1000);
				GLCD_GoTo(0,5);
				sprintf(buff,"     %2d Seconds     ",i);
				GLCD_WriteString(buff);
				i++;
			}
		}
#endif
	}
	
	ls = cs;
}
void checkForAlert()
{
	
}

void updt_sys_config(void)
{
#if BOARD_VER == v1_0
	rtu_data.rtu_config &= ~(1 << BOARD_VER_BIT);
#endif
#if BOARD_VER == v2_0
	rtu_data.rtu_config |= (1 << BOARD_VER_BIT);
#endif

#ifdef TESTING_MODE
	rtu_data.rtu_config |= (1 << TESTING_MODE_BIT);
#endif
}
unsigned int get_rtu_cnfg(void)
{
	return rtu_data.rtu_config;
}
void manage_firmware_update(void)
{
	static unsigned int display_time = 0;
	switch(manage_frmwr_updt.frmwr_update_state)
	{
		case HEX_FILE_UPDATE_CHK:
			manage_hex_file_update();
			flash_hex_file_progress();
		break;
		case HEX_FILE_UPDATE_PRG:
			//flash_file_update_routines();
			flash_hex_file_progress();
		break;
		case HEX_FILE_UPDATE_VERIFY:
			flash_hex_file_progress();
			flash_hex_file_verify();
		break;
		case HEX_FILE_UPDATE_RELEASE:
			flash_hex_file_progress();
			if(++display_time >= DISPLAY_HOLD_TIME)
			{
				//release_hex_file_states();
				//set_frmwr_update_state(HEX_FILE_UPDATE_CHK);
				display_time = 0;
				//set_au_state(AU_NORMAL);
				setRtuState(RTU_NORMAL);
			}
		break;
		default:
		break;
	}
}
void set_frmwr_update_state(frmwr_update_state_t state)
{
	manage_frmwr_updt.frmwr_update_state = state;
}
frmwr_update_state_t get_frmwr_update_state(void)
{
	return manage_frmwr_updt.frmwr_update_state;
}
unsigned char get_frmwr_choice(void)
{
	return manage_frmwr_updt.choice;
}
void set_frmwr_choice(unsigned char data)
{
	manage_frmwr_updt.choice = data;
}

void controlRTU(void)
{
	switch(getRtuState())
	{
		case RTU_FRMWR_UPDATE:
		{
			manage_firmware_update();
		}
		break;
		case RTU_NORMAL:
		{
			alarm_handler();
			motor_handler();
		}
		break;
		default:
		break;
	}
}

void alarm_handler()
{
	static unsigned int wait_time = 0;
	switch(alarm_state)
	{
		case ALARM_OFF:
		{
			ALARM_PORT &= ~(1 << ALARM);		//alarm off
			//if(ram_data.todays_total >= ((unsigned long)product_info.daily_limit*1000*LIMIT_TH/100))
			if(check_alarm())
			{
				alarm_state = ALARM_ON;
#ifdef DEBUG_IO
UWriteString("off_alarm_on:",UART_PC);
#endif
			}
		}
		break;
		
		case ALARM_ON:
		{
			ALARM_PORT |= (1 << ALARM);		//alarm on
			if(!check_alarm())
			{
				alarm_state = ALARM_OFF;
#ifdef DEBUG_IO
UWriteString("on_alm_off:",UART_PC);
#endif
			}
			else if(SNOOZE_SW_PIN & (1 << SNOOZE_SW))		//check snooze off button
			{
				if(wait_time++ >= 500/100)
				{
					wait_time = 0;
					alarm_state = ALARM_SNOOZE;
#ifdef DEBUG_IO
UWriteString("snz_sw_snooze:",UART_PC);
#endif
				}
			}
			else
			{
				wait_time = 0;
			}
			
		}
		break;
		
		case ALARM_SNOOZE:
		{
			ALARM_PORT &= ~(1 << ALARM);		//alarm off
			//if(ram_data.todays_total >= ((unsigned long)product_info.daily_limit*1000*LIMIT_TH/100))
			if(check_alarm())
			{
				if(++wait_time >= ((unsigned long)SNOOZE_TIME))
				{
#ifdef DEBUG_IO
UWriteString("snz_alarm_on:",UART_PC);
#endif
					wait_time = 0;
					alarm_state = ALARM_ON;
				}
			}
			else
			{
				wait_time = 0;
				alarm_state = ALARM_OFF;
#ifdef DEBUG_IO
UWriteString("snz_alarm_off:",UART_PC);
#endif
			}
		}
		break;
	}
}

void motor_handler()
{
	unsigned long daily_lim = ((unsigned long)product_info.daily_limit*1000 + product_info.daily_limit_dp*10);
	//if(ram_data.todays_total >= (unsigned long)product_info.daily_limit*1000)
	//if(ram_data.todays_total >= daily_lim)
	if((ram_data.todays_total >= (daily_lim*MOTOR_OFF_PERC/100)))
	{
		MOTOR_PORT &= ~(1 << MOTOR);	//motor off
#ifdef DEBUG_IO
	UWriteString("motor_off:",UART_PC);
#endif
	}
	else
	{
		MOTOR_PORT |= (1 << MOTOR);		//motor on
	}
}

void check_sms_threshold()
{
	unsigned long daily_lim = ((unsigned long)product_info.daily_limit*1000 + product_info.daily_limit_dp*10);
	//if(ram_data.todays_total >= (unsigned long)product_info.daily_limit*1000)
		switch(sms_alert_state)
		{
			case CHECK_ALERT:
			{
#ifdef DEBUG_SMS
				UWriteString("check_alert:",UART_PC);
#endif
				//if(	(ram_data.todays_total >= ((unsigned long)product_info.daily_limit*1000)) )
				if(	(ram_data.todays_total >= (daily_lim)))
				{
					sms_alert_state = SMS_ALERT_100;
				}
				//else if((ram_data.todays_total >= ((unsigned long)product_info.daily_limit*1000*LIMIT_TH/100)) && (ram_data.inst_reading > (MIN_FLOW_RATE*1000)))
				else if((ram_data.todays_total >= (daily_lim*LIMIT_TH/100)) && (ram_data.inst_reading > (MIN_FLOW_RATE*1000)))
				{
					sms_alert_state = SMS_ALERT_90;
				}
			}
			break;
			case SMS_ALERT_90:
			{
				db_save_status = TRUE;
#ifdef DEBUG_SMS
				UWriteString("ALERT_90:",UART_PC);
#endif
				sms_alert_state = SMS_ALERT_100;
			}
			break;
			case SMS_ALERT_100:
			{
				//if(	(ram_data.todays_total >= ((unsigned long)product_info.daily_limit*1000)) )
				if(	(ram_data.todays_total >= (daily_lim)))
				{
					db_save_status = TRUE;
					sms_alert_state = NO_ALERT;
#ifdef DEBUG_SMS
					UWriteString("ALERT_100:",UART_PC);
#endif
				}
			}
			break;
			case NO_ALERT:
			{
#ifdef DEBUG_SMS
					UWriteString("no_sms_alert:",UART_PC);
#endif			
			}
			break;
			default:
			break;
		}
}

unsigned char check_alarm()
{
	unsigned long daily_lim = ((unsigned long)product_info.daily_limit*1000 + product_info.daily_limit_dp*10);
	//if((ram_data.todays_total >= ((unsigned long)product_info.daily_limit*1000*LIMIT_TH/100)) && (ram_data.inst_reading > (MIN_FLOW_RATE*1000)))
	if((ram_data.todays_total >= (daily_lim*LIMIT_TH/100)) && (ram_data.inst_reading > (MIN_FLOW_RATE*1000)))
	{
		return TRUE;
	}
	else 
	{
		return FALSE;
	}
}

void set_dummy_values()
{
	product_info.daily_limit = 25;
}

void clear_ram_logs(void)
{
	ram_data.inst_reading = 0;
	ram_data.todays_total = 0;
	ram_data.months_total = 0;
	ram_data.cuml_reading = 0;
	ram_data.status = 0;
}

void online_firmware_download(void)
{
	static unsigned int firmware_dwld_time = 0;
	
	if(++firmware_dwld_time >= FIRMWARE_DWNLD_TIME)
	{
		if ((getRtuState() == RTU_NORMAL) && (!check_unsent_log()))
		{
			if (getGPRSConnSts() == AVBL)
			{
				firmware_dwld_time = 0;
				//re_init_lcd();
				set_hex_file_updt_mode(HEX_FILE_UPDT_AUTO);
				hexFileUpdateRoutines();
			}
		}
	}
	
}

void reset_avr()
{
	
	/*asm volatile(
	"clr	r30		\n\t"
	"clr	r31		\n\t"
	"ijmp	\n\t"
	);*/
	//soft_reset();
//	wdt_enable(WDTO_8S);
	cli();
	wdt_reset();
	WDTCSR |= (1<<WDE)|(1<<WDCE);
	WDTCSR = /*(1<<WDIE)|*/(1<<WDE)|(1<<WDP2)|(1<<WDP1)|(1<<WDP0);
	sei();
	while(1)
	{
		SYS_ERR_LED_PORT ^= (1 << SYS_ERR_LED);
		
		_delay_ms(1000);
	}
}

/*
ISR(WDT_vect)
{
	UWriteString("Bye",UART_PC);
}*/

void jump_to_bootloader()
{
#ifdef DEBUG_HEX_FILE_STORE
UWriteString("jump to bootloader",UART_PC);
#endif
	asm volatile("jmp 0x1F000");
}

void WDT_off(void)
{
	cli();
	wdt_reset();
	/* Clear WDRF in MCUSR */
	MCUSR &= ~(1<<WDRF);
	/* Write logical one to WDCE and WDE *//* Keep old prescaler setting to prevent unintentional time-out */
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	/* Turn off WDT */
	WDTCSR = 0x00;
	sei();
}

