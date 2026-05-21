#include "math.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "usart2.h"
#include "esp8266_common.h"
#include "exti.h"
#include "semphr.h"
#include "../HARDWARE/SDIO/sdio.h"
#include "rtc.h"
#include "timer.h"
#include "adc.h"
#include "../Middlewares/FATFS/ff.h" // Include the FatFs header file
#include "../HARDWARE/SDIO/sdio.h"
#include "../HARDWARE/USART6/usart6.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
// #include "sdio.h"
// #include "fatfs.h"
#include "app_window.h"
#include "gui_guider.h"

/** User Info */
char user_name[12] = "billy";

// #include "dance_pic_data.h"
// #include "weather_pic_data.h"

#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

// Task priorities
#define START_TASK_PRIO 0
// Task stack size
#define START_STK_SIZE 128
// Task handle
TaskHandle_t StartTask_Handler;
// Task function
void start_task(void *pvParameters);

// Task priorities
#define MPU6050_TASK_PRIO 1
// Task stack size
#define MPU6050_STK_SIZE 256
// Task handle
TaskHandle_t MPU6050Task_Handler;
// Task function
void MPU6050_task(void *pvParameters);

// Task priorities
#define RTC_TASK_PRIO 4
// Task stack size
#define RTC_STK_SIZE 512
// Task handle
TaskHandle_t RTCTask_Handler;
// Task function
void RTC_task(void *pvParameters);

// Task priorities
#define GIF_TASK_PRIO 3
// Task stack size
#define DANCE_STK_SIZE 256
// Task handle
TaskHandle_t DanceTask_Handler;
// Task function
void gif_task(void *pvParameters);

// Task priorities
#define LVGL_TIME_TASK_PRIO 3
// Task stack size
#define LVGL_TIME_STK_SIZE 512
// Task handle
TaskHandle_t LVGLTimeTask_Handler;
// Task function
void lvgl_time_task(void *pvParameters);

// Task priorities
#define vpet_info_update_PRIO 3
// Task stack size
#define WEATHER_UPDATE_STK_SIZE 256
// Task handle
TaskHandle_t WeatherUpdateTask_Handler;
// Task function
void vpet_info_update(void *pvParameters);

TaskHandle_t UARTListenerTask_Handler;
void uart_listener_task(void *pvParameters);

// Semaphores
SemaphoreHandle_t mpu6050_flag_semphr;	   // mpu6050 binary semaphore handle
SemaphoreHandle_t RTC_flag_semphr;		   // RTC real-time clock binary semaphore handle
SemaphoreHandle_t city_change_flag_semphr; // City change binary semaphore handle

uint16_t current_x_pos = 40, nowypos = 0;

// Initial Euler angles after power-on
float INIT_PITCH = 0, INIT_ROLL = 0, INIT_YAW = 0;

//==================================== Clock and Weather Related Variables ====================================//
TimeStruct esp8266_time; // This structure is only used for correction, real-time acquisition uses RTC clock
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
UserInfoStruct esp8266_userInfo;

WeatherStruct esp8266_weather;

// u8 alarm_flag = 0;                             // Alarm flag: 0 - normal mode; 1 - alarm mode

s8 cityNum = 2; // City index
// After modifying this array, remember to modify the city Chinese array in the updateDeskopCity function in setup_scr_deskop.c
// Because the Chinese array cannot be updated when using global variables
#define CITY_NUM 7 // Number of switchable cities
char cityNameArrPinYin[][20] = {"jiangsu suzhou", "shenzhen", "hong kong", "chifeng", "beijing", "shanghai", "guangzhou"};
//==================================== LVGL Related Variables ====================================//
lv_ui_init lv_gui_init;
lv_ui_deskop lv_gui_deskop;
lv_ui_alarm lv_gui_alarm;
SemaphoreHandle_t UARTSemaphore;
// Redefine printf function
// PUTCHAR_PROTOTYPE
//{
//  /* Place your implementation of fputc here */
//  /* e.g. write a character to the USART1 and Loop until the end of transmission */
//  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
//
//  return ch;
//}
FIL f;
FATFS fs;
char read_buf[512];
uint8_t BLOCK_SIZE = 512;
uint8_t Status;
uint8_t rx_buffer[100];
uint8_t rx_index = 0;
u8 button1 = 0;
u8 button2 = 0;
u8 button3 = 0;
u8 button4 = 0;
u8 touched1 = 0;
u8 touched2 = 0;
u8 placed1 = 0;
u8 changesize = 0;
int nowsize = 5;

