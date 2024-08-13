/*
Created by Ritchie TangWei on 2024/8/13.
*/

#include "nand_flash.h"

#include "my_debug.h"

int rk_nand_open(const char *block_path)
{
    if (assert_ptr(block_path)) {
        return -1;
    }
    return 0;
}

int rk_nand_write(int fd, uint64_t offset, const uint8_t *src_buf, uint64_t src_size)
{
    if (assert_ptr(src_buf)) {
        return -1;
    }
    if (fd < 0) {
        dbg_err("invalid fd(%d)\n", fd);
        return -1;
    }
    return 0;
}

int rk_nand_read(int fd, uint64_t offset, uint8_t *dest_buf, uint64_t dest_size)
{
    if (assert_ptr(dest_buf)) {
        return -1;
    }
    if (fd < 0) {
        dbg_err("invalid fd(%d)\n", fd);
        return -1;
    }
    return 0;
}

int rk_nand_erase(int fd, uint64_t offset, uint64_t length)
{
    if (fd < 0) {
        dbg_err("invalid fd(%d)\n", fd);
        return -1;
    }
    return 0;
}

int rk_nand_close(int fd)
{
    if (fd < 0) {
        dbg_err("invalid fd(%d)\n", fd);
        return -1;
    }
    return 0;
}
