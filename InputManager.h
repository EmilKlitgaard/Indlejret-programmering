/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: InputManager.h
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

#pragma once

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "data_type.h"
#include "global_variables.h"

/*****************************    Defines    *******************************/

/*****************************   Constants   *******************************/

/*****************************   Functions   *******************************/
void init_input_manager(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Initialize input manager (mutexes, initial inputs)
******************************************************************************/

void set_input(INT8U input_id, INT8U value);
/****************************************************************************
*   Input    : Input ID, Value to set
*   Output   : -
*   Function : Thread-safe input setter
******************************************************************************/

INT8U read_input(INT8U input_id);
/****************************************************************************
*   Input    : Input ID
*   Output   : Input value
*   Function : Thread-safe input getter
******************************************************************************/

/****************************** End Of Module *******************************/
