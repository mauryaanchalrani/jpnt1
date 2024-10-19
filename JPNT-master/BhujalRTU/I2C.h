#ifndef I2C_H
#define I2C_H

void I2CInit();
void I2CDeinit();		//Anand 13.07.2014
void I2CClose();

uint8_t I2CStart();		// HJ 05-07-2017   Repalce void with uint8_t
uint8_t I2CStop();		// HJ 05-07-2017   Repalce void with uint8_t

uint8_t I2CWriteByte(uint8_t data);
uint8_t I2CReadByte(uint8_t *data, uint8_t ack);	

// HJ 05-07-2017
void I2CReset(void);

#endif



