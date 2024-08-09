//
// Created by rich on 2024/3/22.
//

#include "pt_semaphoer.h"

#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include "my_debug.h"
#include "utils.h"

#if PT_USE_MY_SEM
pt_semaphore_t *pt_sem_create(void)
{
    pthread_condattr_t condattr = {0};
    pt_semaphore_t *sem = (pt_semaphore_t *)dl_malloc(sizeof (pt_semaphore_t));
    if (assert_ptr(sem)) {
        return NULL;
    }
    sem->num = 0;
    pthread_mutex_init(&sem->mutex, NULL);
    pthread_condattr_init(&condattr);
    pthread_cond_init(&sem->cond, NULL);
    return sem;
}

void pt_sem_destroy(pt_semaphore_t *sem)
{
    if (assert_ptr(sem)) {
        return;
    }
    if (sem != NULL) {
        free(sem);
    }
}

int pt_sem_signal(pt_semaphore_t *sem)
{
    if (assert_ptr(sem)) {
        return -1;
    }
    dbg_lo("in\n");
    pthread_mutex_lock(&sem->mutex);
    sem->num += 1;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
    dbg_lo("out\n");
    return 0;
}

int pt_sem_wait(pt_semaphore_t *sem)
{
    if (assert_ptr(sem)) {
        return -1;
    }
    dbg_lo("sem:%p in\n", sem);
    pthread_mutex_lock(&sem->mutex);
    if (sem->num > 0) {
        sem->num -= 1;
    }
    pthread_cond_wait(&sem->cond, &sem->mutex);
    pthread_mutex_unlock(&sem->mutex);
    dbg_lo("out\n");

    return 0;
}

int pt_sem_wait_time(pt_semaphore_t *sem, uint32_t time_ms)
{
    struct timespec timedout = {0};
    struct timeval now = {0};
    int ret = 0;

    if (assert_ptr(sem)) {
        return -1;
    }
    gettimeofday(&now, NULL);
    timedout.tv_sec = now.tv_sec + time_ms / 1000;
    timedout.tv_nsec = now.tv_usec * 1000 + (time_ms % 1000) * 10000000;

    dbg_lo("%p:sem->num=%d\n", sem, sem->num);
    pthread_mutex_lock(&sem->mutex);
    while (sem->num <= 0 && ret != ETIMEDOUT) {
        ret = pthread_cond_timedwait(&sem->cond, &sem->mutex, &timedout);
    }
    if (sem->num > 0) {
        sem->num -= 1;
    }
    pthread_mutex_unlock(&sem->mutex);
    dbg_lo("out\n");
    if (ret != 0) {
        switch (ret) {
            case EINVAL:
                dbg_err("EINVAL\n");
                break;
            case EPERM:
                dbg_err("EPERM\n");
                break;
            case ETIMEDOUT:
                dbg_lo("ETIMEDOUT\n");
                return ETIMEDOUT;
            default:
                dbg_err("pthread_cond_timedwait error, time_ms=%u, ret=%d\n", time_ms, ret);
                break;
        }
        return -1;
    }
    dbg_lo("[%s]%p:sem->num=%d\n", __func__, sem, sem->num);
    return 0;
}

int pt_sem_broadcast(pt_semaphore_t *sem)
{
    if (assert_ptr(sem)) {
        return -1;
    }
    dbg_lo("sem:%p in\n", sem);
    pthread_mutex_lock(&sem->mutex);
    sem->num += 1;
    int ret = pthread_cond_broadcast(&sem->cond);
    if (ret < 0) {
        dbg_err("pthread_cond_broadcast error\n");
    } else {
        ret = 0;
    }
    pthread_mutex_unlock(&sem->mutex);
    dbg_lo("out\n");
    return ret;
}
#else
pt_semaphore_t *pt_sem_create(void)
{
    pt_semaphore_t *sem = dl_malloc(sizeof(pt_semaphore_t));
    if (assert_ptr(sem)) {
        return NULL;
    }
    int ret = sem_init(sem, 0, 0);
    if (ret < 0) {
        free(sem);
        return NULL;
    }
    return sem;
}

