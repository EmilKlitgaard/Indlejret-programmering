/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: ApplicationManager.h
*
* PROJECT....: Coffee Vending Machine
*
* DESCRIPTION: Application state machine and task management
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260506  User  Module created.
*
*****************************************************************************/

#pragma once

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "data_type.h"
#include "VendingMachine.h"

/*****************************    Defines    *******************************/
#define INVALID_INPUT   0xFF

// System delays (in milliseconds)
#define LCD_UPDATE_DELAY    200
#define TRANSITION_DELAY    1000

/*****************************    Types    *******************************/

/* Menu navigation state */
typedef struct {
    INT8U current_index;
    INT8U input_latch;
    INT8U last_rendered_index;
} MenuState;

/* Payment input state */
typedef struct {
    INT16U cash_amount;
    INT8U card_input[16];
    INT8U pin_input[4];
    INT8U input_index;
    INT8U input_stage;
} PaymentState;

/* Production state */
typedef struct {
    INT32U start_ticks;
    INT32U filter_start_ticks;
    INT32U filter_last_activity_ticks;
    INT16U filter_dispensed_cl_x100;
    INT8U production_stage;
} ProductionState;

/* Cup detection and start states */
typedef struct {
    BOOLEAN cup_detected;
    BOOLEAN start_pressed;
} InteractionState;

/* Complete application state machine context */
typedef struct {
    INT8U system_state;
    INT8U selected_product;
    INT8U payment_method;
    
    MenuState product_menu;
    MenuState payment_menu;
    PaymentState payment_state;
    ProductionState production_state;
    InteractionState interaction_state;
    
    VendingTransaction *transaction;
    xSemaphoreHandle transaction_semaphore;
} AppStateContext;

/*****************************   Functions   *******************************/

void init_application_manager(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Initialize application manager
******************************************************************************/

void application_task(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Application state machine task
******************************************************************************/

void uart_interface_task(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : UART command processing and logging task
******************************************************************************/

/****************************** End Of Module *******************************/
