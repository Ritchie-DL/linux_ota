//
// Created by Ritchie on 2024/3/6.
//
#ifndef __HI_EVENTHUB_MSG_H
#define __HI_EVENTHUB_MSG_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVENT_HUB_ARGS_MAX      32
#define EVENTHUB_MAX_EVENTS     64
#define EVENTHUB_MAX_GROUP      16
#define EVENTHUB_MAGIC          'D'

typedef enum {
    HI_EVENT_MOD_UI = 0x0,
    HI_EVENT_MOD_STATEMNG,
    HI_EVENT_MOD_NETCTRL,
    HI_EVENT_MOD_UART,
    HI_EVENT_MOD_GPIO,
    HI_EVENT_MOD_GPIO_KEY,
    HI_EVENT_MOD_TOUCH,
    HI_EVENT_MOD_VIDEO,
    HI_EVENT_MOD_AUDIO,
    HI_EVENT_MOD_ADC,
    HI_EVENT_MOD_WIFI,
    HI_EVENT_MOD_BLE,
    HI_EVENT_MOD_PWM,
    HI_EVENT_MOD_UPGRADE,
    HI_EVENT_MOD_BUTT,
    HI_EVENT_MOD_INVALID = 0xFFFF,
} hi_event_mod_t;

#define HI_EVENTHUB_EVENT_TYPE(mod, idx)       ((EVENTHUB_MAGIC << 24) | (mod << 16) | idx)
#define HI_EVENTHUB_GET_MOD(event_id)          ((event_id >> 16) & 0xFF)

typedef struct {
    uint32_t event_id;
    int result;
    uint32_t key_type;
    unsigned long key_code;
    unsigned long key_value;
    uint32_t argc;
    uint8_t argv[EVENT_HUB_ARGS_MAX];
    uint64_t time_stamp;
} hi_eventhub_t;

typedef int (*eventhub_func_t)(const hi_eventhub_t *event);

#if defined (__cplusplus)
}
#endif

#endif //__HI_EVENTHUB_MSG_H