int main(void)
{

	u8 result;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // Set system interrupt priority group 4
	delay_init(168);								// Initialize delay function

	TIM3_Int_Init(100 - 1, 840 - 1);
	uart_init(115200);
	// Timer initialization (LVGL time base), timer clock 84M, 1ms interrupt
	SD_Init();
	f_mount(&fs, "0:", 1);

	lv_init(); // lvgl initialization
	lv_port_disp_init();
	lv_port_indev_init(); // Initialize and register input device
	setup_ui_init(&lv_gui_init);
	updateInitLabel("Making-your-cube-work...");
	updateInitBar(20);
	for (int i = 0; i < 500 / LV_DISP_DEF_REFR_PERIOD; i++)
	{
		lv_timer_handler();
		delay_ms(LV_DISP_DEF_REFR_PERIOD);
	}

	usart2_init(115200); // Initialize UART2 for ESP32 communication
	usart6_init(115200); // Initialize UART6 for External communication

	Adc_Init();

	// Initialize ADC_CH22, internal temperature sensor
	// UARTSemaphore = xSemaphoreCreateBinary();
	updateInitLabel("Gathering-Info-around-you...");
	updateInitBar(40);

	for (int i = 0; i < 500 / LV_DISP_DEF_REFR_PERIOD; i++)
	{

		lv_timer_handler();

		delay_ms(LV_DISP_DEF_REFR_PERIOD);
	}
	// Initialize MPU6050
	result = mpu_dmp_init();

	// Initialize MPU6050_DMP, including initializing MPU6050
	while (result != 0)
	{
		printf("MPU6050 DMP Init Failed! Error Num = %d\r\n", result);
		delay_ms(200);
		result = mpu_dmp_init();
	}
	printf("MPU6050 DMP Init Successfully!\r\n");
	extix_init(); // Initialize external interrupt: MPU6050_INT

	updateInitLabel("Waiting-For-WiFi-Connection");
	updateInitBar(60);
	for (int i = 0; i < 500 / LV_DISP_DEF_REFR_PERIOD; i++)
	{
		lv_timer_handler();
		delay_ms(LV_DISP_DEF_REFR_PERIOD);
	}

	// esp8266_ctr_gpio_init();                                    // Initialize ESP32's EN and RST control pins, module not detected after initialization
	esp8266_sta_connect(); // Connect to network

	updateInitLabel("Connecting...");
	updateInitBar(80);
	for (int i = 0; i < 500 / LV_DISP_DEF_REFR_PERIOD; i++)
	{
		lv_timer_handler();
		delay_ms(LV_DISP_DEF_REFR_PERIOD);
	}
	esp8266_time = esp8266_gettime();								  // Get initial Beijing time
	esp8266_weather = ESP8266_GetWeather(cityNameArrPinYin[cityNum]); // Get weather
	esp8266_userInfo = ESP8266_GetUserInfo(user_name);

	My_RTC_Init(esp8266_time); // Initialize RTC
							   //  RTC_Set_AlarmA(1,23,17,0);                                   // Set alarm, setting frequency closed week, only match hours, minutes, and seconds (daily alarm)

	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits, 0); // Configure WAKE UP interrupt, 1 second interrupt once

	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

	char greetings_str[25];
	memset(greetings_str, 0, sizeof(greetings_str));
	strcat(greetings_str, "Welcome!\n");
	strcat(greetings_str, user_name);
	strcat(greetings_str, "!");
	updateInitLabel(greetings_str);
	updateInitBar(100);
	for (int i = 0; i < 500 / LV_DISP_DEF_REFR_PERIOD; i++)
	{
		lv_timer_handler();
		delay_ms(LV_DISP_DEF_REFR_PERIOD + 20); // For better visual effect
	}

	setup_ui_deskop(&lv_gui_deskop);
	printf("setup_ui_deskop success!\r\n");
	// Enter desktop
	// updateVPetPattern(1);
	// Create start task
	xTaskCreate((TaskFunction_t)start_task,			 // Task function
				(const char *)"start_task",			 // Task name
				(uint16_t)512,						 // Task stack size
				(void *)NULL,						 // Parameters passed to task function
				(UBaseType_t)START_TASK_PRIO,		 // Task priority
				(TaskHandle_t *)&StartTask_Handler); // Task handle
	vTaskStartScheduler();							 // Start task scheduler
}