void pt_sem_destroy(pt_semaphore_t *sem)
{
    if (assert_ptr(sem)) {
        return;
    }
    if (sem != NULL) {
        free(sem);
    }
}

int pt_sem_signal(pt_semaphore_t *sem)
{
    if (assert_ptr(sem)) {
        return -1;
    }
    int ret = sem_post(sem);
    if (ret < 0) {
        dbg_err("sem_post failed\n");
        return -1;
    }
    return 0;
}

int pt_sem_wait(pt_semaphore_t *sem)
{
    if (assert_ptr(sem)) {
        return -1;
    }

    int ret = sem_wait(sem);
    if (ret < 0) {
        dbg_err("sem_wait failed\n");
        return -1;
    }
    return 0;
}

int pt_sem_wait_time(pt_semaphore_t *sem, uint32_t time_ms)
{
    struct timespec ts = {0};
    int ret = 0;

    if (assert_ptr(sem)) {
        return -1;
    }

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        dbg_err("clock_gettime");
        return -1;
    }
    ts.tv_sec += time_ms / 1000;
    ts.tv_nsec += time_ms * 1000000;
    ret = sem_timedwait(sem, &ts);
    if (ret == -1) {
        if (errno == ETIMEDOUT) {
            dbg_warn("time out\n");
            return ETIMEDOUT;
        } else {
            dbg_err("sem_timedwait failed, time_ms=%u\n", time_ms);
            return -1;
        }
    }
    return 0;
}

int pt_sem_broadcast(pt_semaphore_t *sem)
{
    if (assert_ptr(sem)) {
        return -1;
    }
    int ret = sem_post(sem);
    if (ret < 0) {
        dbg_err("sem_post failed\n");
        return -1;
    }
    return ret;
}
#endif
/* moved from ipc_ctrl.h */

int sem_get_val(int semid, int semnum)
{
    //semun_t sem_info = {1};
    int ret = 0;
    if (semid < 0 || semnum < 0) {
        dbg_err("bad params input\n");
    }
    ret = semctl(semid, semnum, GETVAL);
    if (ret < 0) {
        perror("\nsemctl");
    }
    return ret;
}

int sem_set_val(int semid, int semnum, int val)
{
    semun_t sem_info;
    int ret = 0;

    if (semid < 0 || semnum < 0) {
        dbg_err("bad params input\n");
    }

    sem_info.val = val;
    ret = semctl(semid, semnum, SETVAL, sem_info);
    if (ret < 0) {
        dbg_perror("\nsemctl");
        dbg_err("semctl failed,ret=%d,semid=%d, semnum=%d\n", ret, semid, semnum);
    } else {
        dbg_lo("setval: semval=%08X\n", sem_get_val(semid, semnum));
    }
    return ret;
}

int sem_p(int fd, int idx)
{
    struct sembuf p_array = {0, -1, SEM_UNDO};

    if (fd < 0 || idx < 0) {
        dbg_err("sem v, bad params\n");
        return -1;
    }

    dbg_lo("p0:the semval=%d\n", sem_get_val(fd, idx));
    p_array.sem_num = idx;
    if (semop(fd, &p_array, 1) == -1) {
        dbg_perror("semop");
        dbg_err("semop,p failed, senval=%d\n", sem_get_val(fd, idx));
        return -1;
    } else {
        dbg_lo("p1:the semval=%d\n", sem_get_val(fd, idx));
    }
    return 0;
}

int sem_v(int fd, int idx)
{
    struct sembuf v_array = {0, 1, SEM_UNDO};

    if (fd < 0 || idx < 0) {
        dbg_err("sem v, bad params\n");
        return -1;
    }

    dbg_lo("fd=%d, idx=%d\n", fd, idx);
    v_array.sem_num = idx;
    if (semop(fd, &v_array, 1) == -1) {
        dbg_perror("semop");
        dbg_err("semop,v failed\n");
        return -1;
    }

    return 0;
}

static int _path_to_key(const char *const path, int key_id)
{
    if (access(path, F_OK) != 0)	{
        perror("access");
        return -1;
    }

    key_t k_sem = ftok(".", key_id);

    if (k_sem < 0) {
        perror("ftok");
        dbg_err("ftok failed\n");
        return -1;
    } else {
        dbg_lo("semph: key=0x%08X\n", k_sem);
    }

    return k_sem;
}

