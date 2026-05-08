/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: QueueManager.h
*
* PROJECT....:
*
* DESCRIPTION: Queue management with mutex protection for FreeRTOS
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260415  User  Module created.
*
*****************************************************************************/

#pragma once

/***************************** Include files *******************************/
#include "FreeRTOS.h"
#include "semphr.h"
#include "data_type.h"

/*****************************    Defines    *******************************/
#define QM_MAX_QUEUES 8
#define QM_MAX_CAPACITY 64
#define INVALID_QUEUE_ID 0xff

typedef INT8S QueueId;

/*****************************   Constants   *******************************/

/*****************************   Functions   *******************************/
void init_queue_manager(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Initialize queue manager
******************************************************************************/

int create_queue(INT8U queue_id, INT16U capacity);
/****************************************************************************
*   Input    : Queue ID (0-7), Capacity (max items, 1-64, 0=use default 32)
*   Output   : 0 success, -1 error
*   Function : Create thread-safe queue at specified ID with given capacity
******************************************************************************/

int put_queue(QueueId qid, INT32U item);
/****************************************************************************
*   Input    : Queue ID, Item value to add to queue
*   Output   : 0 success, INVALID_QUEUE_ID error (queue full or invalid)
*   Function : Add item to back of queue (mutex-protected, blocking)
******************************************************************************/

int get_queue(QueueId qid);
/****************************************************************************
*   Input    : Queue ID
*   Output   : Item value (success), INVALID_QUEUE_ID
*   Function : Remove and get item from front of queue (mutex-protected, blocking)
******************************************************************************/

/****************************** End Of Module *******************************/
