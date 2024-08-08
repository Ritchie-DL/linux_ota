//
// Created by rich on 2024/3/31.
//

#include "my_debug.h"

#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

static dl_log_level_t g_en_log_level = HI_LOG_LEVEL_RAW;
static bool g_log_en = true;
static bool g_log_time_en = true;

static const char *g_log_level[HI_LOG_LEVEL_BUTT] = {
        RED "FATAL",            /* fatal */
        LIGHT_RED "ERROR",      /* error */
        YELLOW "WARN",          /* warn */
        PURPLE "MARK",          /* mark */
        LIGHT_GREEN "INFO" ,    /* info */
        LIGHT_GRAY,             /* raw */
        CYAN "DEBUG",           /* debug */
        WHITE,                  /* redirect */
};

int dl_log_print(dl_log_level_t level, const char *filename, const char *func, uint32_t line, const char *fmt, ...)
{
    struct timeval tv;
    va_list args;
#ifndef BUILD_TYPE_DEBUG
    FILE *fp_out = stderr;
#else
    FILE *fp_out = stdout;
#endif
    if (!g_log_en) {
        return 1;
    }
    if ((g_en_log_level >= level) && (HI_LOG_LEVEL_RAW == level)) {
        va_start(args, fmt);
        vfprintf(fp_out, fmt, args);
        va_end(args);
        fprintf(fp_out, NONE);
    } else if (g_en_log_level >= level) {
        fmt = (NULL == fmt) ? "" : fmt;
        if (level >= HI_LOG_LEVEL_MARK) {
            fp_out = stderr;
        }
        if (g_log_time_en) {
            gettimeofday(&tv, NULL);
            struct tm tm;
            localtime_r(&tv.tv_sec, &tm);
            if (level != HI_LOG_LEVEL_REDIRECT) {
                fprintf(fp_out, NONE "[%02d:%02d:%02d:%03ld]%s:%s[%s, %d] ", tm.tm_hour, tm.tm_min, tm.tm_sec,
                        tv.tv_usec / 1000, g_log_level[level], filename, func, line);
            } else {
                fprintf(fp_out, NONE "[%02d:%02d:%02d:%03ld]%s", tm.tm_hour, tm.tm_min, tm.tm_sec,
                        tv.tv_usec / 1000, g_log_level[level]);
            }
        } else {
            if (level != HI_LOG_LEVEL_REDIRECT) {
                fprintf(fp_out, NONE "%s:%s[%s, %d] ", g_log_level[level], filename, func, line);
            } else {
                fprintf(fp_out, NONE "%s", g_log_level[level]);
            }
        }
        va_start(args, fmt);
        vfprintf(fp_out, fmt, args);
        va_end(args);
        fprintf(fp_out, NONE);
    }

    return 1;
}

void dl_log_config(bool log_on, bool log_time, dl_log_level_t level)
{
    const char *rkmedia_name = "RKMEDIA_LOG_LEVEL";

    g_log_en = log_on;
    g_log_time_en = log_time;
    g_en_log_level = level;

    if (!log_on) {
        setenv("MQTT_C_CLIENT_TRACE", "OFF", 1);
        setenv("MQTT_C_CLIENT_TRACE_LEVEL", "OFF", 1);
        return;
    }
    if (level <= HI_LOG_LEVEL_ERROR) {
        setenv(rkmedia_name, "ERROR", 1);
    } else if (level <= HI_LOG_LEVEL_INFO) {
        setenv(rkmedia_name, "WARN", 1);
    } else if (level <= HI_LOG_LEVEL_RAW) {
        setenv(rkmedia_name, "INFO", 1);
    } else if (level > HI_LOG_LEVEL_DEBUG) {
        setenv(rkmedia_name, "DBG", 1);
    }
}

void print_n_byte(const uint8_t *str, uint32_t len)
{
    int i = 0;

    if (str == NULL) {
        return;
    }
    if (!g_log_en) {
        return;
    }
    if (g_en_log_level < HI_LOG_LEVEL_RAW) {
        return;
    }
    for (i = 0; i < len; i++) {
        if (i && (0 == i % 16)) {
            dbg_raw("\n");
        }
        dbg_raw("%02X ", str[i]);
    }
    dbg_raw("\n");
}
