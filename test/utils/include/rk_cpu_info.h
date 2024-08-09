//
// Created by rich on 2024/3/18.
//

#ifndef __RK_CPU_INFO_H
#define __RK_CPU_INFO_H

#include <stdint.h>

int rk_read_proc_cpuinfo(char *dest_buf, uint32_t dest_len);
uint64_t rk_read_cpu_id(void);

#endif //__RK_CPU_INFO_H
