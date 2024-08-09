//
// Created by rich on 2024/3/22.
//

#ifndef __SEMAPHOER_H
#define __SEMAPHOER_H

#include <pthread.h>
#include <stdint.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define PT_USE_MY_SEM           1

#if PT_USE_MY_SEM
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    long num;
} pt_semaphore_t;
#else
#include <semaphore.h>
typedef  sem_t  pt_semaphore_t;
#endif

typedef union semun {
    int 			 val;	 /* Value for SETVAL */
    struct semid_ds *buf;	 /* Buffer for IPC_STAT, IPC_SET */
    unsigned short	*array;  /* Array for GETALL, SETALL */
    struct seminfo	*__buf;  /* Buffer for IPC_INFO	(Linux-specific) */
} semun_t;

pt_semaphore_t *pt_sem_create(void);
void pt_sem_destroy(pt_semaphore_t *sem);
int pt_sem_signal(pt_semaphore_t *sem);
int pt_sem_wait(pt_semaphore_t *sem);
int pt_sem_wait_time(pt_semaphore_t *sem, uint32_t time_ms);
int pt_sem_broadcast(pt_semaphore_t *sem);

/*-----------------------------------------------------------------------------------*/
int sem_get_val(int semid, int semnum);
int sem_set_val(int semid, int semnum, int val);
int sem_p(int fd, int idx);
int sem_v(int fd, int idx);
int sem_ctrl_create(const char *path, int key_id, int nsems);
int sem_ctrl_destroy(int sem_id);
int msg_ctrl_create(char *path, int key_id);
int msg_ctrl_destroy(int fd);
int named_fifo_create(const char *path, int opt);
int nfifo_put(int fd, unsigned char *data, int len);
int nfifo_get(int fd, unsigned char *dst_buf, int len);

#if defined (__cplusplus)
}
#endif

#endif //__SEMAPHOER_H
