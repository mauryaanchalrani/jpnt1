/*
 * graphic_lcd_ks0108.cpp
 *
 * Created: 07/02/2019 02:36:48
 *  Author: erhem
 */ 
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "pins.h"
#include "graphic_lcd_ks0108.h"
#include "font5x8.h"
#include "main.h"


unsigned char screen_x = 0;
unsigned char screen_y = 0;
unsigned char color = 1;

extern ram_data_t ram_data;

void GLCD_InitalizePorts(void)
{
	KS0108_DATA_DIR = 0xFF;
	KS0108_DATA_PORT = 0x00;
	
	KS0108_CS_CTRL_DIR |= (( 1 << KS0108_CS1) | (1 << KS0108_CS2));
	KS0108_CTRL_DIR |= ((1 << KS0108_RS)| (1 << KS0108_EN) /*| (1 << KS0108_RW)*/);
	
	KS0108_CS_CTRL_PORT |= (( 1 << KS0108_CS1) | (1 << KS0108_CS2));
	//KS0108_CS_CTRL_PORT &= ~(1 << KS0108_RW);
	
	KS0108_CTRL_PORT |= ((1 << KS0108_RS)| (1 << KS0108_EN));
}
//-------------------------------------------------------------------------------------------------
// Enable Controller (0-2)
//-------------------------------------------------------------------------------------------------
void GLCD_EnableController(unsigned char controller)
{
	switch(controller){
		case 0 : KS0108_CS_CTRL_PORT &= ~(1 << KS0108_CS1); break;
		case 1 : KS0108_CS_CTRL_PORT &= ~(1 << KS0108_CS2); break;
	}
}
//-------------------------------------------------------------------------------------------------
// Disable Controller (0-2)
//-------------------------------------------------------------------------------------------------
void GLCD_DisableController(unsigned char controller)
{
	switch(controller){
		case 0 : KS0108_CS_CTRL_PORT |= (1 << KS0108_CS1); break;
		case 1 : KS0108_CS_CTRL_PORT |= (1 << KS0108_CS2); break;
	}
}
//-------------------------------------------------------------------------------------------------
// Write command to specified controller
//-------------------------------------------------------------------------------------------------
void GLCD_WriteCommand(unsigned char commandToWrite, unsigned char controller)
{
	KS0108_CTRL_PORT &= ~(1 << KS0108_RS);
	GLCD_EnableController(controller);
	KS0108_DATA_PORT = commandToWrite;
	KS0108_CTRL_PORT |= (1 << KS0108_EN);
	_delay_us(2);
	KS0108_CTRL_PORT &= ~(1 << KS0108_EN);
	GLCD_DisableController(controller);
	_delay_us(2);
}
//-------------------------------------------------------------------------------------------------
// Write data to current position
//-------------------------------------------------------------------------------------------------
void GLCD_WriteData(unsigned char dataToWrite)
{
	KS0108_CTRL_PORT |= (1 << KS0108_RS);
	//_delay_us(2);
	GLCD_EnableController(screen_x / 64);
	KS0108_DATA_PORT = dataToWrite;
	KS0108_CTRL_PORT |= (1 << KS0108_EN);
	_delay_us(2);
	KS0108_CTRL_PORT &= ~(1 << KS0108_EN);
	GLCD_DisableController(screen_x / 64);
	
	screen_x++;
	if(screen_x >= KS0108_SCREEN_WIDTH)
		screen_x = 0;
	_delay_us(2);
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
unsigned char GLCD_ReadByteFromROMMemory(char *ptr)
{
	return pgm_read_byte(ptr);
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_Initalize(void)
{
	unsigned char i;
	GLCD_InitalizePorts();
	
	for(i = 0; i < 2; i++)
	{
		GLCD_WriteCommand((DISPLAY_ON_CMD | ON), i);
	}
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_GoTo(unsigned char x, unsigned char y)
{
	unsigned char i;
	screen_x = x;
	screen_y = y;

	for(i = 0; i < (KS0108_SCREEN_WIDTH / 32); i++)//64
	{
		GLCD_WriteCommand((DISPLAY_SET_Y | 0), i);
		GLCD_WriteCommand((DISPLAY_SET_X | y), i);
		GLCD_WriteCommand((DISPLAY_START_LINE | 0), i);
	}
	GLCD_WriteCommand((DISPLAY_SET_Y | (x % 64)), (x / 64));
	GLCD_WriteCommand((DISPLAY_SET_X | y), (x / 64));
	
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_ClearScreen(void)
{
	unsigned char i, j;
	for(j = 0; j < (KS0108_SCREEN_HEIGHT / 8); j++)
	{
		GLCD_GoTo(0,j);
		for(i = 0; i < KS0108_SCREEN_WIDTH; i++)
			GLCD_WriteData(0x00);
	}
	//GLCD_GoTo(0, 0);
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WriteChar(char charToWrite)
{
	int i;
	charToWrite -= ' ';			//' ' == 32
	for(i = 0; i < 5; i++)
		GLCD_WriteData(GLCD_ReadByteFromROMMemory((char *)(font5x8 + (5 * charToWrite) + i)));
	GLCD_WriteData(0x00);
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WriteChar_inv(char charToWrite)
{
	int i;
	charToWrite -= ' ';			//' ' == 32
	for(i = 0; i < 5; i++)
		GLCD_WriteData(~GLCD_ReadByteFromROMMemory((char *)(font5x8 + (5 * charToWrite) + i)));
	GLCD_WriteData(~0x00);
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WriteString(char * stringToWrite)
{
	while(*stringToWrite)
		GLCD_WriteChar(*stringToWrite++);
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WriteString_inv(char * stringToWrite)
{
	while(*stringToWrite)
		GLCD_WriteChar_inv(*stringToWrite++);
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_Bitmap(char * bmp, unsigned char x, unsigned char y, unsigned char dx, unsigned char dy)
{
	unsigned char i, j;
	for(j = 0; j < (dy / 8); j++)
	{
		GLCD_GoTo(x, y + j);
		for(i = 0; i < dx; i++) 
			GLCD_WriteData(GLCD_ReadByteFromROMMemory(bmp++));
	}
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_SetPixel(unsigned char x, unsigned char y, unsigned char color)
{
	unsigned char tmp;
	GLCD_GoTo(x, (y / 8));
	tmp = GLCD_ReadData();
	GLCD_GoTo(x, (y / 8));
	tmp = GLCD_ReadData();
	GLCD_GoTo(x, (y / 8));
	tmp |= (1 << (y % 8));
	GLCD_WriteData(tmp);

}

void GLCD_Rectangle(unsigned char x, unsigned char y, unsigned char b, unsigned char a)
{
	unsigned char j; // zmienna pomocnicza
	// rysowanie linii pionowych (boki)
	for (j = 0; j < a; j++) {
		GLCD_SetPixel(x, y + j, color);
		GLCD_SetPixel(x + b - 1, y + j, color);
	}
	// rysowanie linii poziomych (podstawy)
	for (j = 0; j < b; j++)	{
		GLCD_SetPixel(x + j, y, color);
		GLCD_SetPixel(x + j, y + a - 1, color);
	}
}
//

void GLCD_Circle(unsigned char cx, unsigned char cy ,unsigned char radius)
{
	int x, y, xchange, ychange, radiusError;
	x = radius;
	y = 0;
	xchange = 1 - 2 * radius;
	ychange = 1;
	radiusError = 0;
	while(x >= y)
	{
		GLCD_SetPixel(cx+x, cy+y, color);
		GLCD_SetPixel(cx-x, cy+y, color);
		GLCD_SetPixel(cx-x, cy-y, color);
		GLCD_SetPixel(cx+x, cy-y, color);
		GLCD_SetPixel(cx+y, cy+x, color);
		GLCD_SetPixel(cx-y, cy+x, color);
		GLCD_SetPixel(cx-y, cy-x, color);
		GLCD_SetPixel(cx+y, cy-x, color);
		y++;
		radiusError += ychange;
		ychange += 2;
		if ( 2*radiusError + xchange > 0 )
		{
			x--;
			radiusError += xchange;
			xchange += 2;
		}
	}
}
//
#if 0
void GLCD_Line(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2)
{
	int CurrentX, CurrentY, Xinc, Yinc,
	Dx, Dy, TwoDx, TwoDy,
	TwoDxAccumulatedError, TwoDyAccumulatedError;

	Dx = (X2-X1); // obliczenie sk³adowej poziomej
	Dy = (Y2-Y1); // obliczenie sk³adowej pionowej

	TwoDx = Dx + Dx; // podwojona sk³adowa pozioma
	TwoDy = Dy + Dy; // podwojona sk³adowa pionowa

	CurrentX = X1; // zaczynamy od X1
	CurrentY = Y1; // oraz Y1

	Xinc = 1; // ustalamy krok zwiêkszania pozycji w poziomie
	Yinc = 1; // ustalamy krok zwiêkszania pozycji w pionie

	if(Dx < 0) // jesli sk³adowa pozioma jest ujemna
	{
		Xinc = -1; // to bêdziemy siê "cofaæ" (krok ujemny)
		Dx = -Dx;  // zmieniamy znak sk³adowej na dodatni
		TwoDx = -TwoDx; // jak równie¿ podwojonej sk³adowej
	}

	if (Dy < 0) // jeœli sk³adowa pionowa jest ujemna
	{
		Yinc = -1; // to bêdziemy siê "cofaæ" (krok ujemny)
		Dy = -Dy; // zmieniamy znak sk³adowej na dodatki
		TwoDy = -TwoDy; // jak równiez podwojonej sk³adowej
	}

	GLCD_SetPixel(X1,Y1, color); // stawiamy pierwszy krok (zapalamy pierwszy piksel)

	if ((Dx != 0) || (Dy != 0)) // sprawdzamy czy linia sk³ada siê z wiêcej ni¿ jednego punktu ;)
	{
		// sprawdzamy czy sk³adowa pionowa jest mniejsza lub równa sk³adowej poziomej
		if (Dy <= Dx) // jeœli tak, to idziemy "po iksach"
		{
			TwoDxAccumulatedError = 0; // zerujemy zmienn¹
			do // ruszamy w drogê
			{
				CurrentX += Xinc; // do aktualnej pozycji dodajemy krok
				TwoDxAccumulatedError += TwoDy; // a tu dodajemy podwojon¹ sk³adow¹ pionow¹
				if(TwoDxAccumulatedError > Dx)  // jeœli TwoDxAccumulatedError jest wiêkszy od Dx
				{
					CurrentY += Yinc; // zwiêkszamy aktualn¹ pozycjê w pionie
					TwoDxAccumulatedError -= TwoDx; // i odejmujemy TwoDx
				}
				GLCD_SetPixel(CurrentX,CurrentY, color);// stawiamy nastêpny krok (zapalamy piksel)
			}while (CurrentX != X2); // idziemy tak d³ugo, a¿ osi¹gniemy punkt docelowy
		}
		else // w przeciwnym razie idziemy "po igrekach"
		{
			TwoDyAccumulatedError = 0;
			do
			{
				CurrentY += Yinc;
				TwoDyAccumulatedError += TwoDx;
				if(TwoDyAccumulatedError>Dy)
				{
					CurrentX += Xinc;
					TwoDyAccumulatedError -= TwoDy;
				}
				GLCD_SetPixel(CurrentX,CurrentY, color);
			}while (CurrentY != Y2);
		}
	}
}
#endif
void GLCD_Delay(void)
{
	asm("nop");
}

unsigned char GLCD_ReadStatus(unsigned char controller)
{
	
	unsigned char status;
	KS0108_DATA_DIR = 0x00;
	//KS0108_CTRL_PORT |= KS0108_RW;
	KS0108_CTRL_PORT &= ~KS0108_RS;
	GLCD_EnableController(controller);
	KS0108_CTRL_PORT |= KS0108_EN;
	GLCD_Delay();
	status = KS0108_DATA_PIN;
	KS0108_CTRL_PORT &= ~KS0108_EN;
	GLCD_DisableController(controller);
	return status;
}


//-------------------------------------------------------------------------------------------------
// Read data from current position
//-------------------------------------------------------------------------------------------------
unsigned char GLCD_ReadData(void)
{
	unsigned char data;
	while(GLCD_ReadStatus(screen_x / 64)&DISPLAY_STATUS_BUSY);
	KS0108_DATA_DIR = 0x00;
	KS0108_CTRL_PORT |= (/*KS0108_RW |*/ KS0108_RS);
	GLCD_EnableController(screen_x / 64);
	//GLCD_Delay();
	KS0108_CTRL_PORT |= KS0108_EN;
	GLCD_Delay();
	data = KS0108_DATA_PIN;
	KS0108_CTRL_PORT &= ~KS0108_EN;
	GLCD_DisableController(screen_x / 64);
	screen_x++;
	return data;
}



