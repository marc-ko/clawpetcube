#ifndef USART_COMPAT_HAL_H
#define USART_COMPAT_HAL_H

#include "sys.h"
#include <stdarg.h>

#define USART_REC_LEN APP_DEBUG_RX_LEN
#define EN_USART1_RX 1
#define USART2_MAX_RECV_LEN APP_ESP8266_RX_LEN
#define USART2_MAX_SEND_LEN APP_ESP8266_RX_LEN
#define USART2_RX_EN 1
#define USART6_MAX_RECV_LEN APP_USART6_RX_LEN
#define USART6_MAX_SEND_LEN APP_USART6_RX_LEN
#define USART6_RX_EN 1

extern u8 USART_RX_BUF[USART_REC_LEN];
extern u16 USART_RX_STA;
extern u8 USART2_RX_BUF[USART2_MAX_RECV_LEN];
extern u8 USART2_TX_BUF[USART2_MAX_SEND_LEN];
extern u16 USART2_RX_STA;
extern u8 USART6_RX_BUF[USART6_MAX_RECV_LEN];
extern u8 USART6_TX_BUF[USART6_MAX_SEND_LEN];
extern u16 USART6_RX_STA;
extern u8 receive_end;

void uart_init(u32 bound);
void usart2_init(u32 bound);
void usart6_init(u32 bound);
void u2_printf(char *fmt, ...);
void u6_printf(char *fmt, ...);
void USART_Compat_StartRx(void);
void USART_Compat_OnRx(UART_HandleTypeDef *huart);
void USART2_MarkFrameDone(void);

#endif
