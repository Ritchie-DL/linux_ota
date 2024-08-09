//
// Created by rich on 2024/3/25.
//

#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/syscall.h>

#include <stdint-gcc.h>
#include "my_debug.h"

void prefix_signal_handler(fun_signal fun)
{
    signal(SIGINT, fun);	/* ctrl+c */
    signal(SIGTSTP, fun);	/* ctrl+z */
    signal(SIGTERM, fun);	/* reboot */
}

int dl_utils_get_time_string(char *dest_buf, uint32_t dest_size)
{
    if (assert_ptr(dest_buf)) {
        return -1;
    }
    if (dest_size < 16) {
        dbg_err("dest_buf is too small\n", dest_buf);
        return -1;
    }
    time_t cur_time = time(NULL);
    struct tm *local_time = localtime(&cur_time);
    if (assert_ptr(local_time)) {
        return -1;
    }
//    int rc = strftime(dest_buf, dest_size, "%Y-%m-%d %H:%M:%S", local_time);
    int rc = strftime(dest_buf, dest_size, "%H:%M:%S", local_time);
    if (rc > 0) {
        dbg_lo("rc=%d, time:%s\n", rc, dest_buf);
    }
    return rc;
}

uint64_t get_time_value(void)
{
    uint64_t ret_val = 0;

    time_t current_time = time(NULL);
    ret_val = (uint64_t)current_time;

    return ret_val;
}

void dl_print_local_time(const char *label, struct tm *local_tm)
{
    if (assert_ptr(local_tm)) {
        return;
    }
    if (label != NULL) {
        dbg_info("%s time: %u-%u-%u %u:%u:%u\n", label, local_tm->tm_year + 1900, local_tm->tm_mon + 1,
                 local_tm->tm_mday, local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec);
    } else {
        dbg_info("local time: %u-%u-%u %u:%u:%u\n", local_tm->tm_year + 1900, local_tm->tm_mon + 1,
                 local_tm->tm_mday, local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec);
    }
}

void dl_print_utc_time(const char *label, uint64_t utc_val)
{
    time_t utc_time = {0};
    memcpy(&utc_time, &utc_val, sizeof(time_t));
    struct tm *tm_info = localtime(&utc_time);
    dl_print_local_time(label, tm_info);
}

int dl_utils_get_date_string(char *dest_buf, uint32_t dest_size, char separator)
{
    int ret = 0;

    if (assert_ptr(dest_buf)) {
        return -1;
    }
    if (dest_size < 16) {
        dbg_err("dest_buf is too small\n", dest_buf);
        return -1;
    }
    time_t cur_time = time(NULL);
    struct tm *local_time = localtime(&cur_time);
    if (assert_ptr(local_time)) {
        return -1;
    }

    if (separator == 0) {
        ret = strftime(dest_buf, dest_size, "%Y年%m月%d日", local_time);
    } else {
        ret = strftime(dest_buf, dest_size, "%Y-%m-%d", local_time);
    }
    if (ret > 0) {
        dbg_lo("rc=%d, time:%s\n", ret, dest_buf);
    }
    return ret;
}

int dl_utils_get_full_time_string(char *dest_buf, uint32_t dest_size, char separator)
{
    int rc = 0;
    if (assert_ptr(dest_buf)) {
        return -1;
    }
    if (dest_size < 16) {
        dbg_err("dest_buf is too small\n", dest_buf);
        return -1;
    }
    time_t cur_time = time(NULL);
    struct tm *local_time = localtime(&cur_time);
    if (assert_ptr(local_time)) {
        return -1;
    }
    if (separator == 0) {
        rc = strftime(dest_buf, dest_size, "%Y%m%d%H%M%S", local_time);
    } else {
        rc = strftime(dest_buf, dest_size, "%Y-%m-%d-%H-%M-%S", local_time);
    }
    if (rc > 0) {
        dbg_lo("rc=%d, time:%s\n", rc, dest_buf);
    }
    return rc;
}


