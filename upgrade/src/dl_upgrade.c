/*
Created by Ritchie TangWei on 2024/8/13.
*/

#include "dl_upgrade.h"

#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include "my_debug.h"
#include "rk_ab_meta.h"
#include "../../ota/src/common.h"
#include "dl_flash.h"
#include "my_types.h"

#define UPGRADE_BLOCK_PREFIX  "/dev/block/by-name"
#define AB_UBOOT_NAME "uboot"
#define AB_BOOT_NAME "boot"
#define AB_ROOTFS_NAME "system"

#define UPGARDE_PARTITION_MAX                   4
#define UPGARDE_DIR_PATH_MAX                    128

typedef struct {
    bool is_init;
    char temp_dir[UPGARDE_DIR_PATH_MAX];
    pthread_mutex_t mutex;
} upgrade_ctrl_t;

static upgrade_ctrl_t g_upgrade_ctrl;

#define UPGRADE_CHECK_INIT()        if (!g_upgrade_ctrl.is_init) {      \
                                        dbg_err("upgrade not init\n");  \
                                        return -1;                      \
                                     }

int rk_upgrade_init(const char *temp_dir)
{
    if (assert_ptr(temp_dir)) {
        return -1;
    }
    if (g_upgrade_ctrl.is_init) {
        dbg_warn("upgrade has been inited\n");
        return 0;
    }
    snprintf(g_upgrade_ctrl.temp_dir, sizeof(temp_dir) - 1, "%s", temp_dir);

    g_upgrade_ctrl.is_init = true;

    return 0;
}

static int upgrade_do_write(int fd, const char *image_url, size_t src_size, const char *dest_part)
{
    int ret = 0;
    dl_flash_info_t flash_info = {0};
    uint8_t *buf_ptr = NULL;
    uint64_t offset = 0;
    FILE *fp = NULL;
    uint32_t block_size = 0;
    size_t read_len = 0;

    if (assert_ptr(image_url) || assert_ptr(dest_part)) {
        return -1;
    }

    ret = dl_flash_get_info(fd, &flash_info);
    if ((ret < 0) || (flash_info.block_size == 0)) {
        dbg_err("dl_flash_get_info failed\n");
        dl_flash_close(fd);
        return -1;
    }
    block_size = flash_info.block_size;

    fp = fopen(image_url, "rb");
    if (assert_ptr(fp)) {
        return -1;
    }
    buf_ptr = (uint8_t *)calloc(1, block_size + 1);
    if (assert_ptr(buf_ptr)) {
        fclose(fp);
        return -1;
    }
    for (offset = 0; offset < src_size; offset += block_size) {
        read_len = fread(buf_ptr, 1, block_size, fp);
        if (read_len == 0) {
            dbg_err("read file %s failed\n", image_url);
            break;
        }
        ret = dl_flash_erase(fd, offset, block_size);
        if (ret < 0) {
            dbg_err("dl_flash_erase failed\n");
            break;
        }
        ret = dl_flash_write(fd, offset, buf_ptr, MIN(read_len, block_size));
        if (ret < 0) {
            dbg_err("dl_flash_write failed\n");
            break;
        }
    }

    free(buf_ptr);
    fclose(fp);

    return 0;
}

static int upgrade_write_partition(const char *image_url, const char *dest_part)
{
    int ret = 0;
    int fd = -1;
    struct stat image_state;

    if (assert_ptr(image_url) || assert_ptr(dest_part)) {
        return -1;
    }

    ret = stat(image_url, &image_state);
    if (ret) {
        LOGE("stat %s failed.(%s)\n", image_url, strerror(errno));
        return ret;
    }

    ret = dl_flash_open_by_name(image_url);
    if (ret < 0) {
        dbg_err("dl_flash_open_by_name failed\n");
        return -1;
    }
    fd = ret;
    ret = upgrade_do_write(fd, image_url, image_state.st_size, dest_part);
    if (ret < 0) {
        dbg_err("upgrade_do_write failed\n");
    }
    dl_flash_close(fd);

    return ret;
}

static int upgrade_partition(const char *part_name, const char *temp_dir, const char *part_dest)
{
    int ret = 0;
    char src_path[UPGARDE_DIR_PATH_MAX] = {0};

    if (assert_ptr(temp_dir) || assert_ptr(part_name) || assert_ptr(part_dest)) {
        return -1;
    }

    memset(src_path, 0, sizeof(src_path));
    sprintf(src_path, "%s/%s.img", temp_dir, part_name);
    if (access(src_path, F_OK) == 0) {
        ret = upgrade_write_partition(src_path, part_dest);
        if (ret < 0) {
            dbg_err("write %s failed.\n", part_dest);
            return -1;
        }
    }

    return 0;
}

int rk_upgrade_packet(const char *packet)
{
    int ret = 0;
    uint32_t i = 0;
    char unpack_tar_cmd[UPGARDE_DIR_PATH_MAX] = {0};
    char dest_path[UPGARDE_PARTITION_MAX][UPGARDE_DIR_PATH_MAX] = {0};
    const char *partition_name[] = {
            AB_UBOOT_NAME,
            AB_BOOT_NAME,
            AB_ROOTFS_NAME,
    };
    int slot = 0;

    if (assert_ptr(packet)) {
        return -1;
    }
    UPGRADE_CHECK_INIT();

    slot = rk_ab_meta_active_current_slot();
    if (slot == -1) {
        dbg_err("rk_ab_meta_active_current_slot failed\n");
        return -1;
    }

    if (access(g_upgrade_ctrl.temp_dir, F_OK)) {
        mkdir(g_upgrade_ctrl.temp_dir, 0755);
    }
    sprintf(unpack_tar_cmd, "tar -xf %s -C %s", packet, g_upgrade_ctrl.temp_dir);
    if (system(unpack_tar_cmd)) {
        dbg_err("unpack %s failed.\n", packet);
        return -1;
    }
    dbg_info("packet path=%s, save path=%s\n", packet, g_upgrade_ctrl.temp_dir);

    for (i = 0; i < ARRAY_SIZE(partition_name); i++) {
        sprintf(&dest_path[i][0], "%s/%s_%c", UPGRADE_BLOCK_PREFIX, partition_name[i], (slot == 0) ? 'b' : 'a');
        ret = upgrade_partition(partition_name[i], g_upgrade_ctrl.temp_dir, &dest_path[i][0]);
        if (ret < 0) {
            dbg_err("upgrade_partition %s failed, dest_path=%s\n", partition_name[i], &dest_path[i][0]);
            break;
        }
    }
    return ret;
}

int rk_upgrade_deinit(void)
{
    UPGRADE_CHECK_INIT();
    return 0;
}
