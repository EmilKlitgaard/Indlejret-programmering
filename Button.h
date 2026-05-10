/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: Button.h
*
* PROJECT....:
*
* DESCRIPTION: Button handling for FreeRTOS
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 050128  KA    Module created.
* 260415  User  Converted to FreeRTOS
*
*****************************************************************************/

#pragma once

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdbool.h>
#include "data_type.h"
#include "tm4c123gh6pm.h"
#include "global_variables.h"
#include "InputManager.h"
#include "QueueManager.h"
#include "Sleep.h"

/*****************************    Defines    *******************************/
// Debounce delays
#define BUTTON_DEBOUNCE_MS  20
#define BUTTON_SCAN_MS      50

// Button GPIO pins (Port F)
#define BUTTON_SW1          0x10    // PF4 (SW1)
#define BUTTON_SW2          0x01    // PF0 (SW2)

/*****************************   Constants   *******************************/
extern volatile INT8U pending_button;

/*****************************   Functions   *******************************/
extern INT8U read_button_sw1(void);
/****************************************************************************
*   Input    : -   
*   Output   : Button state (0 or 1)
*   Function : Read SW1 button state (active low)
******************************************************************************/


extern INT8U read_button_sw2(void);
/****************************************************************************
*   Input    : -
*   Output   : Button state (0 or 1)
*   Function : Read SW2 button state (active low)
******************************************************************************/


extern void init_button_handler(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Initialize button handler
******************************************************************************/


extern void button_task(void *pvParameters);
/****************************************************************************
*   Input    : FreeRTOS task parameter (unused)
*   Output   : -
*   Function : FreeRTOS task for button handling
******************************************************************************/
