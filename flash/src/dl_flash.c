/*
Created by Ritchie TangWei on 2024/8/12.
*/
#include "dl_flash.h"

#include <unistd.h>
#include <fcntl.h>

#include "emmc_flash.h"
#include "nand_flash.h"
#include "nor_flash.h"

#include "my_debug.h"

#define PROC_CMDLINE    "/proc/cmdline"

#define FLASH_CHECK_INIT()      if (g_flash_init == false) { \
                                    dbg_err("flash not init\n"); \
                                    return -1;               \
                                }

#define FLASH_CHECK_FD(fd)      if (fd < 0) {                        \
                                    dbg_err("invalid  fd(%d)\n", fd);\
                                    return -1;                       \
                                }

typedef struct {
    int (*open)(const char *path);
    int (*read)(int fd, uint64_t offset, uint8_t *buffer, uint64_t length);
    int (*write)(int fd, uint64_t offset, const uint8_t *buffer, uint64_t length);
    int (*erase)(int fd, uint64_t offset, uint64_t length);
    int (*close)(int fd);
} dl_flash_ops_t;

static dl_flash_ops_t g_flash_ops;
static bool g_flash_init;

static dl_flash_type_t get_flash_type(void)
{
    char param[2048] = {0};
    int ret = 0;
    int fd = open(PROC_CMDLINE, O_RDONLY);
    if (fd < 0) {
        dbg_err("open %s failed\n", PROC_CMDLINE);
        return -1;
    }
    ret = read(fd, (char*)param, 2048);
    close(fd);
    if (ret <= 0) {
        dbg_err("read failed\n");
        return -1;
    }

    if (strstr(param, "emmc") != NULL) {
        return DL_FLASH_EMMC;
    } else if (strstr(param, "nand") != NULL) {
        dbg_info("Now is MTD.\n");
        return DL_FLASH_SPI_NAND;
    } else if (strstr(param, "nor") != NULL) {
        return DL_FLASH_SPI_NOR;
    } else {
        dbg_err("No found nand in cmdline, default is not MTD.\n");
        return DL_FLASH_BUTT;
    }
}

int dl_flash_init(void)
{
    int ret = 0;
    dl_flash_type_t flash_type = get_flash_type();

    switch (flash_type) {
        case DL_FLASH_EMMC:
            g_flash_ops.open = rk_block_open;
            g_flash_ops.read = rk_block_read;
            g_flash_ops.write = rk_block_write;
            g_flash_ops.erase = rk_block_erase;
            g_flash_ops.close = rk_block_close;
            break;

        case DL_FLASH_SPI_NAND:
            g_flash_ops.open = rk_nand_open;
            g_flash_ops.read = rk_nand_read;
            g_flash_ops.write = rk_nand_write;
            g_flash_ops.erase = rk_nand_erase;
            g_flash_ops.close = rk_nand_close;
            break;

        case DL_FLASH_SPI_NOR:
            g_flash_ops.open = rk_nor_open;
            g_flash_ops.read = rk_nor_read;
            g_flash_ops.write = rk_nor_write;
            g_flash_ops.erase = rk_nor_erase;
            g_flash_ops.close = rk_nor_close;
            break;

        default:
            dbg_err("flash type %u not supported\n", flash_type);
            return -1;
    }

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

    FLASH_CHECK_INIT();
    if (assert_ptr(g_flash_ops.open)) {
        return -1;
    }
    ret = g_flash_ops.open(path);
    if (ret < 0) {
        dbg_err("rk_block_open %s failed\n", path);
        return -1;
    }

    return ret;
}

int dl_flash_write(int fd, uint64_t offset, const uint8_t *src_buf, uint64_t src_size)
{
    int ret = 0;

    FLASH_CHECK_INIT();
    FLASH_CHECK_FD(fd);
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

    FLASH_CHECK_INIT();
    FLASH_CHECK_FD(fd);

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

    FLASH_CHECK_INIT();
    FLASH_CHECK_FD(fd);

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