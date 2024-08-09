//
// Created by rich on 2024/3/22.
//

#include "uname_pipe.h"

#include <unistd.h>
#include <stdlib.h>
#include "my_debug.h"
#include "utils.h"

pipe_handle_t pipe_create(void)
{
    pipe_handle_t pipe_hdl = dl_malloc(sizeof(pipe_object));
    if (pipe_hdl == NULL) {
        dbg_err("Alloc failed: pipe object\n");
        return NULL;
    }
    pthread_mutex_init(&pipe_hdl->mutex, NULL);
    if (pipe(pipe_hdl->pipes)) {
        free(pipe_hdl);
        return NULL;
    }

    return pipe_hdl;
}

int pipe_delete(pipe_handle_t pipe_hdl)
{
    int ret = 0;

    if (pipe_hdl != NULL) {
        if (close(pipe_hdl->pipes[0])) {
            ret = -1;
        }

        if (close(pipe_hdl->pipes[1])) {
            ret = -1;
        }
        free(pipe_hdl);
    }

    return ret;
}

int pipe_get(pipe_handle_t pipe_hdl, void *data, int num)
{
    size_t ret = 0;
    if (assert_ptr(pipe_hdl)) {
        return -1;
    }

    ret = read(pipe_hdl->pipes[0], data, num);
    if (ret != num) {
        if (ret == 0) {
            dbg_perror("fd=%d, ret=%zd, expect(%d)", pipe_hdl->pipes[0], ret, num);
        } else {
            dbg_lo("fd=%d, ret=%zd, expect(%d)", pipe_hdl->pipes[0], ret, num);
        }
        return -1;
    }

    return 0;
}

int pipe_put(pipe_handle_t pipe_hdl, const void *data, int num)
{
    if (assert_ptr(pipe_hdl)) {
        return -1;
    }
    pthread_mutex_lock(&pipe_hdl->mutex);
    if (write(pipe_hdl->pipes[1], data, num) != num) {
        dbg_err("write error\n");
        pthread_mutex_unlock(&pipe_hdl->mutex);
        return -1;
    }
    pthread_mutex_unlock(&pipe_hdl->mutex);
    return 0;
}

