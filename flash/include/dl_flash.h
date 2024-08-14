/*
Created by Ritchie TangWei on 2024/8/12.
*/

#ifndef __DL_FLASH_H
#define __DL_FLASH_H

#include <stdint.h>

#if defined (__cplusplus)
extern "C" {
#endif

typedef enum {
    DL_FLASH_EMMC = 0,
    DL_FLASH_SPI_NAND,
    DL_FLASH_SPI_NOR,
    DL_FLASH_BUTT,
} dl_flash_type_t;

typedef struct {
    uint64_t total_size;               /**< flash total size */
    uint64_t part_size;                /**< flash partition size */
    uint32_t block_size;               /**< flash block size */
    uint32_t page_size;                /**< flash page size */
    uint32_t oob_size;                 /**< flash OOB size */
    uint64_t open_addr;                /**< flash open address */
    uint64_t open_len;                /**< flash open length */
    dl_flash_type_t flash_type;          /**< flash type */
} dl_flash_info_t;

int dl_flash_init(void);
int dl_flash_open_by_name(const char *path);
int dl_flash_write(int fd, uint64_t offset, const uint8_t *src_buf, uint64_t src_size);
int dl_flash_erase(int fd, uint64_t offset, uint64_t src_size);
int dl_flash_read(int fd, uint64_t offset, uint8_t *dest_buf, uint64_t dest_size);
int dl_flash_close(int fd);
int dl_flash_get_info(int fd, dl_flash_info_t *info);
int dl_flash_deinit(void);

#if defined (__cplusplus)
}
#endif

#endif //__DL_FLASH_H
