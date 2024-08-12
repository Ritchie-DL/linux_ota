/*
Created by Ritchie TangWei on 2024/8/12.
*/

#include "dl_flash.h"
#include "my_debug.h"

#include "emmc_flash.h"

typedef struct {
    int (*open)(const char *path);
    int (*read)(int fd, uint64_t offset, uint8_t *buffer, uint64_t length);
    int (*write)(int fd, uint64_t offset, const uint8_t *buffer, uint64_t length);
    int (*erase)(int fd, uint64_t offset, uint64_t length);
    int (*close)(int fd);
} dl_flash_ops_t;

static dl_flash_ops_t g_flash_ops;
static bool g_flash_init;

int dl_flash_init(void)
{
    int ret = 0;

#ifdef DL_FLASH_TYPE_SPI_NAND
#elif (defined(DL_FLASH_TYPE_SPI_NAND))
#else
    g_flash_ops.open = rk_block_open;
    g_flash_ops.read = rk_block_read;
    g_flash_ops.write = rk_block_write;
    g_flash_ops.erase = rk_block_erase;
    g_flash_ops.close = rk_block_close;
#endif
    if (assert_ptr(g_flash_ops.open) || assert_ptr(g_flash_ops.read) || assert_ptr(g_flash_ops.write) ||
        assert_ptr(g_flash_ops.erase) || assert_ptr(g_flash_ops.close)) {
        return -1;
    }
    g_flash_init = true;

    return ret;
}

int dl_flash_open_by_name(const char *path)
{
    int ret = 0;

    if (assert_ptr(path)) {
        return -1;
    }

    if (g_flash_init == false) {
        return -1;
    }
    if (assert_ptr(g_flash_ops.open)) {
        return -1;
    }
    ret = g_flash_ops.open(path);
    if (ret < 0) {
        dbg_err("rk_block_open\n");
        return -1;
    }

    return ret;
}

int dl_flash_write(int fd, uint64_t offset, const uint8_t *src_buf, uint64_t src_size)
{
    int ret = 0;

    if (g_flash_init == false) {
        return -1;
    }
    if (assert_ptr(g_flash_ops.write)) {
        return -1;
    }
    ret = g_flash_ops.write(fd, offset, src_buf, src_size);
    if (ret < 0) {
        dbg_err("rk_block_write failed\n");
        return -1;
    }

    return ret;
}

int dl_flash_read(int fd, uint64_t offset, uint8_t *dest_buf, uint64_t dest_size)
{
    int ret = 0;

    if (g_flash_init == false) {
        return -1;
    }
    if (assert_ptr(g_flash_ops.read)) {
        return -1;
    }
    ret = g_flash_ops.read(fd, offset, dest_buf, dest_size);
    if (ret < 0) {
        dbg_err("rk_block_read failed\n");
        return -1;
    }

    return ret;
}

int dl_flash_close(int fd)
{
    int ret = 0;

    if (g_flash_init == false) {
        return -1;
    }
    if (assert_ptr(g_flash_ops.close)) {
        return -1;
    }
    ret = g_flash_ops.close(fd);

    return ret;
}

int dl_flash_deinit(void)
{
    g_flash_init = false;

    return 0;
}