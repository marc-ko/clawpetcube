#include "sys.h"
#include "usart.h"
/******************************************************************* */
// if use ucos, jsut include the lib under there
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h" //Use FreeRTOS
#endif
/******************************************************************* */

// Add the code down there to support back printf, don use MicroLIB
#ifdef __GNUC__
int __io_putchar(int ch)
{
	while ((USART1->SR & 0X40) == 0)
		; // it keep looping until it ends
	USART1->DR = (u8)ch;
	return ch;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		__io_putchar(*ptr++);
	}

	return len;
}
#else
int fputc(int ch, FILE *f)
{
	while ((USART1->SR & 0X40) == 0)
		; // it keep looping until it ends
	USART1->DR = (u8)ch;
	return ch;
}
#endif

#if EN_USART1_RX // Enable UART1 RX
// COM1 ended the service
//  Caution idk why but reading USARTx->SR can avoid some weird bugs
u8 USART_RX_BUF[USART_REC_LEN]; // Receive buffer , max USART_REC_LEN BYTES .
// STATUS OF RX
// bit15	Receiving success symbol
// bit14	Receiving 0x0d
// bit13~0	Received Vaild Data
u16 USART_RX_STA = 0; // Rec Status

// Init IO COM1
// bound:BAUDRATE
void uart_init(u32 bound)
{
	// GPIO Setting
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // Can GPIOA Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // Can USART1 CLK

	// uart 1 remapping
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  // GPIOA9 do as USART1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); // GPIOA10 do as USART1

	// USART1 port setting
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; // GPIOA9 and GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			// AF Mode to replace pin
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// Speed 50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//  GPIO_OType_PP to do replace pin output
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			// Pull up
	GPIO_Init(GPIOA, &GPIO_InitStructure);					// Init PA9 PA10

	// USART1 init setting
	USART_InitStructure.USART_BaudRate = bound;					// Baudrate setting
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 8b data format
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;								//  use parity no to check bit
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // no hardware flow control
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// Rec Trans Mode
	USART_Init(USART1, &USART_InitStructure);										// Init UART1

	USART_Cmd(USART1, ENABLE); // enable port

	// USART_ClearFlag(USART1, USART_FLAG_TC);

#if EN_USART1_RX
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // Use Interrupt

	// Usart1 NVIC settings
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; // UART1 interrupt channel
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; // sub prior??
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); // init VIC register

#endif
}

void USART1_IRQHandler(void) // uart interrupt
{
	u8 Res;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) // interrupt rx (the rx must bne ernded with 0x0d 0x0a)
	{
		Res = USART_ReceiveData(USART1); //(USART1->DR);	// read rx data

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
	}
}
#endif
