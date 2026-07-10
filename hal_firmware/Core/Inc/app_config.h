#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define APP_FW_NAME "VPetCube HAL"
#define APP_FW_VERSION "0.1.0-hal-migration"

#define APP_UART_BAUDRATE 115200U
#define APP_LCD_WIDTH 240U
#define APP_LCD_HEIGHT 240U

#define APP_LCD_FLIP_BY_PRISM 1
#define APP_LCD_USE_HORIZONTAL 0

#define APP_ESP8266_RX_LEN 1500U
#define APP_USART6_RX_LEN 1500U
#define APP_DEBUG_RX_LEN 200U

/*
 * Keep credentials out of git. Define these in CubeIDE project symbols or a
 * local, ignored config header if the ESP8266 client is enabled.
 */
#ifndef VPC_WIFI_SSID
#define VPC_WIFI_SSID ""
#endif

#ifndef VPC_WIFI_PASSWORD
#define VPC_WIFI_PASSWORD ""
#endif

#endif
