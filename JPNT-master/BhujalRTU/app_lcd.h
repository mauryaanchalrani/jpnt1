#ifndef APP_LCD_H_
#define APP_LCD_H_




#define PROJECT_NAME			"BHUJAL RTU"
#define VERSION_NUM				"V 1.07"
#define E4E						"      E4ENGINEER     "
#define COMPANY_NAME			" COMP_NAME     "
#define LIMIT					"LIMIT:"
#define INST_FLOW				"I FLOW :"
/*
#define TODAY_TOTAL				"T TOTAL:"
#define MONTHLY_TOTAL			"M TOTAL:"
#define COM_TOTAL				"C TOTAL:"*/
#define TODAY_TOTAL				"T TOT:"
#define MONTHLY_TOTAL			"M TOT:"
#define COM_TOTAL				"C TOT:"

#define Y_TOTAL					"Y TOT:"		//abdul majeed
#define NA						"        NA"

#define COMP_ID					" #215 "
#define PARTION_LINE			"---------------------"
#define L						"L"
#define KL						"KL"
#define CHARACTER_SIZE			(6)
#define CONFIG					"CONFIGURATION"


#define DISPLAY_HOLD_TIME					(5 * (1000 / CTRL_STATE_SCAN_TIME))
#define BLINK_TIME							(500 / CTRL_STATE_SCAN_TIME)

void welcome_screen(void);		//25-02-2019 SK
void data_screen(void);
void config_screen(void);

void manage_hex_file_update(void);
void flash_hex_file_progress(void);
void hexFileUpdateRoutines(void);


#endif /* APP_LCD_H_ */