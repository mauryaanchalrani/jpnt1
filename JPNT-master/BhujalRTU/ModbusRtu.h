/*
 * ModbusRtu.h
 *
 * Created: 7/10/18 4:21:56 PM
 *  Author: Mindfield
 */ 


#ifndef MODBUSRTU_H_
#define MODBUSRTU_H_

#include <inttypes.h>

			



#define OUTPUT				1



#define MODBUS_MIN_SLV_ID	(0)
#define MODBUS_MAX_SLV_ID	(247)
#define MODBUS_MAX_DATA_LEN (64)


#define MB_COMMS_RATE		(100)
#define MB_CMD_TIMEOUT		(300/MB_COMMS_RATE)		//1 sec

#define MB_FC_5_6_PKT_SIZE	(9)


/**
 * @struct modbus_t
 * @brief
 * Master query structure:
 * This includes all the necessary fields to make the Master generate a Modbus query.
 * A Master may keep several of these structures and send them cyclically or
 * use them according to program needs.
 */
typedef struct
{
	uint16_t query_id;     /*!< Pointer to memory image in master */
    uint8_t u8id;          /*!< Slave address between 1 and 247. 0 means broadcast */
    uint8_t u8fct;         /*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
    uint16_t u16RegAdd;    /*!< Address of the first register to access at slave/s */
    uint16_t u16CoilsNo;   /*!< Number of coils or registers to access */
   // uint16_t *au16reg;     /*!< Pointer to memory image in master */
}
modbus_t;

enum
{
    RESPONSE_SIZE  = 6,
    EXCEPTION_SIZE = 3,
    CHECKSUM_SIZE  = 2
};

/**
 * @enum MESSAGE
 * @brief
 * Indexes to telegram frame positions
 */
enum MESSAGE
{
    ID                             = 0,	 //!< ID field
    FUNC,								 //!< Function code position
    ADD_HI,							  	 //!< Address high byte
    ADD_LO,								 //!< Address low byte
    NB_HI,								 //!< Number of coils or registers high byte
    NB_LO,								 //!< Number of coils or registers low byte
    BYTE_CNT							 //!< byte counter
};

/**
 * @enum MB_FC
 * @brief
 * Modbus function codes summary.
 * These are the implement function codes either for Master or for Slave.
 *
 * @see also fctsupported
 * @see also modbus_t
 */
enum MB_FC
{
    MB_FC_NONE                     = 0,   /*!< null operator */
    MB_FC_READ_COILS               = 1,	/*!< FCT=1 -> read coils or digital outputs */
    MB_FC_READ_DISCRETE_INPUTS      = 2,	/*!< FCT=2 -> read digital inputs */
    MB_FC_READ_REGISTERS           = 3,	/*!< FCT=3 -> read registers or analog outputs */
    MB_FC_READ_INPUT_REGISTER      = 4,	/*!< FCT=4 -> read analog inputs */
    MB_FC_WRITE_COIL               = 5,	/*!< FCT=5 -> write single coil or output */
    MB_FC_WRITE_REGISTER           = 6,	/*!< FCT=6 -> write single register */
    MB_FC_WRITE_MULTIPLE_COILS     = 15,	/*!< FCT=15 -> write multiple coils or outputs */
    MB_FC_WRITE_MULTIPLE_REGISTERS = 16	/*!< FCT=16 -> write multiple registers */
};

enum COM_STATES
{
    COM_IDLE                     = 0,
    COM_WAITING                  = 1

};

enum ERR_LIST
{
    ERR_NOT_MASTER                = -1,
    ERR_POLLING                   = -2,
    ERR_BUFF_OVERFLOW             = -3,
    ERR_BAD_CRC                   = -4,
    ERR_EXCEPTION                 = -5
};

enum
{
    NO_REPLY		 = 255,
    EXC_FUNC_CODE	 = 1,
    EXC_ADDR_RANGE	 = 2,
    EXC_REGS_QUANT   = 3,
    EXC_EXECUTE      = 4,
	NO_EXC			=	0	
	
};
typedef enum
{
	MB_FC_READ_COILS_OFFSET_ADDR = 0x2000,
	MB_FC_READ_DISCRETE_INPUTS_OFFSET_ADDR = 0x10001,
	MB_FC_READ_REGISTER_OFFSET_ADDR = 0x40001,
	MB_FC_READ_INPUT_REGISTER_OFFSET_ADDR = 0x30001,
	MB_FC_WRITE_COIL_OFFSET_ADDR = 0x2000,
	MB_FC_WRITE_REGISTER_OFFSET_ADDR = 0x40001,
	MB_FC_WRITE_MULTIPLE_COILS_OFFSET_ADDR = 0x2000,
	MB_FC_WRITE_MULTIPLE_REGISTERS_OFFSET_ADDR = 0x40001,
}mb_fc_offset_addr_t;

