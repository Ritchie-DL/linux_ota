#ifndef _HI_EVENTHUB_H
#define _HI_EVENTHUB_H

#include "hi_eventhub_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char name[64];
    hi_event_mod_t mod_id;
    eventhub_func_t func;
} hi_subscriber_t;

int hi_eventhub_init(void);

int hi_eventhub_register_event(uint32_t event_id);

int hi_eventhub_unregister_event(uint32_t event_id);

int hi_eventhub_subscribe(const hi_subscriber_t *subscriber, const uint32_t *event, uint32_t count);

int hi_eventhub_unsubscribe(hi_event_mod_t event_mod, uint32_t event_id);

int hi_eventhub_publish(const hi_eventhub_t *event);

int hi_eventhub_latency_publish(hi_event_mod_t mode_id, const hi_eventhub_t *event, uint32_t seconds, bool loop_en);

int hi_eventhub_latency_remove_loop(hi_event_mod_t event_mod, uint32_t event_id);

void hi_eventhub_deinit(void);

#if defined (__cplusplus)
}
#endif

#endif
