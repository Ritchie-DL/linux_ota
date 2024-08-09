//
// Created by rich on 2024/3/22.
//

#ifndef __UNAME_PIPE_H
#define __UNAME_PIPE_H

#include <pthread.h>

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct pipe_object_struct {
    pthread_mutex_t mutex;
    int pipes[2];
} pipe_object, *pipe_handle_t;

pipe_handle_t pipe_create(void);
int pipe_delete(pipe_handle_t pipe_hdl);
int pipe_get(pipe_handle_t pipe_hdl, void *data, int num);
int pipe_put(pipe_handle_t pipe_hdl, const void *data, int num);

#if defined (__cplusplus)
}
#endif

#endif //__UNAME_PIPE_H
