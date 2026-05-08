/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: FreeRTOSHooks.c
*
* PROJECT....: Coffee Vending Machine
*
* DESCRIPTION: FreeRTOS application hooks
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260506  User  Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include <FreeRTOS.h>
#include <task.h>
#include "tm4c123gh6pm.h"
#include "GPIO.h"

/*****************************   Functions   *******************************/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;

    taskDISABLE_INTERRUPTS();
    GPIO_PORTF_DATA_R = RED;

    for (;;) {
    }
}

void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    GPIO_PORTF_DATA_R = RED | BLUE;

    for (;;) {
    }
}
