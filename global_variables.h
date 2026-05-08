/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: global_variables.h
*
* PROJECT....:
*
* DESCRIPTION: Defines the elements of the task model..
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 101004  MoH   Module created.
*
*****************************************************************************/

#pragma once

/***************************** States *******************************/
// Input ID's
#define INPUT_MODE              0

// States for INPUT_MODE
#define MODE_IDLE               0
#define MODE_BUTTON             1
#define MODE_NUMPAD             2
#define MODE_ENCODER            3

/***************************** Queues *******************************/
#define BUTTON_INPUT_SW1        0
#define BUTTON_INPUT_SW2        1
#define NUMPAD_INPUT            2
#define ENCODER_INPUT           3

/***************************** Semaphores *******************************/
#define BUTTON_SEMAPHORE        0
#define NUMPAD_SEMAPHORE        1
#define ENCODER_SEMAPHORE       2

