/*
Created by Ritchie TangWei on 2024/8/8.
*/

#include "emmc_flash.h"

#include <fcntl.h>
#include <unistd.h>

#include "my_debug.h"
#include "my_types.h"

int rk_block_open(const char *block_path)
{
    if (assert_ptr(block_path)) {
        return -1;
    }
    int fd = open(block_path, O_RDWR, 0644);
    if (fd < 0) {
        dbg_err("Can't open %s\n", block_path);
        return -1;
    }
    
    return fd;
}

int rk_block_write(int fd, uint64_t offset, const uint8_t *src_buf, uint64_t src_size)
{
    int ret = 0;
    const uint8_t *data_buf = NULL;
    uint64_t write_count = 0;
    uint64_t idx = 0;

    if (assert_ptr(src_buf)) {
        return -1;
    }
    if ((src_size == 0) || (fd < 0)) {
        dbg_err("invalid args,src_size=%llX, fd=%d\n", src_size, fd);
        return -1;
    }

    ret = lseek64(fd, (int64_t)offset, SEEK_SET);
    if (ret < 0) {
        dbg_err("lseek64 failed(%s). fd=%d, ret=%d\n", strerror(errno), fd, ret);
        return -1;
    }
    
    for (idx = 0; idx < src_size; idx += BLOCK_WRITE_LEN) {
        data_buf = &src_buf[idx];
        write_count = MIN(src_size - idx, BLOCK_WRITE_LEN);
        ret = write(fd, data_buf, write_count);
        if (ret != write_count) {
            dbg_err("fd=%d, ret=%d, count=%lld, write failed(%s).\n",  fd, ret, write_count, strerror(errno));
            return -1;
        }
    }

    fsync(fd);

    return 0;
}

int rk_block_read(int fd, uint64_t offset, uint8_t *dest_buf, uint64_t dest_size)
{
    int ret = 0;

    if (assert_ptr(dest_buf)) {
        return -1;
    }
    if (fd < 0) {
        dbg_err("invalid fd=%d\n", fd);
        return -1;
    }

    ret = lseek(fd, (long)offset, SEEK_SET);
    if (ret < 0) {
        dbg_err("lseek failed\n");
        return -1;
    }

    int count = read(fd, dest_buf, dest_size);
    if (count != dest_size) {
        dbg_err("count=%d, dest_size=%#X, reading failed(%s)\n", count, dest_size, strerror(errno));
        return -1;
    }

    return 0;
}

int rk_block_erase(int fd, uint64_t offset, uint64_t length)
{
    if (fd < 0) {
        dbg_err("invalid fd=%d\n", fd);
        return -1;
    }
    return 0;
}

int rk_block_close(int fd)
{
    if (fd < 0) {
        dbg_err("invalid fd=%d\n", fd);
        return -1;
    }
    close(fd);
    return 0;
}