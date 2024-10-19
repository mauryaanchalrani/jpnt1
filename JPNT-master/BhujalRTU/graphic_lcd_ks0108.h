#ifndef __GRAPHIC_LCD_KS0108_H__
#define __GRAPHIC_LCD_KS0108_H__

#define KS0108_SCREEN_WIDTH		128
#define KS0108_SCREEN_HEIGHT	64

#define DISPLAY_SET_Y			0x40
#define DISPLAY_SET_X			0xB8
#define DISPLAY_START_LINE		0xC0
#define DISPLAY_ON_CMD			0x3E
//#define ON						0x01
//#define OFF						0x00
#define DISPLAY_STATUS_BUSY		0x80


void GLCD_InitalizePorts(void);
void GLCD_EnableController(unsigned char);
void GLCD_DisableController(unsigned char);
void GLCD_WriteCommand(unsigned char, unsigned char);
void GLCD_WriteData(unsigned char);
unsigned char GLCD_ReadByteFromROMMemory(char *);
void GLCD_Initalize(void);
void GLCD_GoTo(unsigned char, unsigned char);
void GLCD_ClearScreen(void);
void GLCD_WriteChar(char);
void GLCD_WriteChar_inv(char);
void GLCD_WriteString(char *);
void GLCD_WriteString_inv(char *);
void GLCD_Bitmap(char *, unsigned char, unsigned char, unsigned char, unsigned char);
void GLCD_Rectangle(unsigned char x, unsigned char y, unsigned char b, unsigned char a);
void GLCD_Circle(unsigned char cx, unsigned char cy ,unsigned char radius);
void GLCD_Line(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2);
unsigned char GLCD_ReadData(void);
unsigned char GLCD_ReadStatus(unsigned char);
void GLCD_SetPixel(unsigned char x, unsigned char y, unsigned char color);

#endif