const unsigned char fctsupported[] =
{
    MB_FC_READ_COILS,
    MB_FC_READ_DISCRETE_INPUTS,
    MB_FC_READ_REGISTERS,
    MB_FC_READ_INPUT_REGISTER,
    MB_FC_WRITE_COIL,
    MB_FC_WRITE_REGISTER,
    MB_FC_WRITE_MULTIPLE_COILS,
    MB_FC_WRITE_MULTIPLE_REGISTERS
};

#define MB_READ_COILS_DATA_SIZE	(2)
#define T35  5
#define  MAX_BUFFER  64	//!< maximum size for the communication buffer in bytes

/* ###################### COMMON FUCTIONS USED IN THE MODBUSRTU.H  #######################################################*/ // :VC:11/07/2018: define all functions
unsigned int word( unsigned char hibyte, unsigned char lobyte);
unsigned char highByte(unsigned int);
unsigned char lowByte(unsigned int);
void pinMode(uint8_t, uint8_t,int*);
//unsigned long millis(void);
void digitalWrite(uint8_t, uint8_t,int*);
void delayMicroseconds(unsigned int us);
bool bitRead(uint8_t,uint8_t );
void bitWrite(uint8_t,uint8_t,bool );
uint16_t digitalRead(uint16_t);
void pinModeRtu(uint8_t, uint8_t);
void digitalWriteRtu(uint16_t, uint8_t);
uint16_t digitalReadRtu(uint16_t addr);
/**
 * @class Modbus
 * @brief
 * Arduino class library for communicating with Modbus devices over
 * USB/RS232/485 (via RTU protocol).
 */

class Modbus
{
private:
    //HardwareSerial *port; //!< Pointer to Serial class object
  //  SoftwareSerial *softPort; //!< Pointer to SoftwareSerial class object
    uint8_t u8id; //!< 0=master, 1..247=slave number
    uint8_t u8serno; //!< serial port: 0-Serial, 1..3-Serial1..Serial3; 4: use software serial
    uint8_t u8txenpin; //!< flow control pin: 0=USB or RS-232 mode, >0=RS-485 mode
    uint8_t u8state;
    uint8_t u8lastError;
    uint8_t au8Buffer[MAX_BUFFER];
    uint8_t u8BufferSize;
    uint8_t u8lastRec;
    uint16_t *au16regs;
    uint16_t u16InCnt, u16OutCnt, u16errCnt;
    uint16_t u16timeOut;
    uint32_t u32time, u32timeOut;
    uint8_t u8regsize;

    void init(uint8_t u8id, uint8_t u8serno, uint8_t u8txenpin);
    void init(uint8_t u8id);
    //void sendTxBuffer();
    int8_t getRxBuffer();
    uint16_t calcCRC(uint8_t u8length);
	//uint16_t calcCRC_RS485(uint8_t*, uint8_t);
    int16_t validateAnswer();
    int16_t validateRequest();
    void get_FC1();
    void get_FC3();
    int8_t process_FC1( uint16_t *regs, uint8_t u8size );
    int8_t process_FC3( uint16_t *regs, uint8_t u8size );
    int8_t process_FC5( uint16_t *regs, uint8_t u8size );
    int8_t process_FC6( uint16_t *regs, uint8_t u8size );
    int8_t process_FC15( uint16_t *regs, uint8_t u8size );
    int8_t process_FC16( uint16_t *regs, uint8_t u8size );
    void buildException( uint8_t u8exception ); // build exception message

public:
	//char val;
    Modbus();
    Modbus(uint8_t u8id, uint8_t u8serno);
    Modbus(uint8_t u8id, uint8_t u8serno, uint8_t u8txenpin);
    Modbus(uint8_t u8id);
    void begin(long u32speed);
    //void begin(SoftwareSerial *sPort, long u32speed);
    void begin(long u32speed, uint8_t u8config);
    void begin();
    void setTimeOut( uint16_t u16timeout); //!<write communication watch-dog timer
    uint16_t getTimeOut(); //!<get communication watch-dog timer value
    bool getTimeOutState(); //!<get communication watch-dog timer state
    int8_t query( modbus_t telegram ); //!<only for master
    int16_t poll(); //!<cyclic poll for master
    int16_t poll( uint16_t *regs, uint8_t u8size ); //!<cyclic poll for slave
    uint16_t getInCnt(); //!<number of incoming messages
    uint16_t getOutCnt(); //!<number of outcoming messages
    uint16_t getErrCnt(); //!<error counter
    uint8_t getID(); //!<get slave ID between 1 and 247
    uint8_t getState();
	void setState(uint8_t state);
	void setbuffsize(uint8_t size);
    uint8_t getLastError(); //!<get last error message
    void setID( uint8_t u8id ); //!<write new ID for the slave
    void end(); //!<finish any communication and release serial communication port
	void sendTxBuffer();
	unsigned int calcCRC_RS485(uint8_t*, uint8_t);
  };



#endif /* MODBUSRTU_H_ */