unsigned long dl_utils_get_diff_time(const struct timeval *time_new, const struct timeval *time_old)
{
    const unsigned long factor = 1000 * 1000; /* 1000 * 1000us = 1s */
    if (assert_ptr(time_new) || assert_ptr(time_old)) {
        return 0;
    }
    unsigned long diff;
    diff = (time_new->tv_sec * factor + time_new->tv_usec) - (time_old->tv_sec * factor + time_old->tv_usec);
    return diff;
}

int create_path(const char *path)
{
    const char *start = NULL;
    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

    if (path[0] == '/') {
        start = strchr(path + 1, '/');
    } else {
        start = strchr(path, '/');
    }

    while (start) {
        char *buffer = strdup(path);
        buffer[start-path] = 0x00;

        if (mkdir(buffer, mode) == -1 && errno != EEXIST) {
            printf("creating directory %s fail.\n", buffer);
            free(buffer);
            return -1;
        }
        free(buffer);
        start = strchr(start + 1, '/');
    }
    return 0;
}

int safe_crate_file(const char *name)
{
    if (assert_ptr(name)) {
        return -1;
    }
    int fd = open(name, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        if (errno != ENOENT) {
            dbg_perror("open:");
            return -1;
        }
        if (create_path(name) == 0) {
            fd = open(name, O_WRONLY | O_CREAT, 0644);
        }
    }
    return fd;
}

int number_to_string(char *dest, size_t src_buf, int len)
{
    if (assert_ptr(dest)) {
        return -1;
    }
    int index = 0;
    while (index < len){
        dest[index] = (char)(src_buf & 0xff);
        src_buf = src_buf >> 8;
        index ++;
    }
    return 0;
}

int string_to_bcd(const char *src_buf, size_t src_len, uint8_t *dest_buf, size_t dest_len)
{
    uint8_t byte_high = 0x00;
    uint8_t byte_low = 0x00;
    int i = 0;
    int ret_len = (src_len + 1) / 2;

    if (assert_ptr(src_buf) || assert_ptr(dest_buf)) {
        return -1;
    }
    if (dest_len < ret_len) {
        dbg_err("invalid dest buf size(%zu), expect(%d)\n", dest_len, ret_len);
        return -1;
    }
    for (i = 0; i < src_len / 2; ++i) {
        byte_high = src_buf[i * 2] - '0';
        byte_low = src_buf[i * 2 + 1] - '0';
        dest_buf[i] = (byte_high << 4) | byte_low;
    }

    if ((src_len % 2) != 0) {
        dest_buf[src_len / 2] = (src_buf[src_len - 1] - '0') << 4;
    }
    return ret_len;
}

int bcd_to_string(const uint8_t *src_buf, uint32_t src_len, char *dest_buf, uint32_t dest_len)
{
    uint32_t i = 0;
    uint32_t j = 0;

    if (assert_ptr(src_buf) || assert_ptr(dest_buf)) {
        return -1;
    }
    for (i = 0; i < src_len && j < dest_len; i++) {
        dest_buf[j] = (((char)src_buf[i] >> 4) & 0x0F) + 0x30;
        dest_buf[j + 1] = ((char)src_buf[i] & 0x0F) + 0x30;
        j += 2;
    }
    return 0;
}

void *dl_malloc(size_t size)
{
    if ((size == 0) || (size > HI_APPCOM_MALLOC_MAX_SIZE)) {
        dbg_err("invalid size(%zu)\n", size);
        return NULL;
    }
    void *ptr = malloc(size);
    if (ptr == NULL) {
        dbg_perror("malloc");
    }
    return ptr;
}

int dl_realpath(const char *filename, char *dest_buf, uint32_t dest_size)
{
    if (assert_ptr(filename) || assert_ptr(filename)){
        return -1;
    }
    if (strnlen(filename, dest_size) == dest_size) {
        dbg_err("string too long\n");
        return -1;
    }
    if (!realpath(filename, dest_buf)){
        dbg_err("realpath for %s failed\n", filename);
        return -1;
    }
    return 0;
}

