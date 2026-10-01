#ifndef RING_QUEUE_H
#define RING_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

typedef struct RingQueue RingQueue;

RingQueue*   RingQueueCtor(size_t capacity, size_t element_size);
RingQueue*   RingQueueDtor(RingQueue* queue);

void*        RingQueuePush(RingQueue* queue, void* element);
void*        RingQueuePop(RingQueue* queue);

bool         RingQueueEmpty(RingQueue* queue);

#endif /* RING_QUEUE_H */
