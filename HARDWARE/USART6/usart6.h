#ifndef __USART6_H
#define __USART6_H
#include "sys.h"

#define USART6_MAX_RECV_LEN 1500 // Maximum receive buffer size in bytes
#define USART6_MAX_SEND_LEN 1500 // Maximum send buffer size in bytes
#define USART6_RX_EN 1           // 0: Do not receive; 1: Receive

extern u8 USART6_RX_BUF[USART6_MAX_RECV_LEN]; // Receive buffer, maximum USART6_MAX_RECV_LEN bytes
extern u8 USART6_TX_BUF[USART6_MAX_SEND_LEN]; // Send buffer, maximum USART6_MAX_SEND_LEN bytes
extern u16 USART6_RX_STA;                     // Receive data status
extern u8 receive_end;

void usart6_init(u32 bound);    // Initialize USART6
void u2_printf(char *fmt, ...); // USART6 printf function
#endif