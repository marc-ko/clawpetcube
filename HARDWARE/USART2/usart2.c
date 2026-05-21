#include "delay.h"
#include "usart2.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"

// UART transmit buffer
u8 USART2_TX_BUF[USART2_MAX_SEND_LEN] __attribute__((aligned(8))); // Transmit buffer, maximum USART2_MAX_SEND_LEN bytes
#ifdef USART2_RX_EN												   // If reception is enabled
// UART receive buffer
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; // Receive buffer, maximum USART2_MAX_RECV_LEN bytes.

// By judging whether the time difference between receiving two consecutive characters is not more than 100ms, it is decided whether it is a continuous data.
// If the interval between receiving two characters exceeds 100ms, it is considered not continuous data. That is, if no data is received for more than 100ms,
// it indicates that the reception is complete.
// Received data status
// [15]: 0, no data received; 1, a batch of data received.
// [14:0]: length of received data
u16 USART2_RX_STA = 0;
void USART2_IRQHandler(void)
{
	u8 res;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) // Data received
	{
		res = USART_ReceiveData(USART2);
		if ((USART2_RX_STA & (1 << 15)) == 0) // If the received batch of data has not been processed, do not receive other data
		{
			if (USART2_RX_STA < USART2_MAX_RECV_LEN) // Can still receive data
			{
				TIM_SetCounter(TIM7, 0); // Clear counter
				if (USART2_RX_STA == 0)
					TIM_Cmd(TIM7, ENABLE);			  // Enable timer 7
				USART2_RX_BUF[USART2_RX_STA++] = res; // Record received value
			}
			else
			{
				USART2_RX_STA |= 1 << 15; // Force mark reception complete
			}
		}
	}
}
#endif

// Initialize IO UART2
// bound: baud rate
void usart2_init(u32 bound)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // Enable GPIOA clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); // Enable USART2 clock

	USART_DeInit(USART2); // Reset USART2

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // GPIOA2 alternate function as USART2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); // GPIOA3 alternate function as USART2

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; // Initialize GPIOA2 and GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		   // Alternate function
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   // Speed 50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		   // Push-pull alternate function
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		   // Pull-up
	GPIO_Init(GPIOA, &GPIO_InitStructure);				   // Initialize GPIOA2 and GPIOA3

	USART_InitStructure.USART_BaudRate = bound;										// Baud rate
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// Word length 8 bits
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// One stop bit
	USART_InitStructure.USART_Parity = USART_Parity_No;								// No parity
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // No hardware flow control
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// Receive and transmit mode

	USART_Init(USART2, &USART_InitStructure); // Initialize USART2

	USART_Cmd(USART2, ENABLE); // Enable USART2

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // Enable interrupt

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4; // Preemption priority 4
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // Subpriority 0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // Enable IRQ channel
	NVIC_Init(&NVIC_InitStructure);							  // Initialize VIC register with specified parameters
	TIM7_Int_Init(1000 - 1, 8400 - 1);						  // 100ms interrupt
	USART2_RX_STA = 0;										  // Clear
	TIM_Cmd(TIM7, DISABLE);									  // Disable timer 7
}

// UART2 printf function
// Ensure that the data sent at one time does not exceed USART2_MAX_SEND_LEN bytes
void u2_printf(char *fmt, ...)
{
	u16 i, j;
	va_list ap;
	va_start(ap, fmt);
	vsprintf((char *)USART2_TX_BUF, fmt, ap);
	va_end(ap);
	i = strlen((const char *)USART2_TX_BUF); // Length of data to be sent this time
	for (j = 0; j < i; j++)					 // Loop to send data
	{
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			;											   // Wait for the last transmission to complete
		USART_SendData(USART2, (uint8_t)USART2_TX_BUF[j]); // Send data to USART2
	}
}