void dl_utils_get_uuid_id(char dest_buf[UUID_STR_LENGTH])
{
    char unparse_buf[64] = {0};
    char tmp_buf[64];

    sscanf(unparse_buf, "%[a-zA-Z0-9]-%[^-]-%[^-]-%[^-]-%[^-]", &tmp_buf[0], &tmp_buf[8], &tmp_buf[12], &tmp_buf[16], &tmp_buf[20]);
    memcpy(dest_buf, tmp_buf, UUID_STR_LENGTH);
    dbg_lo("tmp_buf, len=%zu, %s\n", strlen((unparse_buf)), unparse_buf);
    dbg_lo("dest_buf, len=%zu, %s\n", strlen((dest_buf)), dest_buf);
}

char ch_byte_to_hex(char ch)
{
    char out_hex = 0;

    if (isdigit(ch)) {
        out_hex = ch - '0';
    } else if(isupper(ch)) {
        out_hex = ch - 'A' + 10;
    } else{
        out_hex = ch - 'a' + 10;
    }

    return out_hex;
}

char hex_byte_to_char(char hex)
{
    char ch = 0;

    if ((hex >= 0) && (hex <= 9)) {
        ch = hex + '0';
    } else if(hex <= 0x0F) {
        ch = hex - 0x0A + 'A';
    } else {
        ch = 0x7F;
    }
    return ch;
}

int hex_array_to_string(const char *src_hex, uint32_t src_len, char *dest_buf, uint32_t dest_len)
{
    int i = 0, j = 0;

    if (assert_ptr(src_hex) || assert_ptr(dest_buf)) {
        return -1;
    }
    if (dest_len < src_len * 2) {
        dbg_err("invalid dest_len=%u, expect at lest:%u\n", dest_len, 2 *src_len);
        return -1;
    }
    for (i = 0; i < src_len; i++) {
        dest_buf[j] = hex_byte_to_char((src_hex[i] >> 4) & 0xF);
        dest_buf[j + 1] = hex_byte_to_char(src_hex[i] & 0xF);
        j += 2;
    }
    return 0;
}

int dl_utils_match_str_both_case(const char *src, const char *needle_0, const char *needle_1)
{
    if (assert_ptr(src) || assert_ptr(needle_0) || assert_ptr(needle_1)) {
        return -1;
    }
    if ((strstr(src, needle_0) != NULL) || (strstr(src, needle_1) != NULL)) {
        return 1;
    }

    return 0;
}

int dl_utc_to_local_time(uint64_t utc_tm, struct tm *local_tm, bool set_local)
{
    if (assert_ptr(local_tm)) {
        return -1;
    }
    struct timeval tv;
    time_t utc_time, local_time;

    // Convert UTC time to time_t
    utc_time = (time_t)utc_tm;

    // Convert UTC time to local time
    struct tm *local_tmp = localtime(&utc_time);
    if (assert_ptr(local_tmp)) {
        return -1;
    }
    memcpy(local_tm, local_tmp, sizeof(struct tm));
    if (!set_local) {
        return 0;
    }
    // Obtain the local time in time_t format
    local_time = mktime(local_tmp);

    // Set the seconds and microseconds fields of timeval structure
    tv.tv_sec = local_time;
    tv.tv_usec = 0;

    // Set the system time
    if (settimeofday(&tv, NULL) == -1) {
        dbg_err("Error setting system time(%s)\n", strerror(errno));
    } else {
        dbg_lo("System time set successfully\n");
    }

    return 0;
}

void dl_set_timezone(const char *timezone)
{
    if (setenv("TZ", timezone, 1) == 0) {
        dbg_lo("Time zone set to: %s\n", timezone);
    } else {
        dbg_err("Error setting time zone\n");
    }
}

pid_t gettid(void)
{
    return syscall(SYS_gettid);
}
