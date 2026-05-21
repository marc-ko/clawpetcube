#include "esp8266_common.h"
#include "usart.h"
#include "usart2.h"
#include "string.h"
#include "delay.h"
#include "base_function.h"

// WIFI STA mode, set the wireless parameters of the router to connect to, please modify according to your own router settings.
const u8 *wifista_ssid = WIFI_SSID; // Router SSID
// const u8* wifista_ssid="dwy";					// Router SSID
const u8 *wifista_encryption = "wpawpa2_aes"; // WPA/WPA2 AES encryption method
const u8 *wifista_password = WIFI_PASSWORD;   // Connection password

// NTP server IP address
const u8 *NTP_ip = "www.beijing-time.org";
// NTP server port
const u8 *NTP_portnum = "80";

const u8 *vpet_server_ip = "elec3300.marcoko.com";
const u8 *vpet_port = "80";
const u8 *username = "billy";

// Seniverse weather IP address
const u8 *server_ip = "api.seniverse.com";
// Connection port number: 80
const u8 *portnum = "80";
// Seniverse personal account key
const u8 *my_key = "SnSIvs19PdKBANquE";
// City to display weather for
extern u8 cityNum; // City index
extern char cityNameArrPinYin[][10];
extern char cityNameArr[][10];

// const u8  cityNum;															// City array index
// const	char cityName[50];												// City name in Pinyin

// Support for usmart
// Return the received AT command response data to the computer serial port
// mode: 0, do not clear USART2_RX_STA;
//       1, clear USART2_RX_STA;
void esp8266_at_response(u8 mode)
{
    if (USART2_RX_STA & 0X8000) // Data received once
    {
        USART2_RX_BUF[USART2_RX_STA & 0X7FFF] = 0; // Add end character
        printf("%s", USART2_RX_BUF);               // Send to serial port
        if (mode)
            USART2_RX_STA = 0;
    }
}

// After sending a command to esp8266, check the received response
// str: expected response result
// Return value: 0, did not get the expected response result
//               otherwise, the position of the expected response result (position of str)
u8 *esp8266_check_cmd(u8 *str)
{
    char *strx = 0;
    if (USART2_RX_STA & 0X8000) // Data received once
    {
        USART2_RX_BUF[USART2_RX_STA & 0X7FFF] = 0; // Add end character
        strx = strstr((const char *)USART2_RX_BUF, (const char *)str);
    }
    return (u8 *)strx;
}

// Send command to ATK-ESP8266
// cmd: command string to send
// ack: expected response result, if null, no need to wait for response
// waittime: wait time (unit: 10ms)
// Return value: 0, send successful (got the expected response result)
//               1, send failed
u8 esp8266_send_cmd(u8 *cmd, u8 *ack, u16 waittime)
{
    u8 *real_ack;
    u8 res = 0;
    USART2_RX_STA = 0;
    printf("local cmd: %s\r\n", cmd);
    u2_printf("%s\r\n", cmd); // Send command
    if (ack && waittime)      // Need to wait for response
    {
        while (--waittime) // Wait countdown
        {
            delay_ms(10);
            if (USART2_RX_STA & 0X8000) // Received expected response result
            {
                real_ack = esp8266_check_cmd(ack);
                if (real_ack)
                {
                    printf("ack: %s", (u8 *)real_ack);
                    break; // Got valid data
                }
                else
                {
                    printf("\r\nack error: %s", (u8 *)USART2_RX_BUF);
                }
                USART2_RX_STA = 0;
            }
        }
        if (waittime == 0)
            res = 1;
    }
    return res;
}
// Send specified data to esp8266
// data: data to send (no need to add carriage return)
// ack: expected response result, if null, no need to wait for response
// waittime: wait time (unit: 10ms)
// Return value: 0, send successful (got the expected response result)
u8 esp8266_send_data(u8 *data, u8 *ack, u16 waittime)
{
    u8 res = 0;
    USART2_RX_STA = 0;
    u2_printf("%s", data); // Send command
    if (ack && waittime)   // Need to wait for response
    {
        while (--waittime) // Wait countdown
        {
            delay_ms(10);
            if (USART2_RX_STA & 0X8000) // Received expected response result
            {
                if (esp8266_check_cmd(ack))
                    break; // Got valid data
                USART2_RX_STA = 0;
            }
        }
        if (waittime == 0)
            res = 1;
    }
    return res;
}

