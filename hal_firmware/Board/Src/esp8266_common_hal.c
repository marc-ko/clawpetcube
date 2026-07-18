#include "esp8266_common_hal.h"
#include "usart_compat_hal.h"
#include "app_debug.h"
#include <stdio.h>
#include <string.h>

static const char *wifista_ssid = VPC_WIFI_SSID;
static const char *wifista_password = VPC_WIFI_PASSWORD;
static const char *time_host = "www.hko.gov.hk";
static const char *time_port = "80";
static const char *openclaw_host = VPC_OPENCLAW_HOST;
static const char *openclaw_http_host = VPC_OPENCLAW_HTTP_HOST;
static const char *openclaw_http_port = VPC_OPENCLAW_HTTP_PORT;
static const char *openclaw_token = VPC_OPENCLAW_TOKEN;
static uint8_t openclaw_tls_disabled;

static uint8_t parse_http_date_hkt(const char *rx, TimeStruct *out);
static uint8_t parse_month(const char *month);
static uint8_t parse_weekday(const char *weekday);
static uint8_t days_in_month(uint16_t year, uint8_t month);
static uint8_t is_leap_year(uint16_t year);
static uint8_t esp8266_https_get(const char *path, const char *auth_token, const char *ack, uint16_t waittime);
static uint8_t esp8266_http_get_host(const char *host, const char *port, const char *path, const char *auth_token, const char *ack, uint16_t waittime);
static void esp8266_try_optional_cmd(const char *cmd);
static void esp8266_recover_at_mode(void);
static void esp8266_soft_reset(void);
static void esp8266_close_socket(uint16_t settle_ms);
static uint8_t esp8266_start_tcp(const char *host, const char *port);
static void esp8266_log_link_status(void);
static void esp8266_log_cmd(const char *cmd);
static void esp8266_log_ack_miss(const char *rx);
static uint8_t parse_uint_after(const char *base, const char *key, uint8_t *out);
static uint8_t parse_percent_after(const char *base, const char *section, uint8_t *out);
static void parse_string_after(const char *base, const char *key, char *out, size_t out_len);

void esp8266_at_response(u8 mode)
{
    if (USART2_RX_STA & 0x8000U) {
        USART2_RX_BUF[USART2_RX_STA & 0x7FFFU] = 0;
        printf("%s", USART2_RX_BUF);
        if (mode) {
            USART2_RX_STA = 0;
        }
    }
}

u8 *esp8266_check_cmd(u8 *str)
{
    if ((USART2_RX_STA & 0x8000U) == 0U) {
        return NULL;
    }
    USART2_RX_BUF[USART2_RX_STA & 0x7FFFU] = 0;
    return (u8 *)strstr((const char *)USART2_RX_BUF, (const char *)str);
}

u8 esp8266_send_cmd(u8 *cmd, u8 *ack, u16 waittime)
{
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
    esp8266_log_cmd((const char *)cmd);
    u2_printf("%s\r\n", cmd);

    if (ack == NULL || waittime == 0U) {
        return 0;
    }

    while (waittime-- > 0U) {
        HAL_Delay(10);
        if ((USART2_RX_STA & 0x8000U) != 0U) {
            if (esp8266_check_cmd(ack) != NULL) {
                return 0;
            }
            esp8266_log_ack_miss((const char *)USART2_RX_BUF);
            USART2_RX_STA = 0;
        }
    }
    return 1;
}

u8 esp8266_send_data(u8 *data, u8 *ack, u16 waittime)
{
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
    u2_printf("%s", data);

    if (ack == NULL || waittime == 0U) {
        return 0;
    }

    while (waittime-- > 0U) {
        HAL_Delay(10);
        if ((USART2_RX_STA & 0x8000U) != 0U) {
            if (esp8266_check_cmd(ack) != NULL) {
                return 0;
            }
            USART2_RX_STA = 0;
        }
    }
    return 1;
}

u8 esp8266_quit_trans(void)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)"+++", 3, 100U);
    HAL_Delay(500);
    return esp8266_send_cmd((u8 *)"AT", (u8 *)"OK", 20);
}

u8 esp8266_consta_check(void)
{
    return esp8266_send_cmd((u8 *)"AT+CIPSTATUS", (u8 *)"STATUS", 50) == 0 ? 1 : 0;
}

void esp8266_ctr_gpio_init(void)
{
}

