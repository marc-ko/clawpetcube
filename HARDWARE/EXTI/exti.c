#include "exti.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "usart.h"

// Binary semaphore handle
extern SemaphoreHandle_t mpu6050_flag_semphr; // mpu6050 binary semaphore handle

// External interrupt 5 service routine
void EXTI9_5_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;

	if (EXTI_GetITStatus(EXTI_Line5) == SET) //  Check if the interrupt is from MPU6050's INT
	{
		// Release the binary semaphore
		if (mpu6050_flag_semphr != NULL) // Data received and binary semaphore is valid
		{
			xSemaphoreGiveFromISR(mpu6050_flag_semphr, &xHigherPriorityTaskWoken); // Release the binary semaphore
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);						   // Perform a task switch if necessary
		}
		EXTI_ClearITPendingBit(EXTI_Line5); // Clear the interrupt flag on LINE5
	}
}

// External interrupt initialization routine
// Initialize PB5 as interrupt input
void extix_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  // Enable GPIOB clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // Enable SYSCFG clock

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	   // GPIO Input
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   // Pull-up
	GPIO_Init(GPIOB, &GPIO_InitStructure);			   // Initialize GPIOPB5

	EXTI_ClearITPendingBit(EXTI_Line5); // Clear the interrupt flag on LINE5

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource5); // Connect PB5 to EXTI src 5

	/* Config EXTI_Line5 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;				// LINE5
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// Set to interrupt mode
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // // Falling edge trigger
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;		  // External interrupt 9_5
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; // Preemption priority 5
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // Subpriority 0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // Enable external interrupt channel
	NVIC_Init(&NVIC_InitStructure);							  // Check if the interrupt is from MPU6050's INT
}