// Exit transparent transmission mode of esp8266
// Return value: 0, exit successful;
//               1, exit failed
u8 esp8266_quit_trans(void)
{
    while ((USART2->SR & 0X40) == 0)
        ; // Wait for transmission to be empty
    USART2->DR = '+';
    delay_ms(15); // Greater than serial frame time (10ms)
    while ((USART2->SR & 0X40) == 0)
        ; // Wait for transmission to be empty
    USART2->DR = '+';
    delay_ms(15); // Greater than serial frame time (10ms)
    while ((USART2->SR & 0X40) == 0)
        ; // Wait for transmission to be empty
    USART2->DR = '+';
    delay_ms(500);                           // Wait 500ms
    return esp8266_send_cmd("AT", "OK", 20); // Exit transparent transmission judgment.
}

// Get the connection status of the esp8266 module
// Return value: 0, not connected; 1, connected successfully.
u8 esp8266_consta_check(void)
{
    u8 *p;
    u8 res;
    if (esp8266_quit_trans())
        return 0;                              // Exit transparent transmission
    esp8266_send_cmd("AT+CIPSTATUS", ":", 50); // Send AT+CIPSTATUS command to check connection status
    p = esp8266_check_cmd("+CIPSTATUS:");
    res = *p; // Get connection status
    return res;
}

// Initialize EN and RST pins
void esp8266_ctr_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // Enable GPIOA clock

    // Initialize GPIOA0, A1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // General output mode
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;     // Push-pull output
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // Pull-up
    GPIO_Init(GPIOA, &GPIO_InitStructure);             // Initialize

    GPIO_SetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1);
}

// Function: Connect esp8266 to WIFI
// Input: None
// Return: None
// Note: None
void esp8266_sta_connect(void)
{
    char command[200];
    while (esp8266_send_cmd("AT", "OK", 20)) // Check if WIFI module is online
    {
        esp8266_quit_trans();                        // Exit transparent transmission
        esp8266_send_cmd("AT+CIPMODE=0", "OK", 200); // Disable transparent transmission mode
        printf("No ESP8266-12F Detected!\r\n");
        delay_ms(800);
        printf("Attempt to Reconnect...\r\n");
    }
    printf("//========ESP8266-12F Detected!========//\r\n");
    while (esp8266_send_cmd("ATE1", "OK", 20))
        ;                                      // Disable echo
                                               //
    esp8266_send_cmd("AT+CWMODE=1", "OK", 50); // Set WIFI STA mode
    // Delay 3 seconds to wait for successful restart
    delay_ms(200);
    sprintf((char *)command, "AT+CWJAP_DEF=\"%s\",\"%s\"", wifista_ssid, wifista_password); // Set wireless parameters: ssid, password
    while (esp8266_send_cmd((u8 *)command, "WIFI GOT IP", 1000))
        ; // Connect to target router and get IP
    printf("//========WIFI Connected!========//\r\n");
}