void esp8266_sta_connect(void)
{
    char command[160];
    uint8_t busy_count = 0U;

    esp8266_recover_at_mode();

    while (esp8266_send_cmd((u8 *)"AT", (u8 *)"OK", 100)) {
        if (strstr((const char *)USART2_RX_BUF, "busy") != NULL) {
            busy_count++;
            printf("ESP8266 busy, waiting\r\n");
            if ((busy_count % 3U) == 0U) {
                esp8266_recover_at_mode();
                esp8266_soft_reset();
            }
            HAL_Delay(2500);
        } else {
            printf("No ESP8266-12F detected\r\n");
            HAL_Delay(800);
        }
    }

    esp8266_send_cmd((u8 *)"ATE0", (u8 *)"OK", 50);
    esp8266_send_cmd((u8 *)"AT+CWMODE=1", (u8 *)"OK", 50);

    if (wifista_ssid[0] == '\0') {
        printf("WiFi credentials not configured; skipping join\r\n");
        return;
    }

    snprintf(command, sizeof(command), "AT+CWJAP_DEF=\"%s\",\"%s\"", wifista_ssid, wifista_password);
    while (esp8266_send_cmd((u8 *)command, (u8 *)"WIFI GOT IP", 1000)) {
        printf("ESP8266 WiFi join retry\r\n");
    }
    printf("ESP8266 WiFi connected\r\n");
    esp8266_send_cmd((u8 *)"AT+CIFSR", (u8 *)"STAIP", 100);
}

TimeStruct esp8266_gettime(void)
{
    TimeStruct t = {0};
    char command[96];
    const char *request =
        "HEAD / HTTP/1.1\r\n"
        "Host: www.hko.gov.hk\r\n"
        "Connection: close\r\n"
        "\r\n";

    esp8266_send_cmd((u8 *)"AT+CIPMODE=0", (u8 *)"OK", 100);
    esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
    HAL_Delay(300);

    snprintf(command, sizeof(command), "AT+CIPSTART=\"TCP\",\"%s\",%s", time_host, time_port);
    if (esp8266_send_cmd((u8 *)command, (u8 *)"CONNECT", 500)) {
        printf("Time sync TCP connect failed\r\n");
        return t;
    }

    snprintf(command, sizeof(command), "AT+CIPSEND=%u", (unsigned int)strlen(request));
    if (esp8266_send_cmd((u8 *)command, (u8 *)">", 200)) {
        printf("Time sync CIPSEND failed\r\n");
        esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
        return t;
    }

    if (esp8266_send_data((u8 *)request, (u8 *)"Date:", 500)) {
        printf("Time sync Date header missing\r\n");
        esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
        return t;
    }

    if (parse_http_date_hkt((const char *)USART2_RX_BUF, &t) == 0U) {
        printf("Time sync parse failed\r\n");
        memset(&t, 0, sizeof(t));
    } else {
        printf("HKT time: %04u-%02u-%02u %02u:%02u:%02u week %u\r\n",
               t.year, t.month, t.day, t.hour, t.minute, t.second, t.week);
    }

    esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
    return t;
}

static uint8_t parse_http_date_hkt(const char *rx, TimeStruct *out)
{
    const char *date = strstr(rx, "Date:");
    char weekday[4] = {0};
    char month[4] = {0};
    unsigned int day = 0;
    unsigned int year = 0;
    unsigned int hour = 0;
    unsigned int minute = 0;
    unsigned int second = 0;
    uint8_t dim;

    if (date == NULL || out == NULL) {
        return 0;
    }

    if (sscanf(date, "Date: %3[^,], %u %3s %u %u:%u:%u GMT",
               weekday, &day, month, &year, &hour, &minute, &second) != 7) {
        return 0;
    }

    out->week = parse_weekday(weekday);
    out->month = parse_month(month);
    out->day = (u8)day;
    out->year = (u16)year;
    out->hour = (u8)hour;
    out->minute = (u8)minute;
    out->second = (u8)second;

    if (out->week == 0U || out->month == 0U || out->day == 0U ||
        out->hour > 23U || out->minute > 59U || out->second > 59U) {
        return 0;
    }

    out->hour = (u8)(out->hour + 8U);
    if (out->hour >= 24U) {
        out->hour = (u8)(out->hour - 24U);
        out->week++;
        if (out->week > 7U) {
            out->week = 1U;
        }

        out->day++;
        dim = days_in_month(out->year, out->month);
        if (out->day > dim) {
            out->day = 1U;
            out->month++;
            if (out->month > 12U) {
                out->month = 1U;
                out->year++;
            }
        }
    }

    return 1;
}

