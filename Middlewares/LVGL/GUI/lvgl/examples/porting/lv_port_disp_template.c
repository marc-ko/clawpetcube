/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp_template.h"
#include "../../lvgl.h"

#include "lcd.h"
#include "lcd_init.h"
#include "lcd_dma.h"
#include "lcd_spi.h"

/*********************
 *      DEFINES
 *********************/
//#define USE_SRAM        0     
#ifdef USE_SRAM
//#include "./MALLOC/malloc.h"
#endif

#define MY_DISP_HOR_RES (240)   /* LCD WIDTH */
#define MY_DISP_VER_RES (240)   /* LCD HEIGHT */

#define FRESH_DMA 		 1		  // 1: Use DMA to refresh the screen, 0: Use SPI to refresh the screen
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

/* Display refresh func  */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
/* GPU Fill need init GPU first*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

lv_disp_drv_t disp_drv;   // Display device driver         
/**********************
 *  STATIC VARIABLES
 **********************/
//uint32_t* address_buff;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * @brief       LCD accelerated drawing function
 * @param       (sx, sy), (ex, ey): Coordinates of the diagonal corners of the filled rectangle, area size is: (ex - sx + 1) * (ey - sy + 1)
 * @param       color: Color to fill
 * @retval      None
 */
void lcd_draw_fast_rgb_color(int16_t sx, int16_t sy,int16_t ex, int16_t ey, uint16_t *color)
{
	uint16_t w = ex-sx+1;
	uint16_t h = ey-sy+1;
	uint32_t draw_size = w * h;		// calculate the size of the drawing area
	
	LCD_Address_Set(sx,sy,ex,ey);	// Configure the drawing area

#if FRESH_DMA == 1
	LCD_CS_Clr();									// Chip select pin ground
	LCD_DMA_Enable(DMA2_Stream3, (uint32_t)&SPI1->DR, (uint32_t)color, draw_size * sizeof(uint16_t));		// ʹ��DMA
#endif

#if FRESH_DMA == 0	
	LCD_CS_Clr();					//  Chip select pin ground
	for(uint32_t i = 0; i < draw_size; i++)
	{
		LCD_WR_DATA_LVGL(color[i]);	// Non-DMA mode, write data to the screen
	}
	LCD_CS_Set();
#endif
}

/**
 * @brief       Initialize the display and register it in LVGL
 */
void lv_port_disp_init(void)
{
	// Initialize the display
	disp_init();

	// Register the display buffer for LVGL

		/**
	 * LVGL needs a buffer to draw widgets.
	 * Subsequently, the contents of this buffer will be copied to the display device via the `flush_cb` (display device refresh function).
	 * The size of this buffer needs to be larger than the size of one line of the display device.
	 *
	 * There are 3 buffer configurations:
	 * 1. Single buffer:
	 *      LVGL will draw the contents of the display device here and write it to the display device.
	 *
	 * 2. Double buffer:
	 *      LVGL will draw the contents of the display device to one of the buffers and write it to the display device.
	 *      DMA is needed to write the contents to be displayed on the display device into the buffer.
	 *      When data is sent from the first buffer, it allows LVGL to draw the next part of the screen into another buffer.
	 *      This allows rendering and refreshing to be executed in parallel.
	 *
	 * 3. Full-size double buffer:
	 *      Set two full-size buffers of screen size and set disp_drv.full_refresh = 1.
	 *      This way, LVGL will always provide the entire rendered screen in the form of 'flush_cb', and you only need to change the frame buffer address.
	 */
	
	/* Single buffer example */
//    static lv_disp_draw_buf_t draw_buf_dsc_1;
//#if USE_SRAM
//    static lv_color_t buf_1 = mymalloc(SRAMEX, MY_DISP_HOR_RES * MY_DISP_VER_RES);              /* Set the buffer size to the full screen size */
//    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * MY_DISP_VER_RES);     /* Initialize the display buffer */
//#else
//    static lv_color_t buf_1[MY_DISP_HOR_RES * 60];                                              /* Set the buffer size to 60 lines of the screen size */
//    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 60);                  /* Initialize the display buffer */
//#endif

    // Double buffer example
    static lv_disp_draw_buf_t draw_buf_dsc_2;
    static lv_color_t buf_2_1[MY_DISP_HOR_RES * 20];                                            /* Set the buffer size to 30 lines of the screen size */
    static lv_color_t buf_2_2[MY_DISP_HOR_RES * 20];                                            /* Set another buffer size to 30 lines of the screen size */
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * 20);             /* Initialize the display buffer */

    /* Full-size double buffer and config disp_drv.full_refresh = 1 */
