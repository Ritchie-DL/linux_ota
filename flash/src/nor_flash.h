/*
Created by Ritchie TangWei on 2024/8/13.
*/

#ifndef __NOR_FLASH_H
#define __NOR_FLASH_H

#include <stdint.h>

#if defined (__cplusplus)
extern "C" {
#endif

int rk_nor_open(const char *block_path);
int rk_nor_write(int fd, uint64_t offset, const uint8_t *src_buf, uint64_t src_size);
int rk_nor_read(int fd, uint64_t offset, uint8_t *dest_buf, uint64_t dest_size);
int rk_nor_erase(int fd, uint64_t offset, uint64_t length);
int rk_nor_close(int fd);

#if defined (__cplusplus)
}
#endif

#endif //__NOR_FLASH_H
