#ifndef __USART2_H
#define __USART2_H	 
#include "sys.h"  

#define USART2_MAX_RECV_LEN  1500  // Maximum receive buffer size in bytes
#define USART2_MAX_SEND_LEN  1500  // Maximum send buffer size in bytes
#define USART2_RX_EN         1     // 0: Do not receive; 1: Receive

extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN];  // Receive buffer, maximum USART2_MAX_RECV_LEN bytes
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN];  // Send buffer, maximum USART2_MAX_SEND_LEN bytes
extern u16 USART2_RX_STA;                       // Receive data status

void usart2_init(u32 bound);                    // Initialize USART2
void TIM7_Int_Init(u16 arr, u16 psc);           // Initialize TIM7 interrupt
void u2_printf(char* fmt, ...);                 // USART2 printf function
#endif