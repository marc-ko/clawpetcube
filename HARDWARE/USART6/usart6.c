#include "delay.h"
#include "usart6.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"
#include "sys.h"

#include "FreeRTOS.h" //Use FreeRTOS

// UART transmit buffer
u8 USART6_TX_BUF[USART6_MAX_SEND_LEN] __attribute__((aligned(8))); // Transmit buffer, maximum USART6_MAX_SEND_LEN bytes
#ifdef USART6_RX_EN												   // If reception is enabled
// UART receive buffer
u8 USART6_RX_BUF[USART6_MAX_RECV_LEN]; // Receive buffer, maximum USART6_MAX_RECV_LEN bytes.
u8 receive_end = 0;

// By judging whether the time difference between receiving two consecutive characters is not more than 100ms, it is decided whether it is a continuous data.
// If the interval between receiving two characters exceeds 100ms, it is considered not continuous data. That is, if no data is received for more than 100ms,
// it indicates that the reception is complete.
// Received data status
// [15]: 0, no data received; 1, a batch of data received.
// [14:0]: length of received data
u16 USART6_RX_STA = 0;
u8 u6_count = 0;
u8 isbutton = 0;
u8 istouch = 0;
u8 ismoved = 0;
u8 isFmoved = 0;
u8 isplace = 0;
void USART6_IRQHandler(void)
{
	u8 Res;
	if (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
	{
		Res = USART_ReceiveData(USART6); //(USART1->DR);	// read rx data
		if ((USART_RX_STA & 0x8000) == 0) // rx not yet done
		{
			if (USART_RX_STA & 0x4000) // received 0x0d
			{
				if (Res != 0x0a)
					USART_RX_STA = 0; // wrong rx data, restart
				else
					USART_RX_STA |= 0x8000; // rx done
			}
			else // not received 0X0D yet
			{
				if (Res == 0x0d)
					USART_RX_STA |= 0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
						USART_RX_STA = 0; // rx data too long, restart
				}
			}
		}
		USART6_RX_BUF[u6_count++] = Res;

		if(isbutton){
			isbutton = 0;
			u6_count = 0;
			receive_end = 1;
			
		}
		
		if(Res == 'B'){
			isbutton = 1;
		}

		
		if(istouch){
			istouch = 0;
			u6_count = 0;
			receive_end = 1;
			
		}
		if(Res == 'T'){
			istouch = 1;
			
		}
		if(isplace){
			isplace = 0;
			u6_count = 0;
			receive_end = 1;
			
		}

		if(Res == 'P'){
			isplace = 1;
		}
		// if(ismoved){
		// 	ismoved = 0;
		// 	u6_count = 0;
		// 	receive_end = 1;
			
		// }
		// if(Res == 'M'){
		// 	ismoved = 1;
		// }
		// if(isFmoved){
		// 	isFmoved = 0;
		// 	u6_count = 0;
		// 	receive_end = 1;
			
		// }
		// if(Res == 'F'){
		// 	isFmoved = 1;
		// }
		printf("Received UART data: %c\n", Res);
		
	}
}
#endif
// Initialize IO UART2
// bound: baud rate
void usart6_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);  // Can GPIOC Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); // Can USART6 CLK

	// uart 1 remapping
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6); // GPIOC6 do as USART6
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6); // GPIOC7 do as USART6

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	  // AF Mode to replace pin
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Speed 50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	  //  GPIO_OType_PP to do replace pin output
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	  // Pull up
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// USART6 init setting
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 8b data format
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;								//  use parity no to check bit
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // no hardware flow control
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// Rec Trans Mode
	USART_Init(USART6, &USART_InitStructure);
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART6, ENABLE);

	// USART6 NVIC settings
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn; // UART6 interrupt channel
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; // sub prior??
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); // init VIC register

	USART6_RX_STA = 0; // Clear
}

// UART2 printf function
// Ensure that the data sent at one time does not exceed USART6_MAX_SEND_LEN bytes
void u6_printf(char *fmt, ...)
{
	u16 i, j;
	va_list ap;
	va_start(ap, fmt);
	vsprintf((char *)USART6_TX_BUF, fmt, ap);
	va_end(ap);
	i = strlen((const char *)USART6_TX_BUF); // Length of data to be sent this time
	for (j = 0; j < i; j++)					 // Loop to send data
	{
		while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET)
			;											   // Wait for the last transmission to complete
		USART_SendData(USART6, (uint8_t)USART6_TX_BUF[j]); // Send data to USART6
	}
}
