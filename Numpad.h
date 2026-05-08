/****************************************************************************
* Odense University College of Enginerring
* Embedded C Programming (ECP)
*
* MODULENAME.: Numpad.h
*
* PROJECT....:
*
* DESCRIPTION:
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 050128  KA    Module created.
*
*****************************************************************************/

#pragma once

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include "tm4c123gh6pm.h"
#include "global_variables.h"
#include "data_type.h"
#include "InputManager.h"
#include "QueueManager.h"
#include "Print.h"
#include "Sleep.h"

/*****************************    Defines    *******************************/
#define INVALID_NUMPAD_INPUT    0xFF    // Invalid numpad input

// Debounce and scan timing
#define NUMPAD_DEBOUNCE_MS  20
#define NUMPAD_SCAN_MS      50

// Numpad GPIO ports
#define PA2     0x04    // KEYB D (Input, Active LOW)
#define PA3     0x08    // KEYB E (Input, Active LOW)
#define PA4     0x10    // KEYB F (Input, Active LOW)
#define PE0     0x01    // KEYB G (Output, standard LOW)
#define PE1     0x02    // KEYB H (Output, standard LOW)
#define PE2     0x04    // KEYB J (Output, standard LOW)
#define PE3     0x08    // KEYB K (Output, standard LOW)

// Numpad matrix
#define NUMPAD_ROWS     4
#define NUMPAD_COLS     3

// Masks
#define COL_MASK        0x1C        // PA2, PA3, PA4
#define ROW_MASK        0x0F        // PE0, PE1, PE2, PE3


/*****************************   Types   ***********************************/
/*****************************   Constants   *******************************/
/*****************************   Functions   *******************************/
extern void init_numpad(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Initialize numpad GPIO pins.
******************************************************************************/


INT8U scan_numpad(void);
/****************************************************************************
*   Input    : -
*   Output   : Numpad number
*   Function : Returns the pressed number
******************************************************************************/


extern void numpad_task(void *pvParameters);
/****************************************************************************
*   Input    : FreeRTOS task parameter (unused)
*   Output   : -
*   Function : FreeRTOS task for numpad scanning
******************************************************************************/





