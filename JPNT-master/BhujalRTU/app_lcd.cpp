#include "graphic_lcd_ks0108.h"
#include "main.h"
#include "app_uart.h"
#include "app_lcd.h"
#include "sys_info.h"
#include <stdlib.h>
#include <string.h>
#include "gprs_config.h"
#include "flash.h"
#include "app_eeprom.h"
#include "_debug.h"

extern ram_data_t ram_data;
extern product_info_t product_info;
extern flash_hexfile_t flash_hexfile;

//extern e2p_log_y_tot_t e2p_log_y_tot;
extern unsigned long int yest_total;	//abdul majeed

#ifdef DEBUG_HEX_FILE_STORE
	extern unsigned char dyn_mem_alloc_cnt;
	extern unsigned char dyn_mem_dealloc_cnt;
#endif
void welcome_screen()				//25-02-2019 SK
{
	unsigned int i;
	GLCD_ClearScreen();
	GLCD_GoTo(1,0);
	GLCD_WriteString((char*)"=====================");
	GLCD_GoTo(40,2);
	GLCD_WriteString((char*)PROJECT_NAME);
	GLCD_GoTo(50,4);
	//GLCD_WriteString((char*)VERSION_NUM);
	GLCD_WriteString((char*)FIRMWARE_VER);
	GLCD_GoTo(0,6);
	GLCD_WriteString((char*)E4E);
	GLCD_GoTo(1,7);

	GLCD_WriteString((char*)"=====================");

	for(i = 1 ; i <= 6 ; i++)
	{
		GLCD_GoTo(0,i);
		GLCD_WriteChar('|');
		GLCD_GoTo(122,i);
		GLCD_WriteChar('|');
	}
	
}

