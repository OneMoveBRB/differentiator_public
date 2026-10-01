#include "../include/ring_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define FREE(ptr)           \
    do {                    \
        free(ptr);          \
        ptr = NULL;         \
    } while (0);

#define VOID_PTR(base, offset, size) (void*)((char*)(base) + (offset) * (size))
#define RQ_IDX(rq, i)                VOID_PTR(rq->data, i, rq->element_size)

struct RingQueue {
    void*  data;
    size_t element_size;
    size_t capacity;
    size_t head;
    size_t tail;
};

static const unsigned int kInitCapacity = 8;
static const unsigned int kExpMul       = 2;

static inline bool  RingQueueFull(RingQueue* queue);
static void*        RingQueueReallocUp(RingQueue* queue);

RingQueue* RingQueueCtor(size_t capacity, size_t element_size) {
    RingQueue* queue = (RingQueue*)calloc(1, sizeof(RingQueue));
    if (queue == NULL) {
        return NULL;
    }

    queue->element_size = element_size;
    queue->capacity = capacity ? capacity : kInitCapacity;
    queue->data = calloc(queue->capacity, element_size);
    if (queue->data == NULL) {
        FREE(queue);
        return NULL;
    }

    queue->head = 0;
    queue->tail = 0;

    return queue;
}

RingQueue* RingQueueDtor(RingQueue* queue) {
    if (queue == NULL) return NULL;

    FREE(queue->data);
    FREE(queue);

    return NULL;
}

void* RingQueuePush(RingQueue* queue, void* element) {
    assert( queue   != NULL );
    assert( element != NULL );

    if (RingQueueFull(queue)) {
        if (RingQueueReallocUp(queue) == NULL) {
            return NULL;
        }
    }

    size_t queue_tail = queue->tail;
    void*  data_dest  = RQ_IDX(queue, queue_tail);

    memcpy(data_dest, element, queue->element_size);
    queue->tail = (queue_tail + 1 == queue->capacity) ? 0 : queue_tail + 1;

    return data_dest;
}

void* RingQueuePop(RingQueue* queue) {
    assert( queue != NULL );

    if (RingQueueEmpty(queue)) {
        return NULL;
    }

    size_t queue_head = queue->head;
    void* element = RQ_IDX(queue, queue_head);
    queue->head = (queue_head + 1 == queue->capacity) ? 0 : queue_head + 1;

    return element;
}

bool RingQueueEmpty(RingQueue* queue) {
    return queue->head == queue->tail;
}

static inline bool RingQueueFull(RingQueue* queue) {
    return (queue->tail + 1) % queue->capacity == queue->head;
}

static void* RingQueueReallocUp(RingQueue* queue) {
    assert( queue != NULL );

    size_t old_head = queue->head;
    size_t old_tail = queue->tail;
    size_t old_capacity = queue->capacity;
    size_t element_size = queue->element_size;

    size_t new_data_size = 0;
    size_t new_capacity = old_capacity * kExpMul;
    void*  new_data = calloc(new_capacity, element_size);
    if (new_data == NULL) {
        return NULL;
    }

    if (old_tail > old_head) {
        new_data_size = old_tail - old_head;
        memcpy(new_data, RQ_IDX(queue, old_head), new_data_size * element_size);
    } else {
        size_t first_part  = queue->capacity - queue->head;
        size_t second_part = old_tail;
        new_data_size = first_part + second_part;

        memcpy(new_data, RQ_IDX(queue, old_head), first_part * element_size);
        memcpy(VOID_PTR(new_data, first_part, element_size), queue->data, second_part * element_size);
    }

    FREE(queue->data);

    queue->data = new_data;
    queue->capacity = new_capacity;
    queue->head = 0;
    queue->tail = new_data_size;

    return new_data;
}
