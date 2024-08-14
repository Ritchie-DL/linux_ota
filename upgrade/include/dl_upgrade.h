/*
Created by Ritchie TangWei on 2024/8/13.
*/

#ifndef __DL_UPGRADE_H
#define __DL_UPGRADE_H

#if defined (__cplusplus)
extern "C" {
#endif

int rk_upgrade_init(const char *temp_dir);
int rk_upgrade_packet(const char *packet);
int rk_upgrade_deinit(void);

#if defined (__cplusplus)
}
#endif

#endif //__DL_UPGRADE_H
