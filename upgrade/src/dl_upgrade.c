/*
Created by Ritchie TangWei on 2024/8/13.
*/

#include "dl_upgrade.h"

#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include "my_debug.h"
#include "rk_ab_meta.h"
#include "../../ota/src/common.h"
#include "dl_flash.h"
#include "my_types.h"

#define UPGRADE_BLOCK_PREFIX  "/dev/block/by-name"
#define UPGRADE_VALID_DIR_PREFIX "/userdata"
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

/* ------------------------------------------------------------------------------------------------------------------
 * call this function in advance is a must
 ------------------------------------------------------------------------------------------------------------------ */
int rk_upgrade_init(const char *temp_dir)
{
    int ret = 0;
    char resolved_path[PATH_MAX] = {0};

    if (assert_ptr(temp_dir)) {
        return -1;
    }
    if (g_upgrade_ctrl.is_init) {
        dbg_warn("upgrade has been inited\n");
        return 0;
    }

    if (realpath(temp_dir, resolved_path) == NULL) {
        dbg_err("realpath for %s failed\n", temp_dir);
        return -1;
    }
    if (strcmp(resolved_path, UPGRADE_VALID_DIR_PREFIX) < 0) {
        dbg_err("%s not a valid direction\n", resolved_path);
        return -1;
    }
    if (access(resolved_path, F_OK) != 0) {
        ret = mkdir(resolved_path, 0755);
        if (ret < 0) {
            dbg_err("mkdir for %s failed\n", resolved_path);
        }
    }
    snprintf(g_upgrade_ctrl.temp_dir, sizeof(g_upgrade_ctrl.temp_dir) - 1, "%s", resolved_path);

    g_upgrade_ctrl.is_init = true;

    return 0;
}

static int upgrade_do_write(FILE *fp, int fd, const char *dest_part)
{
    int ret = 0;
    dl_flash_info_t flash_info = {0};
    uint8_t *buf_ptr = NULL;
    uint64_t offset = 0;
    uint32_t block_size = 0;
    size_t read_len = 0;

    if (assert_ptr(fp) || assert_ptr(dest_part)) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    const size_t src_size = ftell(fp);
    rewind(fp);

    ret = dl_flash_get_info(fd, &flash_info);
    if ((ret < 0) || (flash_info.block_size == 0)) {
        dbg_err("dl_flash_get_info failed\n");
        dl_flash_close(fd);
        return -1;
    }
    block_size = flash_info.block_size;

    buf_ptr = (uint8_t *)calloc(1, block_size + 1);
    if (assert_ptr(buf_ptr)) {
        return -1;
    }
    for (offset = 0; offset < src_size; offset += block_size) {
        read_len = fread(buf_ptr, 1, block_size, fp);
        if (read_len == 0) {
            if (offset != block_size) {
                dbg_err("read file failed\n");
                ret = -1;
            }
            break;
        }
        ret = dl_flash_erase(fd, offset, block_size);
        if (ret < 0) {
            dbg_err("dl_flash_erase failed\n");
            break;
        }
        dbg_info("writing %s offset %llX\n", dest_part, offset);
        ret = dl_flash_write(fd, offset, buf_ptr, MIN(read_len, block_size));
        if (ret < 0) {
            dbg_err("dl_flash_write failed\n");
            break;
        }
    }

    free(buf_ptr);

    return ret;
}

static int upgrade_write_partition(const char *image_url, const char *dest_part)
{
    int ret = 0;
    int fd = -1;

    if (assert_ptr(image_url) || assert_ptr(dest_part)) {
        return -1;
    }

    FILE *fp = fopen(image_url, "rb");
    if (assert_ptr(fp)) {
        dbg_err("open %s failed. (%s)\n", image_url, strerror(errno));
        return ret;
    }
    dbg_mark("upgrade partition: %s\n", image_url);

    ret = dl_flash_open_by_name(image_url);
    if (ret < 0) {
        dbg_err("dl_flash_open_by_name failed\n");
        fclose(fp);
        return -1;
    }
    fd = ret;
    ret = upgrade_do_write(fp, fd, dest_part);
    if (ret < 0) {
        dbg_err("upgrade_do_write failed\n");
    }
    dl_flash_close(fd);
    fclose(fp);
    remove(image_url);

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
    const char *part_name[] = {
            AB_UBOOT_NAME,
            AB_BOOT_NAME,
            AB_ROOTFS_NAME,
    };
    char resolved_url[PATH_MAX] = {0};
    int slot = 0;

    if (assert_ptr(packet)) {
        return -1;
    }
    UPGRADE_CHECK_INIT();

    if (assert_ptr(realpath(packet, resolved_url))) {
        dbg_err("realpath for %s failed\n", packet);
        return -1;
    }
    slot = rk_ab_meta_get_running_slot();
    if (slot == -1) {
        dbg_err("rk_ab_meta_get_running_slot failed\n");
        return -1;
    }

    if (access(g_upgrade_ctrl.temp_dir, F_OK)) {
        dbg_err("temp_dir(%s) is not valid\n", g_upgrade_ctrl.temp_dir);
        return -1;
    }

    sprintf(unpack_tar_cmd, "tar -xf %s -C %s", resolved_url, g_upgrade_ctrl.temp_dir);
    if (system(unpack_tar_cmd)) {
        dbg_err("unpack %s failed.\n", resolved_url);
        return -1;
    }
    dbg_info("packet path=%s, save path=%s\n", resolved_url, g_upgrade_ctrl.temp_dir);

    for (i = 0; i < ARRAY_SIZE(part_name); i++) {
        sprintf(&dest_path[i][0], "%s/%s_%c", UPGRADE_BLOCK_PREFIX, part_name[i], rk_ab_meta_get_suffix_by_slot(!slot));
        ret = upgrade_partition(part_name[i], g_upgrade_ctrl.temp_dir, &dest_path[i][0]);
        if (ret < 0) {
            dbg_err("upgrade_partition %s failed, dest_path=%s\n", part_name[i], &dest_path[i][0]);
            break;
        }
    }
    if (ret >= 0) {
        dbg_mark("upgrade finish, active another slot\n");
        rk_ab_meta_active_another_slot();
    }
    return ret;
}

int rk_upgrade_deinit(void)
{
    UPGRADE_CHECK_INIT();
    return 0;
}
