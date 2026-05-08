/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: QueueManager.c
*
* PROJECT....:
*
* DESCRIPTION: Simple thread-safe queue manager using FreeRTOS mutexes
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260415  User  Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>
#include "QueueManager.h"
#include "global_variables.h"

/*****************************   Structures   *******************************/
typedef struct {
    INT32U *buffer;
    INT16U head;
    INT16U tail;
    INT16U count;
    INT16U capacity;
    SemaphoreHandle_t mutex;
    int used;
} Queue;

/*****************************   Variables   *******************************/
static Queue queues[QM_MAX_QUEUES];
static SemaphoreHandle_t manager_mutex = NULL;

/*****************************   Functions   *******************************/

void init_queue_manager(void) {
    // Initialize manager mutex
    if (manager_mutex == NULL) {
        manager_mutex = xSemaphoreCreateMutex();
        configASSERT(manager_mutex != NULL);
        memset(queues, 0, sizeof(queues));
    }
}

int create_queue(INT8U queue_id, INT16U capacity) {
    if (queue_id >= QM_MAX_QUEUES) return INVALID_QUEUE_ID;
    if (capacity == 0) capacity = 32;  /* Default capacity */
    if (capacity > QM_MAX_CAPACITY) return INVALID_QUEUE_ID;

    if (xSemaphoreTake(manager_mutex, portMAX_DELAY) != pdTRUE) return INVALID_QUEUE_ID;
    
    /* Allocate at specified queue ID */
    Queue *q = &queues[queue_id];
    if (q->used) {
        xSemaphoreGive(manager_mutex);
        return INVALID_QUEUE_ID;  /* Queue already exists */
    }
    /* Allocate buffer */
    q->buffer = (INT32U *) pvPortMalloc(sizeof(INT32U) * capacity);
    if (q->buffer == NULL) { xSemaphoreGive(manager_mutex); return INVALID_QUEUE_ID; }
    
    /* Initialize queue fields */
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->capacity = capacity;
    
    /* Create per-queue mutex */
    q->mutex = xSemaphoreCreateMutex();
    if (q->mutex == NULL) { 
        vPortFree(q->buffer); 
        q->buffer = NULL; 
        xSemaphoreGive(manager_mutex); 
        return INVALID_QUEUE_ID; 
    }
    
    q->used = 1;
    xSemaphoreGive(manager_mutex);
    return 0;  /* Success */
}

int put_queue(QueueId qid, INT32U item) {
    if (qid < 0 || qid >= QM_MAX_QUEUES) return INVALID_QUEUE_ID;
    
    Queue *q = &queues[qid];
    if (!q->used) return INVALID_QUEUE_ID;

    if (xSemaphoreTake(q->mutex, portMAX_DELAY) != pdTRUE) return INVALID_QUEUE_ID;
    
    /* Check if queue is full */
    if (q->count >= q->capacity) { 
        xSemaphoreGive(q->mutex); 
        return INVALID_QUEUE_ID; 
    }
    
    /* Add item to back */
    q->buffer[q->tail] = item;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    
    xSemaphoreGive(q->mutex);
    return 0;
}

int get_queue(QueueId queue_id) {
    if (queue_id < 0 || queue_id >= QM_MAX_QUEUES) return INVALID_QUEUE_ID;
    
    Queue *q = &queues[queue_id];
    if (!q->used) return INVALID_QUEUE_ID;

    if (xSemaphoreTake(q->mutex, portMAX_DELAY) != pdTRUE) return INVALID_QUEUE_ID;
    
    /* Check if queue is empty */
    if (q->count == 0) { 
        xSemaphoreGive(q->mutex); 
        return INVALID_QUEUE_ID; 
    }
    
    /* Get item from front */
    INT32U out_item = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    
    xSemaphoreGive(q->mutex);
    return out_item;
}

/****************************** End Of Module *******************************/
