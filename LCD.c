/*----------------------------------------------------------------------------
 * Name:    LCD.c
 * Purpose: API ll Graphic LCD (320x240 pixels) on stm3220
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * The API is developed as part of the Programming Embedded Systems course 
 * at Uppsala University.
 * This software is supplied "AS IS" without warranties of any kind.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include "LCD.h"
#include "stdio.h"

static int clearNextTime = 0;

void LCD_init(void){
	STM322xG_LCD_Init();
	LCD_SetFont(&FONTSIZE);
}

void LCD_clear(unsigned short color){
	LCD_Clear(color);
	clearNextTime = 1;
}
void LCD_setTextColor(unsigned short color){
	LCD_SetTextColor(color);
}

void LCD_setBackColor(unsigned short color){
  LCD_SetBackColor(color);
}

void LCD_setColors(unsigned short text, unsigned short back){
  LCD_SetColors(text, back);
}

void LCD_getColors(unsigned short *text, unsigned short *back){
	LCD_GetColors(text, back);	
}
void LCD_displayChar(unsigned int ln, unsigned int col, unsigned char  c){
  LCD_DisplayChar(ln, WIDTH - col, c);
}
void LCD_displayStringLn(unsigned int ln, unsigned char *s){
	LCD_DisplayStringLine(ln, s);
}
/*TODO*/
void LCD_bitmap(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bitmap);
void LCD_clearLn(unsigned int ln){
	LCD_ClearLine(ln);
}
void LCD_putPixel(unsigned int x, unsigned int y){
	PutPixel(y, WIDTH - x);
}
void LCD_drawLine(uint16_t x, uint16_t y, uint16_t length, uint16_t direction){
	LCD_DrawLine(y, x, length, direction);
}
void LCD_drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
	LCD_DrawRect(y, WIDTH - x, height, width);
}
void LCD_fillRect(uint16_t x, uint16_t y, uint16_t height, uint16_t width){
	LCD_DrawFullRect(y, WIDTH - x, height, width);
}

int getMaxLines(void){
	return 240/FONTSIZE.Height;
}

int fputc(int ch, FILE *f) {
	static uint16_t nLine = 0;
	static uint16_t refcolumn = LCD_PIXEL_WIDTH - 1;
	
	if(clearNextTime){
		LCD_Clear(LCD_COLOR_WHITE);
		nLine = 0;
		refcolumn = LCD_PIXEL_WIDTH - 1;
		clearNextTime = 0;
	}
	if(ch == '\n'){
		refcolumn = LCD_PIXEL_WIDTH - 1;
		nLine++;
	}
	else{
		LCD_DisplayChar(LINE(nLine), refcolumn, ch);
		refcolumn -= LCD_GetFont()->Width;
		if (((refcolumn + 1) & 0xFFFF) < LCD_GetFont()->Width){
			refcolumn = LCD_PIXEL_WIDTH - 1;
			nLine++;
		}
	}
	if(nLine>=getMaxLines())
				clearNextTime = 1;
  return ch;
}
