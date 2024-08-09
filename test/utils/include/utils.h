//
// Created by rich on 2024/3/25.
//

#ifndef __UTILS_H
#define __UTILS_H

#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define __utils_weak __attribute__((weak))
#define __app_weak __attribute__((weak))

//#define HI_APPCOM_MALLOC_MAX_SIZE                   8192
#define HI_APPCOM_MALLOC_MAX_SIZE                   2949120

#ifndef UUID_STR_LENGTH
    #define UUID_STR_LENGTH     32
#endif

#define CHINA_SHANGHAI_STR                          "Asia/Shanghai"

typedef void (*const fun_signal)(int n);

void prefix_signal_handler(fun_signal fun);
int dl_utils_get_time_string(char *dest_buf, uint32_t dest_size);
int dl_utils_get_date_string(char *dest_buf, uint32_t dest_size, char separator);
int dl_utils_get_full_time_string(char *dest_buf, uint32_t dest_size, char separator);
unsigned long dl_utils_get_diff_time(const struct timeval *time_new, const struct timeval *time_old);
uint64_t get_time_value(void);
void dl_print_local_time(const char *label, struct tm *local_tm);
void dl_print_utc_time(const char *label, uint64_t utc_val);
int create_path(const char *path);
int safe_crate_file(const char *name);
int number_to_string(char *dest, size_t src, int len);
int string_to_bcd(const char *src_buf, size_t src_len, uint8_t *dest_buf, size_t dest_len);
int bcd_to_string(const uint8_t *src_buf, uint32_t src_len, char *dest_buf, uint32_t dest_len);
void *dl_malloc(size_t size);
int dl_realpath(const char *filename, char *dest_buf, uint32_t dest_size);
void dl_utils_get_uuid_id(char dest_buf[UUID_STR_LENGTH]);
char ch_byte_to_hex(char ch);
char hex_byte_to_char(char hex);
int hex_array_to_string(const char *src_hex, uint32_t src_len, char *dest_buf, uint32_t dest_len);
int dl_utils_match_str_both_case(const char *src, const char *needle_0, const char *needle_1);
int dl_utc_to_local_time(uint64_t utc_tm, struct tm *local_tm, bool set_local);
void dl_set_timezone(const char* timezone);
pid_t gettid(void);

#if defined (__cplusplus)
}
#endif

#endif //__UTILS_H
