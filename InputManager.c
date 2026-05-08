/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: InputManager.c
*
* PROJECT....:
*
* DESCRIPTION: Input management with mutex protection for FreeRTOS
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
#include "InputManager.h"

/*****************************   Variables   *******************************/
// System input array: [SYSTEM_MODE]
static INT8U system_inputs[1] = {0};

// Number of inputs (for bounds checking)
static const INT8U MAX_INPUTS = sizeof(system_inputs) / sizeof(system_inputs[0]);

// Mutex semaphore
static xSemaphoreHandle input_mutex = NULL;

/*****************************   Functions   *******************************/
void init_input_manager(void) {
    // Create mutex for input protection (before any task uses it)
    input_mutex = xSemaphoreCreateMutex();
    
    // Verify mutex creation succeeded
    configASSERT(input_mutex != NULL);
    
    // Initialize all system inputs
    system_inputs[INPUT_MODE] = MODE_IDLE;
}


void set_input(INT8U input_id, INT8U value) {
    // Validate input_id is within bounds
    if (input_id >= MAX_INPUTS) {
        return;
    }
    
    // Validate mutex exists
    if (input_mutex == NULL) {
        return;
    }
    
    // Take mutex with max delay (blocking call within task context)
    if (xSemaphoreTake(input_mutex, portMAX_DELAY)) {
        system_inputs[input_id] = value;
        xSemaphoreGive(input_mutex);
    }
}


INT8U read_input(INT8U input_id) {
    INT8U value = 0;
    
    // Validate input_id is within bounds
    if (input_id >= MAX_INPUTS) {
        return 0;
    }
    
    // Validate mutex exists
    if (input_mutex == NULL) {
        return 0;
    }
    
    // Take mutex with max delay (blocking call within task context)
    if (xSemaphoreTake(input_mutex, portMAX_DELAY)) {
        value = system_inputs[input_id];
        xSemaphoreGive(input_mutex);
    }
    
    return value;
}
