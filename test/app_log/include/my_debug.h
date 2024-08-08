#ifndef __MY_DEBUG_H
#define __MY_DEBUG_H

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if defined (__cplusplus)
  extern "C" {
#endif

#define NONE         "\033[m"
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"

typedef enum {
    HI_LOG_LEVEL_FATAL = 0, /* action must be taken immediately */
    HI_LOG_LEVEL_ERROR,     /* error conditions */
    HI_LOG_LEVEL_WARNING,   /* warning conditions */
    HI_LOG_LEVEL_MARK,      /* debug-level */
    HI_LOG_LEVEL_INFO,      /* informational */
    HI_LOG_LEVEL_RAW,       /* log output with raw */
    HI_LOG_LEVEL_DEBUG,     /* print directly */
    HI_LOG_LEVEL_REDIRECT,  /* redirect log output */
    HI_LOG_LEVEL_BUTT
} dl_log_level_t;

/*---------------------------------------------------------------------------------------------------------*/
#ifndef __SHORT_FILE
#define __SHORT_FILE ((strrchr(__FILE__, '/') != NULL)? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif

#ifndef DEBUG_NONE
#define my_debug(fmt, ...)          fprintf(stderr, fmt, ##__VA_ARGS__)
#define dbg_fatal(fmt, args...)     dl_log_print(HI_LOG_LEVEL_FATAL, __SHORT_FILE, __func__, __LINE__, fmt, ##args)
#define dbg_err(fmt, args...)       dl_log_print(HI_LOG_LEVEL_ERROR, __SHORT_FILE, __func__, __LINE__, fmt, ##args)
#define dbg_perror(fmt, args...)    dl_log_print(HI_LOG_LEVEL_ERROR, __SHORT_FILE, __func__, __LINE__, fmt":%s\n", ##args, strerror(errno))
#define dbg_warn(fmt, args...)      dl_log_print(HI_LOG_LEVEL_WARNING, __SHORT_FILE, __func__, __LINE__, fmt, ##args)

#ifdef DEBUG_INFO
#define dbg_info(fmt, args...)      dl_log_print(HI_LOG_LEVEL_INFO, __SHORT_FILE, __func__, __LINE__, fmt, ##args)
#define dbg_mark(fmt, args...)		dl_log_print(HI_LOG_LEVEL_MARK,  __SHORT_FILE, __func__, __LINE__, fmt, ##args)
#define dbg_raw(fmt, args...)
#define dbg_print(fmt, ...)
#define dbg_lo(fmt, args...)
#define dbg_direct(fmt, args...)

#elif (defined(DEBUG_ERROR))
#define dbg_info(fmt, args...)
#define dbg_mark(fmt, args...)
#define dbg_print(fmt, ...)
#define dbg_lo(fmt, args...)
#define dbg_direct(fmt, args...)
#define dbg_raw(fmt, args...)

#else
#define dbg_info(fmt, args...)      dl_log_print(HI_LOG_LEVEL_INFO, __SHORT_FILE, __func__, __LINE__, fmt, ##args)
#define dbg_raw(fmt, args...)       dl_log_print(HI_LOG_LEVEL_RAW, __SHORT_FILE, __func__, __LINE__, fmt, ##args)
#define dbg_mark(fmt, args...)		dl_log_print(HI_LOG_LEVEL_MARK,  __SHORT_FILE, __func__, __LINE__, fmt, ##args)
#define dbg_print(fmt, args...)	    dbg_raw(fmt, ##args)
#define dbg_lo(fmt, args...)	    dl_log_print(HI_LOG_LEVEL_DEBUG, __SHORT_FILE, __func__, __LINE__, fmt, ##args)
#define dbg_direct(fmt, args...)    dl_log_print(HI_LOG_LEVEL_REDIRECT, __SHORT_FILE, __func__, __LINE__, fmt, ##args)

#endif

#else// no debug
#define	my_debug(fmt, args...)

#define dbg_fatal(fmt, args...)
#define dbg_err(fmt, args...)
#define dbg_perror(fmt, args...)
#define dbg_warn(fmt, args...)
#define dbg_info(fmt, args...)
#define dbg_mark(fmt, args...)
#define dbg_print(fmt, ...)
#define dbg_lo(fmt, args...)
#define dbg_direct(fmt, args...)
#endif

#ifndef MTOSTR
#define	MTOSTR(S)		#S
#endif

#ifndef assert_ptr
	#ifndef DEBUG_NONE
		#define	assert_ptr(x) ((NULL==(x))&&(dbg_err("%s is nullptr\n", MTOSTR(x))))
	#else
		#define	assert_ptr(x) ((NULL==(x))&&(fprintf(stderr, "%s is nullptr\n", MTOSTR(x))))
	#endif
#endif

#ifndef CHECK_AND_RETURN_IF_FALSE
    #define CHECK_AND_RETURN_IF_FALSE(val, ret)     \
    if (false == (val))  {                          \
        dbg_err("check %s failed\n", #val);         \
        return (ret);                               \
    }
#endif


int dl_log_print(dl_log_level_t level, const char *filename, const char *func, uint32_t line, const char *fmt, ...);
void dl_log_config(bool log_on, bool log_time, dl_log_level_t level);
void print_n_byte(const uint8_t *str, uint32_t len);

#if defined (__cplusplus)
  }
#endif

#endif
