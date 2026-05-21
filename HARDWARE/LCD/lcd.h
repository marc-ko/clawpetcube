#ifndef __LCD_H
#define __LCD_H
#include "sys.h"

void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color);  // Fill a specified area with color
void LCD_DrawPoint(u16 x, u16 y, u16 color);                       // Draw a point at a specified position
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);      // Draw a line at a specified position
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color); // Draw a rectangle at a specified position
void Draw_Circle(u16 x0, u16 y0, u8 r, u16 color);                 // Draw a circle at a specified position

void LCD_ShowChinese(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 sizey, u8 mode);      // Display a string of Chinese characters
void LCD_ShowChinese12x12(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 sizey, u8 mode); // Display a single 12x12 Chinese character
void LCD_ShowChinese16x16(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 sizey, u8 mode); // Display a single 16x16 Chinese character
void LCD_ShowChinese24x24(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 sizey, u8 mode); // Display a single 24x24 Chinese character
void LCD_ShowChinese32x32(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 sizey, u8 mode); // Display a single 32x32 Chinese character

void LCD_ShowChar(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode);        // Display a character
void LCD_ShowString(u16 x, u16 y, const u8 *p, u16 fc, u16 bc, u8 sizey, u8 mode); // Display a string
u32 mypow(u8 m, u8 n);                                                             // Calculate power
void LCD_ShowIntNum(u16 x, u16 y, u16 num, u8 len, u16 fc, u16 bc, u8 sizey);      // Display an integer variable
void LCD_ShowFloatNum1(u16 x, u16 y, float num, u8 len, u16 fc, u16 bc, u8 sizey); // Display a floating-point variable with two decimal places

void LCD_ShowPicture(u16 x, u16 y, u16 length, u16 width, const u8 pic[]); // Display a picture

// Pen colors
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40      // Brown
#define BRRED 0XFC07      // Brown-red
#define GRAY 0X8430       // Gray
#define DARKBLUE 0X01CF   // Dark blue
#define LIGHTBLUE 0X7D7C  // Light blue
#define GRAYBLUE 0X5458   // Gray-blue
#define LIGHTGREEN 0X841F // Light green
#define LGRAY 0XC618      // Light gray (PANEL), window background color
#define LGRAYBLUE 0XA651  // Light gray-blue (middle layer color)
#define LBBLUE 0X2B12     // Light brown-blue (reverse color of selected item)

#endif
