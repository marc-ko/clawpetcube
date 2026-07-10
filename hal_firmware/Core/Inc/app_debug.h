#ifndef APP_DEBUG_H
#define APP_DEBUG_H

#include "main.h"
#include <stdarg.h>

void Debug_Init(void);
void Debug_Printf(const char *fmt, ...);
void Debug_Write(const char *data, uint16_t len);

#endif
