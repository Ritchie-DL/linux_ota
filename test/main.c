#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/reboot.h>

#include "bootloader.h"
#include "my_debug.h"
#include "rk_flash.h"
#include "rk_ab_meta.h"

typedef enum{
    RK_UPGRADE_FINISHED,
    RK_UPGRADE_START,
    RK_UPGRADE_ERR,
}RK_Upgrade_Status_t;

RK_Upgrade_Status_t m_status = RK_UPGRADE_ERR;

static void usage(void) 
{
    dbg_info("--misc=now                           Linux A/B mode: Setting the current partition to bootable.\n");
    dbg_info("--misc=other                         Linux A/B mode: Setting another partition to bootable.\n");
    dbg_info("--misc=update                        Linux A/B mode: Setting the partition to be upgraded.\n");
    dbg_info("--misc=display                       Display misc info.\n");
    dbg_info("--tar_path=<path>                    Set upgrade firmware path.\n");
    dbg_info("--save_dir=<path>                    Set the path for saving the image.\n");
    dbg_info("--partition=<uboot/boot/system/all>  Set the partition to be upgraded.('all' means 'uboot', 'boot' and 'system' are included.)\n");
    dbg_info("--extra_part=<name>                  Set the extra partition to be upgraded.\n");
    dbg_info("--reboot                             Restart the machine at the end of the program.\n");
}

static const struct option engine_options[] = {
        { "misc", required_argument, NULL, 'm' },
        { "tar_path", required_argument, NULL, 't' + 'p'},
        { "save_dir", required_argument, NULL, 's' + 'p'},
        { "partition", required_argument, NULL, 'p' },
        { "extra_part", required_argument, NULL, 'e' + 'p' },
        { "reboot", no_argument, NULL, 'r' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 },
};

int test_read_misc(void)
{
#define MISC_PARTITION_NAME_BLOCK "/dev/block/by-name/misc"
    int ret = 0;
    AvbABData info_ab = {0};

    int fd = dl_flash_open_by_name(MISC_PARTITION_NAME_BLOCK);
    if (fd < 0) {
        dbg_err("dl_flash_open_by_name failed\n");
        return -1;
    }
    ret = dl_flash_read(fd, MISC_OFFSET, (uint8_t *)&info_ab, sizeof(info_ab));
    if (ret < 0) {
        dbg_err("rk_block_read failed\n");
        dl_flash_close(fd);
        return -1;
    }
    dl_flash_close(fd);
    
    dbg_info("magic : %s.\n", info_ab.magic);
    dbg_info("version_major = %d.\n", info_ab.version_major);
    dbg_info("version_minor = %d.\n", info_ab.version_minor);
    for (int i = 0; i < 2; i++) {
        dbg_info("slot.[%d]->priority = %d.\n",        i, info_ab.slots[i].priority);
        dbg_info("slot.[%d]->successful_boot = %d.\n", i, info_ab.slots[i].successful_boot);
        dbg_info("slot.[%d]->tries_remaining = %d.\n", i, info_ab.slots[i].tries_remaining);
    }

    dbg_info("last_boot : %d.\n", info_ab.last_boot);
    dbg_info("local crc32: %x.\n", info_ab.crc32);
    dbg_info("sizeof(struct AvbABData) = %ld\n", sizeof(struct AvbABData));

    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int arg;
    char *tar_path = NULL;
    char *save_dir = NULL;
    char *misc_func = NULL;
    char *partition = NULL;
    char *extra_part = NULL;
    bool is_reboot = false;

    dbg_info("\n");

    test_read_misc();
    while ((arg = getopt_long(argc, argv, "", engine_options, NULL)) != -1) {
        switch (arg) {
            case 'm':
                misc_func = optarg;
                continue;
            case 't' + 'p':
                tar_path = optarg;
                continue;
            case 's' + 'p':
                save_dir = optarg;
                continue;
            case 'p':
                partition = optarg;
                continue;
            case 'e' + 'p':
                extra_part = optarg;
                continue;
            case 'r':
                is_reboot = true;
                continue;
            case 'h':
                usage();
                break;
            case '?':
                dbg_err("Invalid command argument\n");
                usage();
                continue;
        }
    }

    if (misc_func != NULL) {
        if (strcmp(misc_func, "now") == 0) {
//            ret = setSlotSucceed();
            ret = rk_ab_meta_active_current_slot();
            if (ret < 0) {
                dbg_err("\n");
            }
            m_status = RK_UPGRADE_FINISHED;
            rk_ab_meta_active_another_slot();
        } else if (strcmp(misc_func, "other") == 0) {
            if (setSlotActivity() == 0) {
                m_status = RK_UPGRADE_FINISHED;
            }
        } else if (strcmp(misc_func, "update") == 0) {
            if (miscUpdate(tar_path, save_dir, partition, extra_part) == 0) {
                m_status = RK_UPGRADE_FINISHED;
            }
        } else if (strcmp(misc_func, "display") == 0) {
            miscDisplay();
        } else {
            dbg_err("unknown misc cmdline : %s.\n", misc_func);
            return 0;
        }
    }

    if (is_reboot && (m_status == RK_UPGRADE_FINISHED)) {
        sync();
        reboot(RB_AUTOBOOT);
        // system(" echo b > /proc/sysrq-trigger ");
    }

    return m_status;
}