static uint8_t parse_month(const char *month)
{
    static const char names[12][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    for (uint8_t i = 0; i < 12U; i++) {
        if (strncmp(month, names[i], 3U) == 0) {
            return (uint8_t)(i + 1U);
        }
    }
    return 0;
}

static uint8_t parse_weekday(const char *weekday)
{
    static const char names[7][4] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };

    for (uint8_t i = 0; i < 7U; i++) {
        if (strncmp(weekday, names[i], 3U) == 0) {
            return (uint8_t)(i + 1U);
        }
    }
    return 0;
}

static uint8_t days_in_month(uint16_t year, uint8_t month)
{
    static const uint8_t days[12] = {
        31U, 28U, 31U, 30U, 31U, 30U,
        31U, 31U, 30U, 31U, 30U, 31U
    };

    if (month == 2U && is_leap_year(year)) {
        return 29U;
    }
    if (month < 1U || month > 12U) {
        return 31U;
    }
    return days[month - 1U];
}

static uint8_t is_leap_year(uint16_t year)
{
    return (uint8_t)(((year % 4U) == 0U && (year % 100U) != 0U) || ((year % 400U) == 0U));
}

WeatherStruct ESP8266_GetWeather(char *city)
{
    WeatherStruct weather = {0};
    printf("Weather fetch pending for city: %s\r\n", city ? city : "");
    return weather;
}

UserInfoStruct ESP8266_GetUserInfo(char *username)
{
    UserInfoStruct user = {0};
    strncpy((char *)user.username, username ? username : "user", sizeof(user.username) - 1);
    user.coins = 100;
    user.hunger = 100;
    user.exp = 0;
    user.energy = 100;
    user.emotion = 100;
    user.status = 'i';
    user.action = ACTION_NONE;
    printf("User API pending; using local defaults for %s\r\n", user.username);
    return user;
}

OpenClawHealthStruct ESP8266_GetOpenClawHealth(void)
{
    OpenClawHealthStruct health = {0};
    const char *rx;

    if (openclaw_http_host[0] != '\0') {
        health.error_code = esp8266_http_get_host(openclaw_http_host, openclaw_http_port, "/health", NULL, "\"alive\"", 300);
    } else {
        health.error_code = esp8266_https_get("/health", NULL, "\"alive\"", 300);
    }

    if (health.error_code != 0U) {
        strncpy(health.status,
                health.error_code == 2U ? "tls-no" : (health.error_code == 3U ? "proxy" : "offline"),
                sizeof(health.status) - 1U);
        if (health.error_code == 2U) {
            printf("OpenClaw health needs HTTP proxy\r\n");
        } else if (health.error_code == 3U) {
            printf("OpenClaw health proxy TCP failed\r\n");
        } else {
            printf("OpenClaw health failed\r\n");
        }
        return health;
    }

    rx = (const char *)USART2_RX_BUF;
    health.ok = (strstr(rx, "\"ok\":true") != NULL || strstr(rx, "\"ok\": true") != NULL) ? 1U : 0U;
    parse_string_after(rx, "\"status\"", health.status, sizeof(health.status));
    if (health.status[0] == '\0') {
        strncpy(health.status, health.ok ? "alive" : "unknown", sizeof(health.status) - 1U);
    }
    printf("OpenClaw health: %s\r\n", health.status);
    return health;
}

