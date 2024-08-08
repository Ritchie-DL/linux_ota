#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/reboot.h>

#include "bootloader.h"
#include "my_debug.h"

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

int main(int argc, char *argv[])
{
    dbg_info("*** rk_ota: Version V1.0.0 ***.\n");
    int arg;
    char *tar_path = NULL;
    char *save_dir = NULL;
    char *misc_func = NULL;
    char *partition = NULL;
    char *extra_part = NULL;
    bool is_reboot = false;


    while ((arg = getopt_long(argc, argv, "", engine_options, NULL)) != -1) {
        switch (arg) {
            case 'm': misc_func = optarg; continue;
            case 't' + 'p': tar_path = optarg; continue;
            case 's' + 'p': save_dir = optarg; continue;
            case 'p': partition = optarg; continue;
            case 'e' + 'p': extra_part = optarg; continue;
            case 'r': is_reboot = true; continue;
            case 'h': usage(); break;
            case '?':
                dbg_err("Invalid command argument\n");
                continue;
        }
    }

    if (misc_func != NULL) {
        if (strcmp(misc_func, "now") == 0) {
            if (setSlotSucceed() ==0) {
                m_status = RK_UPGRADE_FINISHED;
            }
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