void config_screen(void)
{
	GLCD_ClearScreen();
	GLCD_GoTo(1,0);
	GLCD_WriteString((char*)CONFIG);
	
}
void data_screen(void)
{
	
	char tmpstr[25];
	static char flg = 0;
	memset(tmpstr,' ',sizeof(tmpstr));
	
	unsigned int comp_id = product_info.industry_id;
	char status_icon = 'X';
	char supply_state = 'M';
	
	if(ram_data.status & (1 << MAIN_CUTOFF))
	{
		supply_state = 'B';
	}
	if (getGPRSConnSts() == AVBL)
	{
		if (check_unsent_log())
		{
			if (flg)
			{
				status_icon = ' ';
				flg = 0;
			}
			else
			{
				status_icon = '^';
				flg = 1;
			}
		}
		else
		{
			status_icon = 'G';
		}
	}
	/*else if()
	{
		
	}*/
	else
	{
		status_icon = 'X';
	}
	GLCD_GoTo(0,0);
	//sprintf(tmpstr,"%02d/%02d/%02d %02d:%02d:%02d    ",ram_data.time.dd,ram_data.time.mm,ram_data.time.yr,ram_data.time.hr,ram_data.time.min,ram_data.time.sec);
	sprintf(tmpstr,"%02d/%02d/%02d %02d:%02d:%02d %c %c",ram_data.time.dd,ram_data.time.mm,ram_data.time.yr,ram_data.time.hr,ram_data.time.min,ram_data.time.sec,status_icon, supply_state);		//03-05-2021 SK: this is new screen in which gprs and main battery state is shown in first line
	GLCD_WriteString(tmpstr);
	memset(&tmpstr,' ',sizeof(tmpstr));
	
	GLCD_GoTo(0,1);
	GLCD_WriteString((char*)PARTION_LINE);
	
	GLCD_GoTo(0,2);
	//sprintf(tmpstr,"%s %7u.00 %s",LIMIT,product_info.daily_limit,KL);
	sprintf(tmpstr,"%s %7u.%02u %s",LIMIT,product_info.daily_limit,product_info.daily_limit_dp%100,KL);
	GLCD_WriteString(tmpstr);
	memset(&tmpstr,' ',sizeof(tmpstr));
	
	GLCD_GoTo(0,3);
	sprintf(tmpstr,"%s %7ld.%02ld %s",TODAY_TOTAL,ram_data.todays_total/1000,(ram_data.todays_total%1000)/10,KL);
	GLCD_WriteString(tmpstr);
	memset(&tmpstr,' ',sizeof(tmpstr));
	
	GLCD_GoTo(0,4);
#if 0
	sprintf(tmpstr,"%s %7ld.%02ld %s",MONTHLY_TOTAL,ram_data.months_total/1000,(ram_data.months_total%1000)/10,KL);
#endif
	if(yest_total==0xffffffff)
		sprintf(tmpstr,"%s %s %s",Y_TOTAL,NA,KL);
	else
		sprintf(tmpstr,"%s %7ld.%02ld %s",Y_TOTAL,yest_total/1000,(yest_total%1000)/10,KL);
	
	GLCD_WriteString(tmpstr);
	memset(&tmpstr,' ',sizeof(tmpstr));

	GLCD_GoTo(0,5);
	sprintf(tmpstr,"%s %7ld.%02ld %s",COM_TOTAL,(ram_data.cuml_reading/1000)%10000000,(ram_data.cuml_reading%1000)/10,KL);
	GLCD_WriteString(tmpstr);
	
	memset(&tmpstr,' ',sizeof(tmpstr));
	
	GLCD_GoTo(0,6);
#ifdef DEBUG_HEX_FILE_STORE
	char buff[20];
	sprintf(buff,"%u,%u",dyn_mem_alloc_cnt,dyn_mem_dealloc_cnt);
	GLCD_WriteString(buff);
#else
	GLCD_WriteString((char*)PARTION_LINE);
#endif
	GLCD_GoTo(0,7);
	//sprintf(tmpstr,"%c %c ", status_icon, supply_state);
	sprintf(tmpstr,"%s ",FIRMWARE_VER);			//03-05-21 SK: firmware is shown on home screen
	GLCD_WriteString(tmpstr);
	memset(tmpstr,0,sizeof(tmpstr));
	//sprintf(tmpstr,"%4lu.%01lu KL/Hr", ram_data.inst_reading/1000, (ram_data.inst_reading%1000)/100);
	sprintf(tmpstr,"%3lu.%01lu KL/Hr", ram_data.inst_reading/1000, (ram_data.inst_reading%1000)/100);	//03-05-2021 SK: inst flow display changed to 3+1
	GLCD_WriteString_inv(tmpstr);
	memset(tmpstr,0,sizeof(tmpstr));
	sprintf(tmpstr,"#%03u", comp_id);
	GLCD_WriteString(tmpstr);
	memset(tmpstr,0,sizeof(tmpstr));
	
	memset(&tmpstr,' ',sizeof(tmpstr));
#ifdef DEBUG_UPDATE_LCD
	memset(tmpstr,0,sizeof(tmpstr));
	sprintf(tmpstr,"\n%02d/%02d/%02d %02d:%02d:%02d    ",ram_data.time.yr,ram_data.time.mm,ram_data.time.dd,ram_data.time.hr,ram_data.time.min,ram_data.time.sec);
	UWriteString(tmpstr,UART_PC);
	
	memset(tmpstr,0,sizeof(tmpstr));
	//sprintf(tmpstr,"%s %7u.00 %s",LIMIT,product_info.daily_limit,KL);
	sprintf(tmpstr,"\n%s %7u.%02u %s",LIMIT,product_info.daily_limit,product_info.daily_limit_dp%100,KL);
	UWriteString(tmpstr,UART_PC);
	
	memset(tmpstr,0,sizeof(tmpstr));
	sprintf(tmpstr,"\n%4lu.%01lu KL/Hr", ram_data.inst_reading/1000, (ram_data.inst_reading%1000)/10);
	UWriteString(tmpstr,UART_PC);
	
	memset(tmpstr,0,sizeof(tmpstr));
	sprintf(tmpstr,"\n%s %7ld.%02ld %s",TODAY_TOTAL,ram_data.todays_total/1000,(ram_data.todays_total%1000)/10,KL);
	UWriteString(tmpstr,UART_PC);
	
	memset(tmpstr,0,sizeof(tmpstr));
	sprintf(tmpstr,"\n%s %7ld.%02ld %s",MONTHLY_TOTAL,ram_data.months_total/1000,(ram_data.months_total%1000)/10,KL);
	UWriteString(tmpstr,UART_PC);
	
	memset(tmpstr,0,sizeof(tmpstr));
	sprintf(tmpstr,"\n%s %7ld.%02ld %s",COM_TOTAL,(ram_data.cuml_reading/1000)%10000000,(ram_data.cuml_reading%1000)/10,KL);
	UWriteString(tmpstr,UART_PC);
	
#endif
}

