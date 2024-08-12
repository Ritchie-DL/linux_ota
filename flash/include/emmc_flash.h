/*
Created by Ritchie TangWei on 2024/8/8.
*/

#ifndef __EMMC_FLASH_H
#define __EMMC_FLASH_H

#include <stdint.h>

#if defined (__cplusplus)
extern "C" {
#endif

int rk_block_open(const char *block_path);
int rk_block_write(int fd, uint64_t offset, const uint8_t *src_buf, uint64_t src_size);
int rk_block_read(int fd, uint64_t offset, uint8_t *dest_buf, uint64_t dest_size);
int rk_block_close(int fd);

#if defined (__cplusplus)
}
#endif

#endif //__EMMC_FLASH_H
