/**
 * @file lv_log.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_log.h"
#if LV_USE_LOG

#include <stdarg.h>
#include <string.h>
#include "lv_printf.h"
#include "../hal/lv_hal_tick.h"

#if LV_LOG_PRINTF
    #include <stdio.h>
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_log_print_g_cb_t custom_print_cb;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Register custom print/write function to call when a log is added.
 * It can format its "File path", "Line number" and "Description" as required
 * and send the formatted log message to a console or serial port.
 * @param print_cb a function pointer to print a log
 */
void lv_log_register_print_cb(lv_log_print_g_cb_t print_cb)
{
    custom_print_cb = print_cb;
}

/**
 * Add a log
 * @param level the level of log. (From `lv_log_level_t` enum)
 * @param file name of the file when the log added
 * @param line line number in the source code where the log added
 * @param func name of the function when the log added
 * @param format printf-like format string
 * @param ... parameters for `format`
 */
void _lv_log_add(lv_log_level_t level, const char * file, int line, const char * func, const char * format, ...)
{
    if(level < 0 || level >= _LV_LOG_LEVEL_NUM) return; /*Invalid level*/

    static uint32_t last_log_time = 0;

    if(level >= LV_LOG_LEVEL) {
        va_list args;
        va_start(args, format);

        const char * file_name = file ? file : "";
        const char * func_name = func ? func : "";

        /*Use only the file name not the path*/
        size_t p;
        for(p = strlen(file_name); p > 0; p--) {
            if(file_name[p] == '/' || file_name[p] == '\\') {
                p++;    /*Skip the slash*/
                break;
            }
        }

        uint32_t t = lv_tick_get();
        const char * level_name = "User";
        switch(level) {
            case LV_LOG_LEVEL_TRACE:
                level_name = "Trace";
                break;
            case LV_LOG_LEVEL_INFO:
                level_name = "Info";
                break;
            case LV_LOG_LEVEL_WARN:
                level_name = "Warn";
                break;
            case LV_LOG_LEVEL_ERROR:
                level_name = "Error";
                break;
            case LV_LOG_LEVEL_USER:
                level_name = "User";
                break;
        }

#if LV_LOG_PRINTF
        printf("[%s]\t(%" LV_PRId32 ".%03" LV_PRId32 ", +%" LV_PRId32 ")\t %s: ",
               level_name, t / 1000, t % 1000, t - last_log_time, func_name);
        vprintf(format, args);
        printf(" \t(in %s line #%d)\n", &file_name[p], line);
#else
        if(custom_print_cb) {
            char buf[512];
#if LV_SPRINTF_CUSTOM
            char msg[256];
            lv_vsnprintf(msg, sizeof(msg), format, args);
            lv_snprintf(buf, sizeof(buf), "[%s]\t(%" LV_PRId32 ".%03" LV_PRId32 ", +%" LV_PRId32 ")\t %s: %s \t(in %s line #%d)\n",
                        level_name, t / 1000, t % 1000, t - last_log_time, func_name, msg, &file_name[p], line);
#else
            lv_vaformat_t vaf = {format, &args};
            lv_snprintf(buf, sizeof(buf), "[%s]\t(%" LV_PRId32 ".%03" LV_PRId32 ", +%" LV_PRId32 ")\t %s: %pV \t(in %s line #%d)\n",
                        level_name, t / 1000, t % 1000, t - last_log_time, func_name, (void *)&vaf, &file_name[p], line);
#endif
            custom_print_cb(buf);
        }
#endif

        last_log_time = t;
        va_end(args);
    }
}

void lv_log(const char * buf)
{
#if LV_LOG_PRINTF
    puts(buf);
#endif
    if(custom_print_cb) custom_print_cb(buf);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_LOG*/