// Start task function
void start_task(void *pvParameters)
{
	taskENTER_CRITICAL(); // Enter critical section

	// Create binary semaphores
	mpu6050_flag_semphr = xSemaphoreCreateBinary();
	RTC_flag_semphr = xSemaphoreCreateBinary();
	city_change_flag_semphr = xSemaphoreCreateBinary();
	// Create MPU6050 task
	xTaskCreate((TaskFunction_t)MPU6050_task,
				(const char *)"MPU6050_task",
				(uint16_t)256,
				(void *)NULL,
				(UBaseType_t)1,
				(TaskHandle_t *)&MPU6050Task_Handler);
	// Create ESP32 task
	xTaskCreate((TaskFunction_t)RTC_task,
				(const char *)"RTC_task",
				(uint16_t)256,
				(void *)NULL,
				(UBaseType_t)5,
				(TaskHandle_t *)&RTCTask_Handler);

	// Create screen dance update task
	xTaskCreate((TaskFunction_t)gif_task,
				(const char *)"gif_task",
				(uint16_t)512,
				(void *)NULL,
				(UBaseType_t)4,
				(TaskHandle_t *)&DanceTask_Handler);

	// Create weather update task
	xTaskCreate((TaskFunction_t)vpet_info_update,
				(const char *)"vpet_info_update",
				(uint16_t)512,
				(void *)NULL,
				(UBaseType_t)3,
				(TaskHandle_t *)&WeatherUpdateTask_Handler);

	// Create LVGL refresh task
	xTaskCreate((TaskFunction_t)lvgl_time_task,
				(const char *)"lvgl_time_task",
				(uint16_t)512,
				(void *)NULL,
				(UBaseType_t)3,
				(TaskHandle_t *)&LVGLTimeTask_Handler);

	xTaskCreate((TaskFunction_t)uart_listener_task,
				(const char *)"uart_listener_task",
				(uint16_t)256,
				(void *)NULL,
				(UBaseType_t)2,
				(TaskHandle_t *)&UARTListenerTask_Handler);
	vTaskDelete(StartTask_Handler); // Delete start task
	taskEXIT_CRITICAL();			// Exit critical section
}

