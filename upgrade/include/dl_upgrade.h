/*
Created by Ritchie TangWei on 2024/8/13.
*/

#ifndef __DL_UPGRADE_H
#define __DL_UPGRADE_H

#include "hi_eventhub.h"

#if defined (__cplusplus)
extern "C" {
#endif

enum {
    RK_UPGRADE_RESULT_SUCCESS = HI_EVENTHUB_EVENT_TYPE(HI_EVENT_MOD_UPGRADE, 0),
    RK_UPGRADE_RESULT_FAILURE,
    RK_UPGRADE_REPORT_PROGRESS,
    RK_UPGRADE_PACKET_FOUND,
    RK_UPGRADE_NEEDS_RECOVERY,
};

int rk_upgrade_init(const char *backup_dir, const char *temp_dir);
int rk_upgrade_packet(const char *packet);
int rk_upgrade_deinit(void);

#if defined (__cplusplus)
}
#endif

#endif //__DL_UPGRADE_H