int sem_ctrl_create(const char *const path, int key_id, int nsems)
{
    if (assert_ptr(path)) {
        return -1;
    }
    if (nsems <= 0) {
        dbg_err("nsems=%d\n", nsems);
        return -1;
    }

    key_t k_sem = _path_to_key(path, key_id);
    int semfd = semget(k_sem, 1, IPC_PRIVATE|SEM_UNDO|IPC_CREAT|S_IRUSR|S_IWUSR);

    if (semfd < 0) {
        dbg_perror("semget");
        dbg_err("semget failed,semfd=%d\n",semfd);
        int error_no = errno;
        switch(error_no) {
            case EEXIST:
                semfd = semget(k_sem, nsems, IPC_CREAT|SEM_UNDO);
                if (semfd < 0) {
                    perror("semget again");
                    dbg_err("semget again failed,semfd=%d\n",semfd);
                    return -1;
                }
                break;
                /*
                case EACCESS:
                    dbg_err("access\n");
                    break;
                */
            default:
                dbg_err("errno=%d\n", error_no);
                return -1;
        }
    }else {
        sem_set_val(semfd, 0, 1);
        dbg_lo("semph: semfd=%d\n", semfd);
    }

    return semfd;
}

int sem_ctrl_destroy(int sem_id)
{
    int ret = 0;
    if (sem_id < 0) {
        dbg_err("bad params input\n");
        return -1;
    }
    ret = semctl(sem_id, 0, IPC_RMID);
    if (ret < 0) {
        dbg_err("semctl failed, sem_id=%d, ret=%d\n", sem_id, ret);
        perror("semctl");
    }

    return ret;
}

int msg_ctrl_create(char *path, int key_id)
{
    int fd = -1;
    key_t k_msg = _path_to_key(path, key_id);
    fd = msgget(k_msg, IPC_CREAT|0666);
    if (-1 == fd) {
        dbg_err(" error\n");
        return -1;
    } else {
        dbg_lo("msgid is %d\n", fd);
    }

    return fd;
}

int msg_ctrl_destroy(int fd)
{
    int ret = 0;
    if (fd < 0) {
        dbg_err("bad params input\n");
        return -1;
    }
    ret = msgctl(fd, IPC_RMID, NULL);
    if (ret < 0) {
        dbg_err("msgctl failed\n");
        ret = -1;
    }
    close(fd);
    fd = -1;

    return ret;
}

int named_fifo_create(const char * const path, int opt)
{
    int fd = -1, ret = 0;
    if (assert_ptr(path)) {
        return -1;
    }

    if (access(path, F_OK) == -1) {
        //file not exsit, create it
        ret= mkfifo(path, 0777);
        if (ret != 0) {
            perror("mkfifo:");
            dbg_err("mkfifo [%s] failed\n", path);
            exit(EXIT_FAILURE);
        }
    }

    /*
    if (-1 == mkfifo(path, O_CREAT|O_EXCL|0777)) {
        perror("mkfifo:");
        dbg_err("mkfifo [%s] failed\n", path);
    }*/

    switch(opt) {
        case 1:
            opt = O_WRONLY;
            dbg_lo("open %s as write only\n", path);
            break;

        case 0:
        default:
            opt = O_RDONLY;
            dbg_lo("open %s as read only\n", path);
            break;
    }

    fd = open(path, opt);
    if (fd < 0) {
        perror("open:");
        dbg_err("open %s failed\n", path);
    }
    return fd;
}

int nfifo_put(int fd, unsigned char *data, int len)
{
    if (assert_ptr(data)) {
        return -1;
    }

    if (write(fd, data, len) != len) {
        dbg_err("write fifo err\n");
        return -1;
    }

    return 0;
}

int nfifo_get(int fd, unsigned char *dst_buf, int len)
{
    int ret = 0;
    if (assert_ptr(dst_buf)) {
        return -1;
    }

    ret = read(fd, dst_buf, len);
    if (ret < 0) {
        perror("read:");
        dbg_err("read fifo err\n");
        return -1;
    }

    return ret;
}
