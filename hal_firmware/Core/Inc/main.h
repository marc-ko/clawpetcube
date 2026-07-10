#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "app_config.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;
extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern I2C_HandleTypeDef hi2c1;
extern SD_HandleTypeDef hsd;
extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim7;
extern ADC_HandleTypeDef hadc1;

#define LCD_BLK_GPIO_Port GPIOB
#define LCD_BLK_Pin GPIO_PIN_0
#define LCD_CS_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_1
#define LCD_RES_GPIO_Port GPIOC
#define LCD_RES_Pin GPIO_PIN_4
#define LCD_DC_GPIO_Port GPIOC
#define LCD_DC_Pin GPIO_PIN_5
#define MPU6050_INT_GPIO_Port GPIOB
#define MPU6050_INT_Pin GPIO_PIN_5

void Error_Handler(void);
void SystemClock_Config(void);

#ifdef __cplusplus
}
#endif

#endif