// Parse time from the returned string
TimeStruct esp8266_gettime(void)
{
    TimeStruct esp8266_time;
    char command[200];
    char *timeStr;

    u8 month_dayNum[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    char weekStr[8][3] = {" ", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    char monthStr[13][3] = {" ", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    sprintf((char *)command, "AT+CIPSTART=\"TCP\",\"%s\",%s", NTP_ip, NTP_portnum); // Reconnect if disconnected after a period of time
    while (esp8266_send_cmd((u8 *)command, "CONNECT", 300))
        ;

    esp8266_send_cmd("AT+CIPMODE=1", "OK", 100); // Enable transparent transmission
    esp8266_send_cmd("AT+CIPSEND", "OK", 100);   // Start transparent transmission

    // The first time to get the time is incorrect
    sprintf((char *)command, "s"); // Set server IP address and port
    esp8266_send_cmd((u8 *)command, "HTTP", 200);

    // The second time to get the correct time, Beijing time needs to add +8h on this basis
    sprintf((char *)command, "s"); // Set server IP address and port
    esp8266_send_cmd((u8 *)command, "HTTP", 200);
    printf("//========Real Time Obtained!========//\r\n");
    timeStr = (char *)USART2_RX_BUF; // Get weather data

    // Start parsing, example: Date: Tue, 24 Oct 2023 01:55:23 GMT
    timeStr = strstr(timeStr, "Date");
    timeStr = strstr(timeStr, ":");
    timeStr++;

    // Year
    esp8266_time.year = (timeStr[13] - '0') * 1000 + (timeStr[14] - '0') * 100 + (timeStr[15] - '0') * 10 + (timeStr[16] - '0');
    // Day
    esp8266_time.day = (timeStr[6] - '0') * 10 + (timeStr[7] - '0');

    // Hour, minute, second
    esp8266_time.hour = (timeStr[18] - '0') * 10 + (timeStr[19] - '0');
    esp8266_time.minute = (timeStr[21] - '0') * 10 + (timeStr[22] - '0');
    esp8266_time.second = (timeStr[24] - '0') * 10 + (timeStr[25] - '0');

    //========= Week and month are strings, not numbers, so special parsing is needed ========//
    // Week
    char weekStr_real[3] = {0};
    for (u8 i = 0; i < 3; i++)
        weekStr_real[i] = timeStr[i + 1]; // Get week string

    for (u8 i = 1; i <= 7; i++)
    {
        for (u8 j = 0; j < 3; j++)
        {
            if (weekStr_real[j] != weekStr[i][j])
                break;
            else if (j == 2 && weekStr_real[j] == weekStr[i][j])
                esp8266_time.week = i;
        }
    }

    // Month
    char monthStr_real[3] = {0};
    for (u8 i = 0; i < 3; i++)
        monthStr_real[i] = timeStr[i + 9]; // Get month string
    for (u8 i = 1; i <= 12; i++)
    {
        for (u8 j = 0; j < 3; j++)
        {
            if (monthStr_real[j] != monthStr[i][j])
                break;
            else if (j == 2 && monthStr_real[j] == monthStr[i][j])
                esp8266_time.month = i;
        }
    }

    // Recalculate Beijing time, considering the switch of hour, week, day, month, and year at 0 o'clock
    esp8266_time.hour = esp8266_time.hour + 8;
    if (esp8266_time.hour >= 24)
    {
        esp8266_time.hour = esp8266_time.hour - 24;

        esp8266_time.week = esp8266_time.week + 1;
        if (esp8266_time.week >= 8)
            esp8266_time.week = 1;

        esp8266_time.day = esp8266_time.day + 1;
        if (esp8266_time.day > month_dayNum[esp8266_time.month])
        {
            esp8266_time.day = 1;
            esp8266_time.month = esp8266_time.month + 1;
            if (esp8266_time.month >= 13)
            {
                esp8266_time.month = 1;
                esp8266_time.year = esp8266_time.year + 1;
            }
        }
    }

    printf("==========%d/%02d/%02d %d==========\r\n", esp8266_time.year, esp8266_time.month, esp8266_time.day, esp8266_time.week);
    printf("==========%02d:%02d:%02d==========\r\n", esp8266_time.hour, esp8266_time.minute, esp8266_time.second);

    esp8266_quit_trans();                        // Exit transparent transmission
    esp8266_send_cmd("AT+CIPMODE=0", "OK", 100); // Disable transparent transmission
    esp8266_send_cmd("AT+CIPCLOSE", "OK", 100);  // Disconnect TCP connection
                                                 //	memset(usart2RecvData,0,sizeof(usart2RecvData));usart2Pos = 0;
                                                 ////	ESP8266_PrintTime(ESP8266_Time);

    return esp8266_time;
}

// Function: Get local temperature through Seniverse weather
// Input: None
// Return: Weather-related structure
// Note: None
WeatherStruct ESP8266_GetWeather(char *city)
{
    WeatherStruct esp8266_weather;
    char command[200];
    char *tempStr;
    char numString[5];

    sprintf((char *)command, "AT+CIPSTART=\"TCP\",\"%s\",%s", server_ip, portnum); // Reconnect if disconnected after a period of time
    while (esp8266_send_cmd((u8 *)command, "CONNECT", 300))
        ;

    esp8266_send_cmd("AT+CIPMODE=1", "OK", 100); // Enable transparent transmission
    esp8266_send_cmd("AT+CIPSEND", "OK", 100);   // Start transparent transmission

    //================Current Weather================//
    sprintf((char *)command, "GET https://api.seniverse.com/v3/weather/now.json?key=%s&location=%s&language=en&unit=c", my_key, city); // Set server IP address and port
    esp8266_send_cmd((u8 *)command, "{\"results\":", 200);
    printf("//========Now Weather Obtained!========//\r\n");
    tempStr = (char *)USART2_RX_BUF; // Get weather data

    // Get weather code
    tempStr = strstr(tempStr, "code");
    tempStr = strstr(tempStr, ":");
    memset(numString, 0, sizeof(numString));
    u8 i = 2;
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.present_code = stringToNum(numString);
    printf("present_code: %d\r\n", esp8266_weather.present_code);

    // Get current temperature
    memset(numString, 0, sizeof(numString));
    i = 2;
    tempStr = strstr(tempStr, "ture");
    tempStr = strstr(tempStr, ":");
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.present_temp = stringToNum(numString);

    delay_ms(200); // There should be a certain interval between two accesses

    //================Today's Weather================//
    // Note: Today's weather returns data for 3 days, so a large space is allocated for the receive buffer of USART2
    sprintf((char *)command, "GET https://api.seniverse.com/v3/weather/daily.json?key=%s&location=%s&language=en&unit=c", my_key, city); // Set server IP address and port
    esp8266_send_cmd((u8 *)command, "{\"results\":", 200);
    printf("//========Daily and Tomorrow Weather Obtained!========//\r\n");
    tempStr = (char *)USART2_RX_BUF; // Get weather data

    // Get today's highest temperature
    memset(numString, 0, sizeof(numString));
    i = 2;
    tempStr = strstr(tempStr, "high");
    tempStr = strstr(tempStr, ":");
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.today_highTemp = stringToNum(numString);

    // Get today's lowest temperature
    memset(numString, 0, sizeof(numString));
    i = 2;
    tempStr = strstr(tempStr, "low");
    tempStr = strstr(tempStr, ":");
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.today_lowTemp = stringToNum(numString); // Get lowest temperature

    // Get today's precipitation
    memset(numString, 0, sizeof(numString));
    i = 2;
    tempStr = strstr(tempStr, "rainfall");
    tempStr = strstr(tempStr, ":");
    while (tempStr[i] != '.') // Discard the decimal point
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.today_precipitation = stringToNum(numString);

    // Get today's humidity
    memset(numString, 0, sizeof(numString));
    i = 2;
    tempStr = strstr(tempStr, "humidity");
    tempStr = strstr(tempStr, ":");
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.today_humidity = stringToNum(numString);

    //================Tomorrow's Weather================//
    // Get tomorrow's highest temperature
    memset(numString, 0, sizeof(numString));
    i = 2;
    tempStr = strstr(tempStr, "high");
    tempStr = strstr(tempStr, ":");
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.tomorrow_highTemp = stringToNum(numString);

    // Get tomorrow's lowest temperature
    memset(numString, 0, sizeof(numString));
    i = 2;
    tempStr = strstr(tempStr, "low");
    tempStr = strstr(tempStr, ":");
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.tomorrow_lowTemp = stringToNum(numString); // Get lowest temperature

    // Get tomorrow's precipitation
    memset(numString, 0, sizeof(numString));
    i = 2;
    tempStr = strstr(tempStr, "rainfall");
    tempStr = strstr(tempStr, ":");
    while (tempStr[i] != '.') // Discard the decimal point
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.tomorrow_precipitation = stringToNum(numString);

    // Get tomorrow's humidity
    memset(numString, 0, sizeof(numString));
    i = 2;
    tempStr = strstr(tempStr, "humidity");
    tempStr = strstr(tempStr, ":");
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    esp8266_weather.tomorrow_humidity = stringToNum(numString);

    esp8266_quit_trans();                        // Exit transparent transmission
    esp8266_send_cmd("AT+CIPMODE=0", "OK", 100); // Disable transparent transmission
    esp8266_send_cmd("AT+CIPCLOSE", "OK", 100);  // Disconnect TCP connection

    return esp8266_weather;
}

UserInfoStruct ESP8266_GetUserInfo(char *username)
{
    UserInfoStruct userInfo;
    char command[512]; // Increase from 200 to 512 to accommodate longer headers
    char *tempStr;
    char numString[5];

    sprintf((char *)command, "AT+CIPSTART=\"TCP\",\"%s\",%s", vpet_server_ip, vpet_port); // Reconnect if disconnected after a period of time
    while (esp8266_send_cmd((u8 *)command, "CONNECT", 300))
        ;

    if (esp8266_send_cmd("AT+CIPMODE=1", "OK", 100))
    {
        printf("//========fuck CIPMODE!========//\r\n");
        goto end;
    }

    printf("//========Transmission Mode Enabled!========//\r\n");

    if (esp8266_send_cmd("AT+CIPSEND", "OK", 100))
    {
        printf("//========fuck CIPSEND!========//\r\n");
        goto end;
    }
    printf("//========CIPSEND OK!========//\r\n");

    //================Current Backend================//
    sprintf((char *)command,
            "GET /api/getuserstatus?username=%s HTTP/1.1\r\n"
            "Host: elec3300.marcoko.com\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n"
            "Accept: application/json\r\n"
            "Accept-Language: en-US,en;q=0.9\r\n"
            "Connection: close\r\n"
            "\r\n",
            username);
    int res = esp8266_send_cmd((u8 *)command, "{\"results\":", 200);
    if (res == 0)
    {
        printf("//========User Info Obtained!======== TIM WOO!!!//\r\n");
    }
    else
    {
        printf("//========User Info Not Obtained!========//\r\n");
        printf("res: %d\r\n", res);
        printf("end game I gg go end\r\n");
        userInfo.coins = 100;
        userInfo.hunger = 100;
        userInfo.exp = 100;
        userInfo.energy = 100;
        userInfo.emotion = 100;
        userInfo.status = 0;
        userInfo.action = ACTION_NONE;
        strcpy(userInfo.username, username);
        printf("userInfo: %s\r\n", userInfo.username);
        goto end;
    }

    tempStr = (char *)USART2_RX_BUF; // Get user data

    strcpy(userInfo.username, username);

    // get coins
    tempStr = strstr(tempStr, "coins");
    tempStr = strstr(tempStr, ":");
    memset(numString, 0, sizeof(numString));
    u8 i = 2;
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }

    printf("numString: %s\r\n", numString);
    userInfo.coins = stringToNum(numString);

    printf("coins: %d\r\n", userInfo.coins);

    // get hunger
    tempStr = strstr(tempStr, "hunger");
    tempStr = strstr(tempStr, ":");
    memset(numString, 0, sizeof(numString));
    i = 2;
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    printf("hunger numString: %s\r\n", numString);
    userInfo.hunger = stringToNum(numString);

    printf("hunger: %d\r\n", userInfo.hunger);

    // get exp
    tempStr = strstr(tempStr, "exp");
    tempStr = strstr(tempStr, ":");
    memset(numString, 0, sizeof(numString));
    i = 2;
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    userInfo.exp = stringToNum(numString);

    printf("exp: %d\r\n", userInfo.exp);

    // get energy
    tempStr = strstr(tempStr, "energy");
    tempStr = strstr(tempStr, ":");
    memset(numString, 0, sizeof(numString));
    i = 2;
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    userInfo.energy = stringToNum(numString);

    printf("energy: %d\r\n", userInfo.energy);

    // get emotion
    tempStr = strstr(tempStr, "emotion");
    tempStr = strstr(tempStr, ":");
    memset(numString, 0, sizeof(numString));
    i = 2;
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    userInfo.emotion = stringToNum(numString);

    printf("emotion: %d\r\n", userInfo.emotion);

    // get status
    tempStr = strstr(tempStr, "status");
    tempStr = strstr(tempStr, ":");
    memset(numString, 0, sizeof(numString));
    i = 2;
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    printf("status before: %d\r\n", (int)numString[0]); // Cast to int to get ASCII code

    userInfo.status = (int)numString[0]; // Just take the first character

    printf("status: %d\r\n", userInfo.status);

    // get action
    tempStr = strstr(tempStr, "action");
    tempStr = strstr(tempStr, ":");
    memset(numString, 0, sizeof(numString));
    i = 2;
    while (tempStr[i] != '\"')
    {
        numString[i - 2] = tempStr[i];
        i++;
    }
    u8 actionChar = numString[0]; // Just take the first character
    if (actionChar == 0)
        userInfo.action = ACTION_NONE;
    else if (actionChar == 102) // f
        userInfo.action = ACTION_EAT;
    else if (actionChar == 119) // w
        userInfo.action = ACTION_WRITE;
    else if (actionChar == 115) // s
        userInfo.action = ACTION_SLEEP;
    else if (actionChar == 100) // d
        userInfo.action = ACTION_CLEAN;
    else
        userInfo.action = ACTION_NONE;

    printf("action after: %d\r\n", userInfo.action);

    esp8266_quit_trans();
    esp8266_send_cmd("AT+CIPMODE=0", "OK", 100);
    esp8266_send_cmd("AT+CIPCLOSE", "OK", 100);
end:
    printf("userInfo: %s %d %d %d %d %d %d %d\r\n", userInfo.username, userInfo.coins, userInfo.hunger, userInfo.exp, userInfo.energy, userInfo.emotion, userInfo.status, userInfo.action);

    return userInfo;
}