// MPU6050 task function
void MPU6050_task(void *pvParameters)
{
	float pitch, roll, yaw; // Euler angles
	BaseType_t err = pdFALSE;
	u8 isAttitudeRecovered = 1; // State judgment flag, 0 - tilted; 1 - stable
	short aacx, aacy, aacz;		// Accelerometer raw data
	short gyrox, gyroy, gyroz;	// Gyroscope raw data
	u8 num = 0;

	while (1)
	{
		if (mpu6050_flag_semphr != NULL) // Check if semaphore is created successfully
		{
			err = xSemaphoreTake(mpu6050_flag_semphr, portMAX_DELAY); // Take semaphore
			if (err == pdTRUE)										  // Successfully took semaphore
			{
				if (mpu_dmp_get_data(&pitch, &roll, &yaw) == 0)
				{
					if ((pitch - INIT_PITCH) > MPU6050_TURN_ANGLE) // Turn left
					{
						if (isAttitudeRecovered == 1)
						{
							isAttitudeRecovered = 0;

							// printf("left pitch detected:%.3f init:%.3f", pitch, INIT_PITCH);

							// if (city_change_flag_semphr != NULL) // Detect attitude change, binary semaphore valid
							// {
							// 	xSemaphoreGive(city_change_flag_semphr); // Release binary semaphore
							// }

							if (current_x_pos > 20)
							{
								current_x_pos -= 30;
								lv_obj_set_x(lv_gui_deskop.screen_gif, current_x_pos);
							}
						}
					}
					else if ((pitch - INIT_PITCH) < -MPU6050_TURN_ANGLE) // Turn right
					{
						if (isAttitudeRecovered == 1)                                              
						{
							isAttitudeRecovered = 0;

							// 	//printf("right pitch detected:%.3f init:%.3f", pitch, INIT_PITCH);

							// 	// if (city_change_flag_semphr != NULL) // Detect attitude change, binary semaphore valid
							// 	// {
							// 	// 	xSemaphoreGive(city_change_flag_semphr); // Release binary semaphore
							// 	// }
							// }
							if (current_x_pos < 200)
							{
								current_x_pos += 30;
								lv_obj_set_x(lv_gui_deskop.screen_gif, current_x_pos);
							}
						}
					}
					else if ((roll - INIT_ROLL) > MPU6050_TURN_ANGLE) // Turn forward
					{
						if (isAttitudeRecovered == 1)
						{
							isAttitudeRecovered = 0;
							// printf("forward pitch detected:%.3f init:%.3f", roll, INIT_ROLL);
						}
					}
					else if ((roll - INIT_ROLL) < -MPU6050_TURN_ANGLE) // Turn backward
					{
						if (isAttitudeRecovered == 1)
						{
							isAttitudeRecovered = 0;
							// printf("backward pitch detected:%.3f init:%.3f", roll, INIT_ROLL);
						}
					}
					else if ((fabs(pitch - INIT_PITCH) < MPU6050_RETURN_ANGLE) && (fabs(INIT_ROLL - roll) < MPU6050_RETURN_ANGLE))
					{
						// printf("mpu recovered p%.3f ip%.3f r%.3f ir%.3f", pitch, INIT_PITCH, roll, INIT_ROLL);
						if (isAttitudeRecovered == 0)
							lv_obj_set_x(lv_gui_deskop.screen_gif, 40);

						isAttitudeRecovered = 1;
						current_x_pos = 40;
					}
				}
			}
		}

		else if (err == pdFALSE)
		{
			vTaskDelay(50);
		}
	}
}