OpenClawStatusStruct ESP8266_GetOpenClawStatus(void)
{
    OpenClawStatusStruct status = {0};
    const char *rx;
    const char *gateway;
    const char *process;
    const char *cron;

    if (openclaw_token[0] == '\0') {
        printf("OpenClaw token missing\r\n");
        return status;
    }

    if (openclaw_http_host[0] != '\0') {
        status.error_code = esp8266_http_get_host(openclaw_http_host, openclaw_http_port, "/status", NULL, "\"cron_jobs\"", 4500);
    } else {
        status.error_code = esp8266_https_get("/status", openclaw_token, "\"cron_jobs\"", 4500);
    }

    if (status.error_code != 0U) {
        if (status.error_code == 2U) {
            printf("OpenClaw status needs HTTP proxy\r\n");
        } else if (status.error_code == 3U) {
            printf("OpenClaw status proxy TCP failed\r\n");
        } else {
            printf("OpenClaw status failed\r\n");
        }
        return status;
    }

    rx = (const char *)USART2_RX_BUF;
    gateway = strstr(rx, "\"gateway\"");
    process = strstr(rx, "\"process\"");
    cron = strstr(rx, "\"cron_jobs\"");

    status.gateway_ok = (gateway != NULL &&
                         (strstr(gateway, "\"ok\":true") != NULL ||
                          strstr(gateway, "\"ok\": true") != NULL)) ? 1U : 0U;
    if (process != NULL) {
        parse_uint_after(process, "\"count\"", &status.process_count);
    }
    if (cron != NULL) {
        parse_uint_after(cron, "\"total\"", &status.cron_total);
        parse_uint_after(cron, "\"ok\"", &status.cron_ok);
        parse_uint_after(cron, "\"failed\"", &status.cron_failed);
    }
    parse_percent_after(rx, "\"disk\"", &status.disk_percent);
    parse_percent_after(rx, "\"memory\"", &status.memory_percent);
    parse_string_after(rx, "\"uptime\"", status.uptime, sizeof(status.uptime));
    parse_string_after(rx, "\"timestamp\"", status.timestamp, sizeof(status.timestamp));

    status.ok = status.gateway_ok;
    printf("OpenClaw status: gw=%u proc=%u cron=%u/%u disk=%u mem=%u\r\n",
           status.gateway_ok, status.process_count, status.cron_ok,
           status.cron_total, status.disk_percent, status.memory_percent);
    return status;
}

OpenClawMessageStruct ESP8266_GetOpenClawMessage(void)
{
    OpenClawMessageStruct msg = {0};
    const char *rx;

    if (openclaw_http_host[0] != '\0') {
        msg.error_code = esp8266_http_get_host(openclaw_http_host, openclaw_http_port, "/message", NULL, "\"timestamp\"", 500);
    } else {
        msg.error_code = esp8266_https_get("/message", NULL, "\"timestamp\"", 500);
    }

    if (msg.error_code != 0U) {
        if (msg.error_code == 2U) {
            printf("OpenClaw message needs HTTP proxy\r\n");
        } else if (msg.error_code == 3U) {
            printf("OpenClaw message proxy TCP failed\r\n");
        } else {
            printf("OpenClaw message failed\r\n");
        }
        return msg;
    }

    rx = (const char *)USART2_RX_BUF;
    parse_string_after(rx, "\"message\"", msg.message, sizeof(msg.message));
    parse_string_after(rx, "\"from\"", msg.from, sizeof(msg.from));
    parse_string_after(rx, "\"timestamp\"", msg.timestamp, sizeof(msg.timestamp));

    msg.ok = (msg.timestamp[0] != '\0' && msg.message[0] != '\0') ? 1U : 0U;
    printf("OpenClaw message: %s\r\n", msg.ok ? "received" : "empty");
    return msg;
}

static uint8_t esp8266_https_get(const char *path, const char *auth_token, const char *ack, uint16_t waittime)
{
    char command[96];
    char sni_command[96];
    char request[512];
    int request_len;

    if (openclaw_tls_disabled != 0U) {
        return 2U;
    }

    esp8266_send_cmd((u8 *)"AT+CIPMODE=0", (u8 *)"OK", 100);
    esp8266_send_cmd((u8 *)"AT+CIPMUX=0", (u8 *)"OK", 100);
    esp8266_try_optional_cmd("AT+CIPSSLSIZE=4096");
    esp8266_try_optional_cmd("AT+CIPSSLCCONF=0");
    esp8266_try_optional_cmd("AT+CIPSSLCCONF=0,0,0");
    snprintf(sni_command, sizeof(sni_command), "AT+CIPSSLCSNI=\"%s\"", openclaw_host);
    esp8266_try_optional_cmd(sni_command);
    esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
    HAL_Delay(300);

    snprintf(command, sizeof(command), "AT+CIPSTART=\"SSL\",\"%s\",443", openclaw_host);
    if (esp8266_send_cmd((u8 *)command, (u8 *)"CONNECT", 800)) {
        openclaw_tls_disabled = 1U;
        return 2U;
    }

    if (auth_token != NULL && auth_token[0] != '\0') {
        request_len = snprintf(request, sizeof(request),
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "Authorization: Bearer %s\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               path, openclaw_host, auth_token);
    } else {
        request_len = snprintf(request, sizeof(request),
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               path, openclaw_host);
    }

    if (request_len <= 0 || request_len >= (int)sizeof(request)) {
        esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
        return 1U;
    }

    snprintf(command, sizeof(command), "AT+CIPSEND=%u", (unsigned int)request_len);
    if (esp8266_send_cmd((u8 *)command, (u8 *)">", 300)) {
        esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
        return 1U;
    }

    if (esp8266_send_data((u8 *)request, (u8 *)ack, waittime)) {
        esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
        return 1U;
    }

    return 0U;
}

static void esp8266_try_optional_cmd(const char *cmd)
{
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
    esp8266_log_cmd(cmd);
    u2_printf("%s\r\n", (char *)cmd);

    for (uint16_t wait = 0; wait < 80U; wait++) {
        HAL_Delay(10);
        if ((USART2_RX_STA & 0x8000U) != 0U) {
            USART2_RX_BUF[USART2_RX_STA & 0x7FFFU] = 0;
            if (strstr((const char *)USART2_RX_BUF, "OK") != NULL) {
                return;
            }
            if (strstr((const char *)USART2_RX_BUF, "ERROR") != NULL) {
                printf("ESP8266 opt unsupported\r\n");
                return;
            }
            USART2_RX_STA = 0;
        }
    }
}

static void esp8266_recover_at_mode(void)
{
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
    printf("ESP8266 AT recovery\r\n");
    HAL_Delay(1000);
    HAL_UART_Transmit(&huart2, (uint8_t *)"+++", 3, 100U);
    HAL_Delay(1200);
    HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 100U);
    HAL_Delay(500);
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
}

