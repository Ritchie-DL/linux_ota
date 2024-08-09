//
// Created by rich on 2024/3/22.
//

#include "pthread_attr.h"

#include "my_debug.h"

int get_thread_policy(pthread_attr_t *attr)
{
    int policy = SCHED_RR;
    int ret = 0;
    ret = pthread_attr_getschedpolicy(attr, &policy);
    if (ret < 0) {
        dbg_err("error\n");
        return -1;
    }
    switch (policy){
        case SCHED_FIFO:
            dbg_lo("SCHED_FIFO\n");
            break;

        case SCHED_RR:
            dbg_lo("SCHED_RR\n");
            break;

        case SCHED_OTHER:
            dbg_lo("SCHED_OTHER\n");
            break;
        default:
            dbg_info("SCHED Unknown\n");
            break;
    }

    return policy;
}

int set_thread_policy(int policy)
{
    pthread_attr_t attr = {0};
    int ret = get_thread_policy(&attr);
    if (ret < 0) {
        dbg_err("get_thread_policy error\n");
        return -1;
    }
    ret = pthread_attr_setschedpolicy(&attr, policy);
    if(0 != ret) {
        dbg_err("set policy\n");
        return -1;
    }
    return ret;
}

int set_thread_attr_policy(pthread_attr_t *attr, int policy)
{
    int ret = pthread_attr_setschedpolicy(attr, policy);
    if(0 != ret) {
        dbg_err("set policy\n");
    } else {
        get_thread_policy(attr);
    }
    return ret;
}