// RTC clock update display task function
void RTC_task(void *pvParameters)
{
	BaseType_t err = pdFALSE;
	short temp_mpu; // mpu6050 temperature
	short temp_mcu; // STM32 temperature
	long gx_mpu;
	long gy_mpu;
	long gz_mpu;
	while (1)
	{
		if (RTC_flag_semphr != NULL) // Check if semaphore is created successfully
		{
			err = xSemaphoreTake(RTC_flag_semphr, portMAX_DELAY); // Take RTC semaphore, RTC wakeup interrupt releases this semaphore every 1s
			if (err == pdTRUE)									  // Successfully took semaphore
			{
				// Get RTC hours, minutes, and seconds
				RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
				// Get RTC year, month, day, and week, must read year, month, and day after reading time, otherwise it won't update (related to RTC shadow register)
				RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

				updateDeskopTime(RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes, RTC_TimeStruct.RTC_Seconds); // Update displayed time

				bool mpu_result = MPU_Get_Gyroscope(gx_mpu, gy_mpu, gz_mpu);
				temp_mpu = MPU_Get_Temperature() / 100; // Get mpu6050 temperature
				temp_mcu = Get_Temprate() / 100;		// Get mcu temperature
				printf("temp_mcu=%02d    temp_mpu=%02d\r\n", temp_mcu, temp_mpu);
				printf("res%s gx_mpu=%d    gy_mpu=%d    gz_mpu=%d\r\n", (mpu_result) ? "200" : "500", gx_mpu, gy_mpu, gz_mpu);
			}
		}
		else
		{
			vTaskDelay(500);
		}
	}
}
u16 waiting[50] = {};
u8 waiting_index = 0;
u8 toggle_status = 0;
// Dance character update task function
void gif_task(void *pvParameters)
{
	u8 num = 0;
	
	
	// updateVPetPattern(num++);
	u8 arraynum = 0;
	u8 finished = 1;
	u8 status = 0;

	u8 changedsize = 0;
	u8 leftwalk = 0;
	u8 rightwalk = 0;
	int r = 50;
	u8 count = 0;
	if(lv_gui_deskop.screen_gif->h_layout == 0 && lv_gui_deskop.screen_gif->w_layout == 0){
				SD_Init();
				f_mount(&fs, "0:", 1);
			}
	
	while (1)
	{

		if(count == 10){

		r = rand() % 100;
		count = 0;
		}
		
		
		if(rightwalk == 1 && r >= 90 && leftwalk == 0){
			arraynum = 6;
			if(finished == 1){
				num = 0;
				finished = 0;
				rightwalk = 0;
				leftwalk = 1;
			}
		}
		else if(r >= 95 && leftwalk == 0){
			arraynum = 6;
			if(finished == 1){
				num = 0;
				finished = 0;
				leftwalk = 1;
			}	

		}
		
		if(leftwalk == 1 && r <= 10 && rightwalk == 0){
			arraynum = 7;
			if(finished == 1){
				num = 0;
				finished = 0;
				rightwalk = 1;
				leftwalk = 0;
			}
		}
		else if(r <= 5 && rightwalk == 0){
			arraynum = 7;
			if(finished == 1){
				num = 0;
				finished = 0;
				rightwalk = 1;
			}

		}

		status = updateVPetPattern(&num, arraynum);
		num++;
		if (status && status != 2)
		{
			finished = 1;
			num = 0;
		}
		else if (status == 2)
		{
			finished = 1;
		}

		// printf("gif num=%d\r\n", num);
		if (button1 == 1)
		{
			printf("button1 pressed\r\n");
			arraynum = 6;
			button1 = 0;
			if (finished == 1)
			{
				num = 0;
				finished = 0;
			}
		}
		else if (button2 == 1)
		{
			printf("button2 pressed\r\n");
			arraynum = 7;
			button2 = 0;
			if (finished == 1)
			{
				finished = 0;
				num = 0;
			}
		}
		else if (button3 == 1)
		{
			printf("button3 pressed\r\n");
			if(toggle_status){
				lv_obj_clear_flag(lv_gui_deskop.screen_coins,LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(lv_gui_deskop.screen_hunger,LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(lv_gui_deskop.screen_exp,LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(lv_gui_deskop.screen_energy,LV_OBJ_FLAG_HIDDEN);
				toggle_status = 0;
			}else{
				lv_obj_add_flag(lv_gui_deskop.screen_coins,LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(lv_gui_deskop.screen_hunger,LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(lv_gui_deskop.screen_exp,LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(lv_gui_deskop.screen_energy,LV_OBJ_FLAG_HIDDEN);
				toggle_status = 1 ;
			}



			arraynum = 0;
			button3 = 0;
			if (finished == 1)
			{
				finished = 0;
				num = 0;
			}
		 	}
		else if (button4 == 1)
		{
			printf("button4 pressed\r\n");
			arraynum = 4;
			button4 = 0;
			lv_obj_add_flag(lv_gui_deskop.screen_coins,LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(lv_gui_deskop.screen_hunger,LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(lv_gui_deskop.screen_exp,LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(lv_gui_deskop.screen_energy,LV_OBJ_FLAG_HIDDEN);
			if (finished == 1)
			{
				finished = 0;
				num = 0;
			}
		}
		else if(touched1 == 1){
			printf("touched1 pressed\r\n");
			arraynum = 5;
			touched1 = 0;
			if(finished == 1){
				finished = 0;
				num = 0;
			}
		}
		else if(touched2 == 1){
			printf("touched2 pressed\r\n");
			arraynum = 8;
			touched2 = 0;
			if(finished == 1){
				finished = 0;
				num = 0;
			}
		}
		else if(placed1 == 1){
			printf("placed1 pressed\r\n");
			arraynum = 9;
			placed1 = 0;
			if(finished == 1){
				finished = 0;
				num = 0;
			}
		}
		else{
			arraynum = 0;
		}
		
		
		count++;
		vTaskDelay(150);
	}
}

	// Weather update task function
	void vpet_info_update(void *pvParameters)
	{
		BaseType_t err = pdFALSE;
		while (1)
		{
			if (err == pdFALSE)
				{	
					printf("vpet_info_update starting\r\n");
					err = pdTRUE;
					vTaskDelay(15000);
				}

		printf("vpet_info_update loop\r\n");
			
			
			printf("vpet_info_update get user info\r\n");
			taskENTER_CRITICAL();
			esp8266_userInfo = ESP8266_GetUserInfo(user_name);
			taskEXIT_CRITICAL();
			printf("vpet_info_update get info\r\n");
			updateDesktopUserInfo(esp8266_userInfo.coins, esp8266_userInfo.hunger, esp8266_userInfo.exp, esp8266_userInfo.energy);
			
			
//			delay_ms(10000);
			vTaskDelay(20000);
			
		}
	}

	// LVGL refresh task
	void lvgl_time_task(void *pvParameters)
	{
		while (1)
		{
			lv_task_handler();
			vTaskDelay(5);
		}
	}

	extern u8 receive_end;

	void uart_listener_task(void *pvParameters)
	{
		while (1)
		{

			if (receive_end)
			{

				if (USART6_RX_BUF[0] == 'B')
				{
					switch (USART6_RX_BUF[1])
					{
					case '1':
						button1 = 1;
						break;
					case '2':
						button2 = 1;
						break;
					case '3':
						button3 = 1;
						break;
					case '4':
						button4 = 1;
						break;
					}
					for (int i = 0; i < 3; i++)
					{
						USART6_RX_BUF[i] = 0;
					}
				}
				if (USART6_RX_BUF[0] == 'T')
				{	
					switch(USART6_RX_BUF[1]){
						case '1':
							touched1 = 1;
							break;
						case '2':
							touched2 = 1;
							break;
					}
					for (int i = 0; i < 3; i++)
					{
						USART6_RX_BUF[i] = 0;
					}
				}
					if(USART6_RX_BUF[0] == 'P'){
						placed1 = 1;
						for(int i = 0; i < 3; i++){
							USART6_RX_BUF[i] = 0;
						}
					}
				

				// if(USART6_RX_BUF[0] == 'M'){
				// 	changesize = USART6_RX_BUF[1] - '0';
				// 		nowsize = nowsize + changesize;

				// 		if(nowsize > 5){
				// 			nowsize = 5;
				// 		}

				// 	for(int i = 0; i < 3; i++){
				// 		USART6_RX_BUF[i] = 0;
				// 	}}
				// }
				// if(USART6_RX_BUF[0] == 'F'){
				// 	changesize = USART6_RX_BUF[1] - '0';
				// 		nowsize = nowsize - changesize;

				// 		if(nowsize < 0){
				// 			nowsize = 0;
				// 		}

				// 	for(int i = 0; i < 3; i++){
				// 		USART6_RX_BUF[i] = 0;
				// 	}
				receive_end = 0;
			}

			vTaskDelay(100);
		}

		// Wait for data to be received
	}

	void process_uart_data(uint8_t * data, uint8_t length)
	{
		// Implement your data processing logic here
		// For example, you can print the received data
		printf("Received UART data: ");
		for (uint8_t i = 0; i < length; i++)
		{
			printf("%02X ", data[i]);
		}
		printf("\n");
	}
