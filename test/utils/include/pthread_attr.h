//
// Created by rich on 2024/3/22.
//

#ifndef __PTHREAD_ATTR_H
#define __PTHREAD_ATTR_H

#include <pthread.h>

#if defined (__cplusplus)
extern "C" {
#endif

int get_thread_policy(pthread_attr_t *attr);
int set_thread_policy(int policy);
int set_thread_attr_policy(pthread_attr_t *attr, int policy);

#if defined (__cplusplus)
  }
#endif

#endif //__PTHREAD_ATTR_H
