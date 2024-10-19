#ifndef __SYSTEM_STS_H__
#define __SYSTEM_STS_H__

typedef enum
{
	IMV_IDLE,
	IMV_ACTIVE,
	IMV_INACTIVE,
}imv_state_t;

//HJ 21-09-2017
void check_tank_lid(void);
//void update_sts_bit(sts_bits_t, int);

void check_ignition(void);
void check_alive(void);
void check_movement(void);

void update_system_sts(unsigned int *);

//HJ 26-03-2018
void immobilze_vehicle(void);
char set_imv_state(char *);

#endif