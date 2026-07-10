#include "app_debug.h"
#include <stdio.h>
#include <string.h>

static volatile uint8_t debug_ready;

void Debug_Init(void)
{
    debug_ready = 1;
    Debug_Printf("\r\n[%s] %s\r\n", APP_FW_NAME, APP_FW_VERSION);
}

void Debug_Write(const char *data, uint16_t len)
{
    if (!debug_ready || data == NULL || len == 0U) {
        return;
    }

    HAL_UART_Transmit(&huart1, (const uint8_t *)data, len, 100U);
}

void Debug_Printf(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    int len;

    va_start(ap, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (len <= 0) {
        return;
    }
    if (len > (int)sizeof(buf)) {
        len = sizeof(buf);
    }
    Debug_Write(buf, (uint16_t)len);
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    if (len > 0) {
        Debug_Write(ptr, (uint16_t)len);
    }
    return len;
}
