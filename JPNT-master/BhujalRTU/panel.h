

/*
 * panel.h
 *
 * Created: 8/31/18 4:53:22 PM
 *  Author: Mindfield
 */ 

#ifndef _PANEL_H_
#define _PANEL_H_


#define  BIAS     0x52             //0b1000 0101 0010  
#define  SYSDIS   0X00             //0b1000 0000 0000  
#define  SYSEN    0X02             //0b1000 0000 0010 
#define  LCDOFF   0X04             //0b1000 0000 0100 
#define  LCDON    0X06             //0b1000 0000 0110 
#define  XTAL     0x28             //0b1000 0010 1000 
#define  RC256    0X30             //0b1000 0011 0000 
#define  TONEON   0X12             //0b1000 0001 0010 
#define  TONEOFF  0X10             //0b1000 0001 0000 
#define  WDTDIS1  0X0A             //0b1000 0000 1010 
#define NUM_OF_PANEL	(3)

/*
typedef struct  
{
	char dis_num;
	char num_of_parameter;
	void *para_1;
	void *para_2;
	void *para_3;
	void *para_4;
}panel_display_map_t;

*/

void floatToString(float , char *, int );
int intToStr(int , char* , int );
void reverse(char *, int );
int indexOfDp(char*);
class  mylcd
{
	public:
	int cs;
	int wr;
	int dat;
	int backlight;
	mylcd();
	
	void run(int cs, int wr, int dat,int backlight);
	void conf();//
	void clr();//
	void display(unsigned char addr, unsigned char sdata);//
	void backlightOn();//
	void backlightOff();//
	void wrOne(unsigned char addr, unsigned char sdata);
	void wrClrData(unsigned char addr, unsigned char sdata);
	void wrClr(unsigned char len);
	void wrData(unsigned char data, unsigned char cnt);
	void wrCmd(unsigned char CMD);
	void bat1();
	void bat1Off();
	void bat2();
	void bat2Off();
	void bat3();
	void bat3Off();
	void dispNum(float num);
	void lcdOn();
	void lcdOff();
	void updatePanel1(void);
	void updatePanel2(void);
	void updatePanel3(void);
	
	private:
	int _cs;
	int _wr;
	int _dat;
	int _backlight;
};



#endif