void hexFileUpdateRoutines(void)
{
#ifdef DEBUG_HEX_FILE_STORE
UWriteString("hex_upd_strt:",UART_PC);
#endif
	//char buff_for_lcd[LCD_MAXCOLS + 1] = {' '};
	#ifdef _20x4_LCD
	lcd_clear();
	#endif
	//clr_lcd_buff();											// reset the buff of the lcd
	//set_lcd_buff_sts(TRUE);								// requried for printing in lcd

	//set_au_state(AU_FRMWR_UPDATE);						// change the mode of operation
	GLCD_ClearScreen();
	setRtuState(RTU_FRMWR_UPDATE);
	set_frmwr_update_state(HEX_FILE_UPDATE_CHK);
	set_hex_file_updt_state(HEX_FILE_INIT);
	set_flash_file_rqst_type(HEX_NO_RQST);
	#ifdef DEBUG_GPRS_SERVER_SYNC
	UWriteString((char*)"config_mode:",UART_PC);
	#endif
}

void manage_hex_file_update(void)
{
	switch(get_hex_file_updt_state())
	{
		case HEX_FILE_INIT:
			GLCD_ClearScreen();
			GLCD_GoTo(0,1);
			GLCD_WriteString((char*)"CHECKING PENDING");
			GLCD_GoTo(0,2);
			GLCD_WriteString((char*)"DOWNLOAD");
			if(check_pending_hex_data())
			{
				//24-08-2020 SK: This is the case where we have pending file
#ifdef _20x4_LCD
				lcd_clear();
#endif
				//clr_lcd_buff();
				GLCD_ClearScreen();
				
				//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
				GLCD_GoTo(0,1);
				GLCD_WriteString((char*)"1. RESUME       ");
				
				GLCD_GoTo(0,2);
				GLCD_WriteString((char*)"2. RESTART      ");
				//copyStrToLcdBuff(hex_file_resume_line, 1);
				//copyStrToLcdBuff(hex_file_restart_line, 2);
				set_hex_file_updt_state(RESUME_PENDING);
				if(get_hex_file_updt_mode() == HEX_FILE_UPDT_AUTO)
				{
					set_frmwr_choice(1);		//12-04-21 SK: forcefully resume firmware download
				}
			}
			else
			{
				//24-08-2020 SK: This is the case where we have complete file
#ifdef _20x4_LCD
				lcd_clear();
#endif
				//clr_lcd_buff();
				
				GLCD_ClearScreen();
				
				//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
				GLCD_GoTo(0,1);
				GLCD_WriteString((char*)"DOWNLOAD UPDATE?");
				
				GLCD_GoTo(0,2);
				GLCD_WriteString((char*)" ENTER      ESC ");
				//copyStrToLcdBuff(cnfrm_dwnload_line1, 1);
				//copyStrToLcdBuff(power_key_lcd_line2, 2);
				set_hex_file_updt_state(CNFRM_DWNLD);
				if(get_hex_file_updt_mode() == HEX_FILE_UPDT_AUTO)
				{
					#ifdef DEBUG_HEX_FILE_STORE
					UWriteString("set_fw_ch0:",UART_PC);
					#endif	
					set_frmwr_choice(0);		//12-04-21 SK: forcefully check for new update
				}
			}
			//set_lcd_buff_sts(TRUE);
		break;
		case CNFRM_DWNLD:
		{
			//download fresh file
			if (get_frmwr_choice() == 0)
			{
				#ifdef DEBUG_HEX_FILE_STORE
				UWriteString("cnf_dwn:",UART_PC);
				#endif
				//if(allot_extra_mem())
				{
					flash_hexfile.addr = FL_HEX_FILE_START_ADDR;
					set_frmwr_choice(0xFF);
					set_hex_file_updt_state(CHK_UPDT);
					set_flash_file_rqst_type(HEX_CHK_FRMWR_UPDT);
					//flashClrHexFile();	//22-06-21 SK: it should not done here. if new firmware is not available then there is no need to clear the file.
					//clr_lcd_buff();
					
					GLCD_ClearScreen();
					
					//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
					GLCD_GoTo(0,1);
					GLCD_WriteString((char*)"    CHECKING    ");
					//copyStrToLcdBuff(hex_file_init_update_line1, 1);
					set_frmwr_update_state(HEX_FILE_UPDATE_PRG);
					//set_lcd_buff_sts(TRUE);
				}
				/*else
				{
					//write code to handle if dynamic memory is not created.
					set_hex_file_updt_state(UPDATE_FAIL);
				}*/
			}
		}
		break;
		case RESUME_PENDING:
		{
			if(get_frmwr_choice() == 1)
			{
				#ifdef DEBUG_HEX_FILE_STORE
				UWriteString("resum:",UART_PC);
				#endif
				//Resume download
				//if(allot_extra_mem())
				{
					set_frmwr_choice(0xFF);
					set_hex_file_updt_state(DOWNLOADING);
					set_flash_file_rqst_type(HEX_RQST_NEW_REC);
					eepromReadHexFileHdr();	//lets restore the last pending hex file name from eeprom.
					//clr_lcd_buff();
					GLCD_ClearScreen();
					
					//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
					GLCD_GoTo(0,1);
					GLCD_WriteString((char*)"   DOWNLOADING  ");
					//copyStrToLcdBuff(hex_file_update_line2, 1);
					set_frmwr_update_state(HEX_FILE_UPDATE_PRG);
					//set_lcd_buff_sts(TRUE);
				}
				/*else
				{
					//write code to handle if dynamic memory is not created.
					set_hex_file_updt_state(UPDATE_FAIL);
				}*/
			}
			else if(get_frmwr_choice() == 2)
			{
				#ifdef DEBUG_HEX_FILE_STORE
				UWriteString("rstrt:",UART_PC);
				#endif
				//Restart download
				//if (allot_extra_mem())
				{
					flash_hexfile.addr = FL_HEX_FILE_START_ADDR;
					set_frmwr_choice(0xFF);
					set_hex_file_updt_state(CHK_UPDT);
					set_flash_file_rqst_type(HEX_CHK_FRMWR_UPDT);
					//flashClrHexFile();	//22-06-21 SK: it should not done here. if new firmware is not available then there is no need to clear the file.
					hex_file_ready(FALSE);
					//clr_lcd_buff();
					GLCD_ClearScreen();
					
					//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
					GLCD_GoTo(0,1);
					GLCD_WriteString((char*)"    CHECKING    ");
					//copyStrToLcdBuff(hex_file_init_update_line1, 1);
					set_frmwr_update_state(HEX_FILE_UPDATE_PRG);
					//set_lcd_buff_sts(TRUE);
				}
				/*else
				{
					//write code to handle if dynamic memory is not created.
					set_hex_file_updt_state(UPDATE_FAIL);
				}*/
			}
		}
		break;
		default:
		break;
	}
}
void flash_hex_file_progress(void)
{
	static unsigned int time = 0;
	static unsigned char blink_flag = TRUE;
	switch(get_hex_file_updt_state())
	{
		case CHK_UPDT:
		{
			if(++time >= BLINK_TIME)
			{
				time = 0;
				
				//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
				
				if(blink_flag)
				{
					blink_flag = FALSE;
					GLCD_GoTo(0,1);
					GLCD_WriteString((char*)"    CHECKING    ");
					//copyStrToLcdBuff(hex_file_init_update_line1, 1);
				}
				else
				{
					blink_flag = TRUE;
					GLCD_GoTo(0,1);
					GLCD_WriteString((char*)"                ");
					//copyStrToLcdBuff(empty_string, 1);
				}
				//set_lcd_buff_sts(TRUE);
			}
		}
		break;
		case DWNLD_CMPLT:
			//clr_lcd_buff();
			GLCD_ClearScreen();
			
			//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
			GLCD_GoTo(0,1);
			GLCD_WriteString((char*)" DOWNLOAD  DONE ");
			//copyStrToLcdBuff(download_complete_line, 1);
			//set_lcd_buff_sts(TRUE);
			set_hex_file_updt_state(DISPLAY_HOLD);
			reset_avr();		//15-04-21 SK: lets reset the controller when the downloading is complete
			//jump_to_bootloader();
			//release_hex_file_states();
			//display_time = 0;
		break;
		case FILE_NOT_FOUND:
			//clr_lcd_buff();
			GLCD_ClearScreen();
			
			//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
			GLCD_GoTo(0,1);
			GLCD_WriteString((char*)" FILE NOT FOUND ");
			//copyStrToLcdBuff(file_not_found_line, 1);
			//set_lcd_buff_sts(TRUE);
			set_hex_file_updt_state(DISPLAY_HOLD);
			//release_hex_file_states();
			//display_time = 0;
		break;
		case CONN_ERROR:
			//clr_lcd_buff();
			GLCD_ClearScreen();
			
			//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
			GLCD_GoTo(0,1);
			GLCD_WriteString((char*)"CONNECTION ERROR");
			//copyStrToLcdBuff(conn_error_line, 1);
			//set_lcd_buff_sts(TRUE);
			set_hex_file_updt_state(DISPLAY_HOLD);
			//release_hex_file_states();
			//display_time = 0;
		break;
		case DOWNLOADING:
			if(++time >= BLINK_TIME)
			{
				time = 0;
				//GLCD_ClearScreen();
				
				//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
				
				if(blink_flag)
				{
					blink_flag = FALSE;
					GLCD_GoTo(0,1);
					GLCD_WriteString((char*)"   DOWNLOADING  ");
					//copyStrToLcdBuff(hex_file_update_line2, 1);
				}
				else
				{
					blink_flag = TRUE;
					GLCD_GoTo(0,1);
					GLCD_WriteString((char*)"                ");
					//copyStrToLcdBuff(empty_string, 1);
				}
				//set_lcd_buff_sts(TRUE);
			}
		break;
		case NO_UPDATE:
			//clr_lcd_buff();
			
			GLCD_ClearScreen();
			
			//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
			GLCD_GoTo(0,1);
			GLCD_WriteString((char*)"   NO  UPDATE   ");
			
			//sprintf(buff,"%u,%u",dyn_mem_alloc_cnt,dyn_mem_dealloc_cnt);
			//GLCD_GoTo(0,2);
			//GLCD_WriteString(buff);
			//copyStrToLcdBuff(no_update_line, 1);
			//set_lcd_buff_sts(TRUE);
			set_hex_file_updt_state(DISPLAY_HOLD);
			//release_hex_file_states();
			//display_time = 0;
		break;
		case UPDATE_FAIL:
		{
			//clr_lcd_buff();
			GLCD_ClearScreen();
			
			//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
			GLCD_GoTo(0,1);
			GLCD_WriteString((char*)"   UPDATE FAILED    ");
			//copyStrToLcdBuff(gprs_config_lcd_line3, 1);
			//set_lcd_buff_sts(TRUE);
			//release_hex_file_states();
			set_hex_file_updt_state(DISPLAY_HOLD);
		}
		break;
		case VERIFYING:
			if(++time >= BLINK_TIME)
			{
				time = 0;
				//GLCD_ClearScreen();
				
				//	31-12-2020 SK: add code to print the below lines on graphical lcd according to library
				
				if(blink_flag)
				{
					blink_flag = FALSE;
					GLCD_GoTo(0,1);
					GLCD_WriteString((char*)"    VERIFYING   ");
					//copyStrToLcdBuff(conn_verify_line, 1);
				}
				else
				{
					blink_flag = TRUE;
					GLCD_GoTo(0,1);
					GLCD_WriteString((char*)"                ");
					//copyStrToLcdBuff(empty_string, 1);
				}
				//set_lcd_buff_sts(TRUE);
			}
		break;
		case DISPLAY_HOLD:
			release_hex_file_states();
			set_frmwr_update_state(HEX_FILE_UPDATE_RELEASE);
			set_hex_file_updt_state(DEFAULT);
			/*if(++display_time >= DISPLAY_HOLD_TIME)
			{
				release_hex_file_states();
				display_time = 0;
				changeFdmState(FDM_ON);
			}*/
		break;
		default:
		break;
	}
}