static void esp8266_soft_reset(void)
{
    printf("ESP8266 soft reset\r\n");
    (void)esp8266_send_cmd((u8 *)"AT+RST", (u8 *)"ready", 800);
    HAL_Delay(2500);
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
}

static void esp8266_close_socket(uint16_t settle_ms)
{
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
    esp8266_log_cmd("AT+CIPCLOSE");
    u2_printf("AT+CIPCLOSE\r\n");

    for (uint16_t wait = 0U; wait < 200U; wait++) {
        HAL_Delay(10);
        if ((USART2_RX_STA & 0x8000U) != 0U) {
            USART2_RX_BUF[USART2_RX_STA & 0x7FFFU] = 0;
            if (strstr((const char *)USART2_RX_BUF, "OK") != NULL ||
                strstr((const char *)USART2_RX_BUF, "ERROR") != NULL ||
                strstr((const char *)USART2_RX_BUF, "CLOSED") != NULL) {
                break;
            }
            USART2_RX_STA = 0;
        }
    }

    HAL_Delay(settle_ms);
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
}

static void esp8266_log_cmd(const char *cmd)
{
    if (cmd == NULL) {
        return;
    }

    if (strncmp(cmd, "AT+CWJAP", 8) == 0) {
        printf("ESP8266 cmd: AT+CWJAP=<local>\r\n");
    } else if (strncmp(cmd, "AT+CIPSTART", 11) == 0) {
        printf("ESP8266 cmd: AT+CIPSTART=<configured>\r\n");
    } else if (strncmp(cmd, "AT+CIPSSLCSNI", 13) == 0) {
        printf("ESP8266 cmd: AT+CIPSSLCSNI=<configured>\r\n");
    } else {
        printf("ESP8266 cmd: %s\r\n", cmd);
    }
}

static void esp8266_log_ack_miss(const char *rx)
{
    if (rx == NULL) {
        printf("ESP8266 ack miss\r\n");
    } else if (strstr(rx, "WIFI DISCONNECT") != NULL) {
        printf("ESP8266 ack miss: WIFI DISCONNECT\r\n");
    } else if (strstr(rx, "DNS FAIL") != NULL) {
        printf("ESP8266 ack miss: DNS FAIL\r\n");
    } else if (strstr(rx, "CONNECT FAIL") != NULL) {
        printf("ESP8266 ack miss: CONNECT FAIL\r\n");
    } else if (strstr(rx, "ALREADY CONNECTED") != NULL) {
        printf("ESP8266 ack miss: ALREADY CONNECTED\r\n");
    } else if (strstr(rx, "busy") != NULL) {
        printf("ESP8266 ack miss: busy\r\n");
    } else if (strstr(rx, "no ip") != NULL) {
        printf("ESP8266 ack miss: no ip\r\n");
    } else if (strstr(rx, "ERROR") != NULL) {
        printf("ESP8266 ack miss: ERROR\r\n");
    } else if (strstr(rx, "CLOSED") != NULL) {
        printf("ESP8266 ack miss: CLOSED\r\n");
    } else {
        printf("ESP8266 ack miss\r\n");
    }
}

