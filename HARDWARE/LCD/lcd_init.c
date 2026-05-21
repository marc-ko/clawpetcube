#include "lcd_init.h"
#include "lcd_spi.h"
#include "lcd_dma.h"

void LCD_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;		   // LCD_BLK
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // GPIO OUTPUT
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // Push-pull output
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   // Pull-up
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5; // LCD_RES,LCD_DC
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		   // GPIO OUTPUT
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		   // Push-pull output
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	   // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		   // Pull-up
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_0);
	GPIO_SetBits(GPIOC, GPIO_Pin_4 | GPIO_Pin_5);
}

/******************************************************************************
Function: LCD serial data write function
Input:    dat  Serial data to be written
Return:   None
******************************************************************************/
void LCD_Writ_Bus(u8 dat)
{
	LCD_CS_Clr();
	SPI1_ReadWriteByte(dat);
	LCD_CS_Set();
}

/******************************************************************************
Function: LCD write data
Input:    dat  Data to be written
Return:   None
******************************************************************************/
void LCD_WR_DATA8(u8 dat)
{
	LCD_Writ_Bus(dat);
}

/******************************************************************************
Function: LCD write data
Input:    dat  Data to be written
Return:   None
******************************************************************************/
void LCD_WR_DATA(u16 dat)
{
	LCD_Writ_Bus(dat >> 8);
	LCD_Writ_Bus(dat);
}

/******************************************************************************
Function: LCD write data for LVGL
Input:    dat  Data to be written
Return:   None
******************************************************************************/
void LCD_WR_DATA_LVGL(u16 dat)
{
	SPI1_ReadWriteByte_LVGL(dat >> 8);
	SPI1_ReadWriteByte_LVGL(dat);
}

/******************************************************************************
Function: LCD write command
Input:    dat  Command to be written
Return:   None
******************************************************************************/
void LCD_WR_REG(u8 dat)
{
	LCD_DC_Clr(); // Write command
	LCD_Writ_Bus(dat);
	LCD_DC_Set(); // Write data
}

/******************************************************************************
Function: Set start and end addresses
Input:    x1, x2  Set the start and end addresses of the column
		  y1, y2  Set the start and end addresses of the row
Return:   None
******************************************************************************/
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2)
{
	if (!FLIP_DISP_BY_PRISM)
	{
		if (USE_HORIZONTAL == 0)
		{
			LCD_WR_REG(0x2a); // Set column address
			LCD_WR_DATA(x1);
			LCD_WR_DATA(x2);
			LCD_WR_REG(0x2b); // Set row address
			LCD_WR_DATA(y1);
			LCD_WR_DATA(y2);
			LCD_WR_REG(0x2c); // Write to memory
		}
		else if (USE_HORIZONTAL == 1)
		{
			LCD_WR_REG(0x2a); // Set column address
			LCD_WR_DATA(x1);
			LCD_WR_DATA(x2);
			LCD_WR_REG(0x2b); // Set row address
			LCD_WR_DATA(y1 + 80);
			LCD_WR_DATA(y2 + 80);
			LCD_WR_REG(0x2c); // Write to memory
		}
		else if (USE_HORIZONTAL == 2)
		{
			LCD_WR_REG(0x2a); // Set column address
			LCD_WR_DATA(x1);
			LCD_WR_DATA(x2);
			LCD_WR_REG(0x2b); // Set row address
			LCD_WR_DATA(y1);
			LCD_WR_DATA(y2);
			LCD_WR_REG(0x2c); // Write to memory
		}
		else
		{
			LCD_WR_REG(0x2a); // Set column address
			LCD_WR_DATA(x1 + 80);
			LCD_WR_DATA(x2 + 80);
			LCD_WR_REG(0x2b); // Set row address
			LCD_WR_DATA(y1);
			LCD_WR_DATA(y2);
			LCD_WR_REG(0x2c); // Write to memory
		}
	}
	else
	{
		if (USE_HORIZONTAL == 0)
		{
			LCD_WR_REG(0x2a); // Set column address
			LCD_WR_DATA(x1);
			LCD_WR_DATA(x2);
			LCD_WR_REG(0x2b); // Set row address
			LCD_WR_DATA(y1);
			LCD_WR_DATA(y2);
			LCD_WR_REG(0x2c); // Write to memory
		}
	}
}

void LCD_Init(void)
{
	SPI1_Init();	 // INIT SPI1
	LCD_GPIO_Init(); // INIT LCD GPIO

	SPI1_SetSpeed(SPI_BaudRatePrescaler_2); // Set SPI  speed

	LCD_RES_Clr(); // Clear reset
	delay_ms(100);
	LCD_RES_Set();
	delay_ms(100);

	LCD_BLK_Set(); // Turn on the backlight
	delay_ms(100);

	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11); // Sleep out
	delay_ms(120);	  // Delay 120ms

	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x36);

	if (FLIP_DISP_BY_PRISM == 0)
	{
		if (USE_HORIZONTAL == 0)
			LCD_WR_DATA8(0x00);
		else if (USE_HORIZONTAL == 1)
			LCD_WR_DATA8(0xC0);
		else if (USE_HORIZONTAL == 2)
			LCD_WR_DATA8(0x70);
		else
			LCD_WR_DATA8(0xA0);
	}
	else if (FLIP_DISP_BY_PRISM == 1)
	{
		if (USE_HORIZONTAL == 0)
			LCD_WR_DATA8(0x40);
		//		else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
		//		else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
		//		else LCD_WR_DATA8(0xA0);
	}
	LCD_WR_REG(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_REG(0xB7);
	LCD_WR_DATA8(0x35);

	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x19);

	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x2C);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x12);

	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x20);

	LCD_WR_REG(0xC6);
	LCD_WR_DATA8(0x0F);

	LCD_WR_REG(0xD0);
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);
	//LCD_WR_REG(0x21);

	LCD_WR_REG(0x29);
}