//    static lv_disp_draw_buf_t draw_buf_dsc_3;
//    static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];                               /* Set a full-size buffer */
//    static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES];                               /* Set another full-size buffer */
//    lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, MY_DISP_HOR_RES * MY_DISP_VER_RES);/* Initialize the display buffer */
	/*-----------------------------------
	 * Register the display in LVGL
	 *----------------------------------*/

	lv_disp_drv_init(&disp_drv);                    /* Init it to default setting */

	disp_drv.hor_res = MY_DISP_HOR_RES;
	disp_drv.ver_res = MY_DISP_VER_RES;

	/* Copying the flush in buffer to the display */
	disp_drv.flush_cb = disp_flush;

	/* Setting up drawing buffer */
	disp_drv.draw_buf = &draw_buf_dsc_2;

	/* double buffer here */
	//disp_drv.full_refresh = 1

	/* If you have a GPU, use it to fill the memory array with color.
	 * Note, you can enable LVGL's built-in GPU support in lv_conf.h.
	 * But if you have a different GPU, you can use this callback function. */
	//disp_drv.gpu_fill_cb = gpu_fill;

	/* REGISTER THE DISP */
	lv_disp_drv_register(&disp_drv);
	
#if FRESH_DMA == 1
	LCD_DMA2_Config((uint32_t)&SPI1->DR,(uint32_t)buf_2_1, MY_DISP_HOR_RES * 30);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief       Initialize the display device and necessary peripherals
 * @param       None
 * @retval      None
 */
static void disp_init(void)
{
    /*Your code here*/
    LCD_Init();			// LCD initialization function (hardware SPI)
}

/**
 * @brief       Refresh the content of the internal buffer to a specific area on the display
 *   @note      You can use DMA or any hardware to accelerate this operation in the background
 *              However, you need to call the function 'lv_disp_flush_ready()' after the refresh is complete
 *
 * @param       disp_drv    : Display device
 *   @arg       area        : The area to be refreshed, containing the coordinates of the diagonal corners of the filled rectangle
 *   @arg       color_p     : Color array
 *
 * @retval      None
 */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /* LVGL Official scr drawing !!NOT RECOMMEND SINCE LOW EFFIENCT */

//    int32_t x;
//    int32_t y;
//    for(y = area->y1; y <= area->y2; y++) {
//        for(x = area->x1; x <= area->x2; x++) {
//            /*Put a pixel to the display. For example:*/
//            /*put_px(x, y, *color_p)*/
//            color_p++;
//        }
//    }

    /* Fill a specified area with a specified color block */
	
    lcd_draw_fast_rgb_color(area->x1,area->y1,area->x2,area->y2,(uint16_t*)color_p);

/* Important!!!
 * Notify the graphics library that the refresh is complete */
   // lv_disp_flush_ready(disp_drv);
}

/* Optional: GPU interface */

/* If your MCU has a hardware accelerator (GPU), you can use it to fill the memory with color */
/**
 * @brief       Use GPU for color filling
 * @note        If the MCU has a hardware accelerator (GPU), it can be used to fill the memory with color
 *
 * @param       disp_drv    : Display device
 * @arg         dest_buf    : Destination buffer
 * @arg         dest_width  : Width of the destination buffer
 * @arg         fill_area   : Area to fill
 * @arg         color       : Color array
 *
 * @retval      None
 */
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/

//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf += dest_width;    /*Go to the next line*/
//    }
//}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