static uint8_t esp8266_start_tcp(const char *host, const char *port)
{
    char command[96];
    uint8_t attempt;

    for (attempt = 0U; attempt < 2U; attempt++) {
        if (attempt != 0U) {
            esp8266_close_socket(700);
        }

        snprintf(command, sizeof(command), "AT+CIPSTART=\"TCP\",\"%s\",%s", host, port);
        if (esp8266_send_cmd((u8 *)command, (u8 *)"CONNECT", 1200) == 0U) {
            if (attempt != 0U) {
                printf("ESP8266 TCP connected after retry\r\n");
            }
            return 0U;
        }

        printf("ESP8266 TCP connect attempt %u failed\r\n", (unsigned int)(attempt + 1U));
        esp8266_log_link_status();
    }

    return 1U;
}

static void esp8266_log_link_status(void)
{
    const char *rx;

    if (esp8266_send_cmd((u8 *)"AT+CIPSTATUS", (u8 *)"STATUS", 150) != 0U) {
        printf("ESP8266 link: status unavailable\r\n");
        return;
    }

    rx = (const char *)USART2_RX_BUF;
    if (strstr(rx, "STATUS:2") != NULL) {
        printf("ESP8266 link: got ip\r\n");
    } else if (strstr(rx, "STATUS:3") != NULL) {
        printf("ESP8266 link: connected\r\n");
    } else if (strstr(rx, "STATUS:4") != NULL) {
        printf("ESP8266 link: disconnected\r\n");
    } else if (strstr(rx, "STATUS:5") != NULL) {
        printf("ESP8266 link: wifi not connected\r\n");
    } else {
        printf("ESP8266 link: status unknown\r\n");
    }
}

static uint8_t esp8266_http_get_host(const char *host, const char *port, const char *path, const char *auth_token, const char *ack, uint16_t waittime)
{
    char command[96];
    char request[512];
    int request_len;

    esp8266_send_cmd((u8 *)"AT+CIPMODE=0", (u8 *)"OK", 100);
    esp8266_close_socket(500);

    if (esp8266_start_tcp(host, port) != 0U) {
        return 3U;
    }

    if (auth_token != NULL && auth_token[0] != '\0') {
        request_len = snprintf(request, sizeof(request),
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "Authorization: Bearer %s\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               path, host, auth_token);
    } else {
        request_len = snprintf(request, sizeof(request),
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               path, host);
    }

    if (request_len <= 0 || request_len >= (int)sizeof(request)) {
        esp8266_close_socket(300);
        return 1U;
    }

    snprintf(command, sizeof(command), "AT+CIPSEND=%u", (unsigned int)request_len);
    if (esp8266_send_cmd((u8 *)command, (u8 *)">", 300)) {
        esp8266_close_socket(300);
        return 1U;
    }

    if (esp8266_send_data((u8 *)request, (u8 *)ack, waittime)) {
        esp8266_close_socket(300);
        return 1U;
    }

    return 0U;
}

static uint8_t parse_uint_after(const char *base, const char *key, uint8_t *out)
{
    const char *p = strstr(base, key);
    unsigned int value = 0;

    if (p == NULL || out == NULL) {
        return 0U;
    }

    p = strchr(p, ':');
    if (p == NULL || sscanf(p + 1, " %u", &value) != 1) {
        return 0U;
    }
    if (value > 255U) {
        value = 255U;
    }
    *out = (uint8_t)value;
    return 1U;
}

static uint8_t parse_percent_after(const char *base, const char *section, uint8_t *out)
{
    const char *p = strstr(base, section);
    unsigned int whole = 0;

    if (p == NULL || out == NULL) {
        return 0U;
    }

    p = strstr(p, "\"percent\"");
    if (p == NULL || (p = strchr(p, ':')) == NULL || sscanf(p + 1, " %u", &whole) != 1) {
        return 0U;
    }
    if (whole > 100U) {
        whole = 100U;
    }
    *out = (uint8_t)whole;
    return 1U;
}

static void parse_string_after(const char *base, const char *key, char *out, size_t out_len)
{
    const char *p = strstr(base, key);
    size_t i = 0;

    if (out == NULL || out_len == 0U) {
        return;
    }
    out[0] = '\0';
    if (p == NULL) {
        return;
    }
    p = strchr(p, ':');
    if (p == NULL) {
        return;
    }
    p = strchr(p, '"');
    if (p == NULL) {
        return;
    }
    p++;

    while (*p != '\0' && *p != '"' && i < out_len - 1U) {
        out[i++] = *p++;
    }
    out[i] = '\0';
}
