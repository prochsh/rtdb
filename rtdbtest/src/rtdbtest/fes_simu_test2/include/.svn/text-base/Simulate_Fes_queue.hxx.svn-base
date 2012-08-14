#ifndef _SIMULATE_FES_QUEUE_H_
#define _SIMULATE_FES_QUEUE_H_

typedef struct
{
    //SGDPS_CALGCHG buf[QUEUE_SIZE_SIMULATE_FES];
    void *buf;
    int size,length;
    int head,tail;
    pthread_mutex_t *mutex;
    pthread_cond_t *notFull,*notEmpty;
} SIMFES_QUEUE;

SIMFES_QUEUE * SimFes_queue_init (int size);
void SimFes_queue_add(SIMFES_QUEUE *q, void *item);
void* SimFes_queue_del (SIMFES_QUEUE *q);
void SimFes_queue_destroy (SIMFES_QUEUE*q);

#endif
