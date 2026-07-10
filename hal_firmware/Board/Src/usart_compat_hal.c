#include "usart_compat_hal.h"
#include <stdio.h>
#include <string.h>

u8 USART_RX_BUF[USART_REC_LEN];
u16 USART_RX_STA;
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN];
u8 USART2_TX_BUF[USART2_MAX_SEND_LEN];
u16 USART2_RX_STA;
u8 USART6_RX_BUF[USART6_MAX_RECV_LEN];
u8 USART6_TX_BUF[USART6_MAX_SEND_LEN];
u16 USART6_RX_STA;
u8 receive_end;

static uint8_t uart1_rx_byte;
static uint8_t uart2_rx_byte;
static uint8_t uart6_rx_byte;
static uint16_t usart6_index;
static uint8_t usart6_pending_prefix;

void uart_init(u32 bound)
{
    (void)bound;
}

void usart2_init(u32 bound)
{
    (void)bound;
}

void usart6_init(u32 bound)
{
    (void)bound;
}

static void compat_vprintf(UART_HandleTypeDef *huart, uint8_t *tx_buf, size_t tx_len, char *fmt, va_list ap)
{
    int len = vsnprintf((char *)tx_buf, tx_len, fmt, ap);
    if (len <= 0) {
        return;
    }
    if ((size_t)len > tx_len) {
        len = (int)tx_len;
    }
    HAL_UART_Transmit(huart, tx_buf, (uint16_t)len, 1000U);
}

void u2_printf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    compat_vprintf(&huart2, USART2_TX_BUF, sizeof(USART2_TX_BUF), fmt, ap);
    va_end(ap);
}

void u6_printf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    compat_vprintf(&huart6, USART6_TX_BUF, sizeof(USART6_TX_BUF), fmt, ap);
    va_end(ap);
}

void USART_Compat_StartRx(void)
{
    HAL_UART_Receive_IT(&huart1, &uart1_rx_byte, 1);
    HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1);
    HAL_UART_Receive_IT(&huart6, &uart6_rx_byte, 1);
}

void USART2_MarkFrameDone(void)
{
    USART2_RX_STA |= 1U << 15;
}

static void uart1_push(uint8_t b)
{
    if ((USART_RX_STA & 0x8000U) != 0U) {
        return;
    }
    if ((USART_RX_STA & 0x4000U) != 0U) {
        if (b == '\n') {
            USART_RX_STA |= 0x8000U;
        } else {
            USART_RX_STA = 0;
        }
    } else if (b == '\r') {
        USART_RX_STA |= 0x4000U;
    } else {
        USART_RX_BUF[USART_RX_STA & 0x3FFFU] = b;
        USART_RX_STA++;
        if (USART_RX_STA > (USART_REC_LEN - 1U)) {
            USART_RX_STA = 0;
        }
    }
}

static void uart2_push(uint8_t b)
{
    if ((USART2_RX_STA & 0x8000U) != 0U) {
        return;
    }
    if ((USART2_RX_STA & 0x7FFFU) < USART2_MAX_RECV_LEN) {
        USART2_RX_BUF[USART2_RX_STA & 0x7FFFU] = b;
        USART2_RX_STA++;
        __HAL_TIM_SET_COUNTER(&htim7, 0);
        HAL_TIM_Base_Start_IT(&htim7);
    } else {
        USART2_RX_STA |= 1U << 15;
    }
}

static void uart6_push(uint8_t b)
{
    if (usart6_index < USART6_MAX_RECV_LEN) {
        USART6_RX_BUF[usart6_index++] = b;
    }

    if (usart6_pending_prefix != 0U) {
        usart6_pending_prefix = 0;
        usart6_index = 0;
        receive_end = 1;
        return;
    }

    if (b == 'B' || b == 'T' || b == 'P') {
        usart6_pending_prefix = b;
    }
}

void USART_Compat_OnRx(UART_HandleTypeDef *huart)
{
    if (huart == &huart1) {
        uart1_push(uart1_rx_byte);
        HAL_UART_Receive_IT(&huart1, &uart1_rx_byte, 1);
    } else if (huart == &huart2) {
        uart2_push(uart2_rx_byte);
        HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1);
    } else if (huart == &huart6) {
        uart6_push(uart6_rx_byte);
        HAL_UART_Receive_IT(&huart6, &uart6_rx_byte, 1);
    }
}
