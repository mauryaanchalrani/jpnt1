/*
 * sms.h
 *
 * Created: 09-02-2018 12:43:39 PM
 *  Author: OWNER
 */ 


#ifndef SMS_H_
#define SMS_H_

//#include "gprs.h"

#define GSM_RETRY_CNT (2)
#define GSM_TIMEOUT (30000/WEB_COMMS_SCHEDULAR_TIME)

#define GSM_LOGIN_TIMEOUT	(150)
#define GSM_MSG_ACK_TIMEOUT	(GSM_TIMEOUT)	//10second 50

#define MOB_NUM_LEN 14 // include \0 at the end!
#define COUNTRY_CODE_LEN 3 //+91
#define MOB_NUM_BUFFER 16
#define RCV_MSG_BUF	30

//#define RCV_MSG_BUF	120

#define GSM_RX_BUFFER_MAX 128

#define LOGIN_SIZE	(8)

#define NUM_OF_ADMIN	(4)
#define PASS_LEN		(5)	//including null


//#define NUM_OF_CMDS		(10)
#define CMD_LEN			(4)


#define SMS_DATA_INDEX	(4)
#define MOB_NUM_INDEX	(6)
#define MASTER_CMD_INDEX	(40)
#define ADMIN_CMD_INDEX	(45)
#define PASS_INDEX	(39)

#define NULL_INDEX	(4)

enum
{
	AAD, 	//Add Admin
	EAD,	//Edit Admin
	CPS,	//Change Password
	CIP,	//Change IP
	CPO,	//Change Port
	CAP,	//Change APN
	SSR,	//Set Static rate
	//SMR,	//Set movable rate
	GGL,	//Get google link
	GCF,	//Get config
	FDE,	//Factory Default
	CSN,	//Change SIM numberr
	IMV,	//ImMobilize Vehicle
	CFM,
	CPC,
	RCC,
	PWR,
	PID,
	NUM_OF_CMDS
};

typedef enum
{
	GSM_CMD_AT,
	GSM_RSP_AT,
	GSM_CMD_ECHO_OFF,
	GSM_RSP_ECHO_OFF,
	GSM_CMD_TEXT_MODE,
	GSM_RSP_TEXT_MODE,
	GSM_CMD_CNMI,
	GSM_RSP_CNMI,
	GSM_CMD_CPMS,
	GSM_RSP_CPMS,
	GSM_CMD_CMGR,
	GSM_RSP_CMGR_NUM,
	GSM_RSP_CMGR_MSG,
	GSM_RX_MSG_RSP,
	GSM_CMD_CMGD,
	GSM_RSP_CMGD,				
	GSM_CMD_POWER_OFF,
	GSM_RSP_POWER_OFF,
	GSM_MSTR_RSP,	//Response to master
}gsm_state_t;

typedef enum
{
	GSM_PASS,
	GSM_FAIL,
	GSM_IN_PRG,
	GSM_WAIT,
}gsm_status_t;

enum {GSM_NO_NEW_MSG, GSM_MATCH_FAIL, GSM_MATCH_OK} ;

typedef enum
{
	MSG_AUTH,
	MSG_DATA,
	MSG_RESP,
}rx_sms_state_t;

enum {TX_MSG_NUM,TX_MSG_NUM_RSP,TX_MSG_DATA,TX_MSG_DATA_RSP};
	
typedef struct
{
	/*gsm_state_t state;
	char receive_now;*/
	//SEND_SMS_t sms_type;//we use SEND_SMS_t here - bcoz finally it is used to send SMS
	char sender_number[MOB_NUM_LEN];
	char old_number[MOB_NUM_LEN];
	char new_number[MOB_NUM_LEN];
	char recv_msg_data[RCV_MSG_BUF];
	char send_msg_data[GSM_RX_BUFFER_MAX];
}sms_data_t;

typedef struct
{
	unsigned char mob_num[NUM_OF_ADMIN][MOB_NUM_LEN];	// 56 bytes
	unsigned char pswd[PASS_LEN];  // 5 bytes
	//unsigned char movable_rate;	//:VC - 10/07/18 - Need to check if we need this variable
	//unsigned char static_rate;
	unsigned char chksum;                 
}__attribute__((packed)) gsm_cfg_data_t;  // total 62 bytes of data

gsm_state_t check_msg_data(void);	

gsm_status_t sms_handler(void);
gsm_status_t tx_msg(void);

int decode_msg(char *str);
int validatePass(char*);

#endif /* SMS_H_ */