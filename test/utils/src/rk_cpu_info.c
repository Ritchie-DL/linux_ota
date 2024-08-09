//
// Created by rich on 2024/3/18.
//

#include "rk_cpu_info.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "my_debug.h"

#define PROC_CPUINFO            "/proc/cpuinfo"

int rk_read_proc_cpuinfo(char *dest_buf, uint32_t dest_len)
{
    char *serial = NULL;
    size_t n = 0;
    char *line = NULL;

    if (assert_ptr(dest_buf)) {
        return -1;
    }

    FILE *fp = fopen(PROC_CPUINFO, "r");
    if (fp == NULL) {
        printf("fopen (%s) failed\n", PROC_CPUINFO);
        return -1;
    }

    while (getline(&line, &n, fp) > 0) {
        if (strstr(line, "Serial")) {
            serial = strstr(line, ":") + 2;
            dbg_info("serial(%u):%s", strlen(serial), serial);
            strncpy(dest_buf, serial, dest_len);
        }
    }
    free(line);
    fclose(fp);

    return errno;
}

uint64_t rk_read_cpu_id(void)
{
    int ret = 0;
    char cpu_id_str[18] = {0};
    typedef struct {
        uint32_t lo;
        uint32_t hi;
    } union_uint64_t;

    typedef union {
        uint64_t llu;
        union_uint64_t hilo;
    } cpu_id_info_t;

    cpu_id_info_t cpu_id_info = {0};

    ret = rk_read_proc_cpuinfo(cpu_id_str, sizeof(cpu_id_str));
    if (ret < 0) {
        dbg_err("rk_read_proc_cpuinfo failed\n");
        return -1;
    }
    cpu_id_str[16] = '\0';

    sscanf(cpu_id_str, "%llX", &cpu_id_info);
    dbg_info("llu=%llX, hi=%X, lo=%X\n", cpu_id_info.llu, cpu_id_info.hilo.hi, cpu_id_info.hilo.lo);

    return cpu_id_info.llu;
}
