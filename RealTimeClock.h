/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: RealTimeClock.h
*
* PROJECT....: Coffee Vending Machine
*
* DESCRIPTION: Real-time clock for transaction logging
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
#include "data_type.h"

/*****************************    Defines    *******************************/

/*****************************   Types   *******************************/

typedef struct {
    INT8U hour;
    INT8U minute;
    INT8U second;
} TimeOfDay;

typedef struct {
    INT8U day;
    INT8U month;
    INT16U year;
    INT8U hour;
    INT8U minute;
    INT8U second;
} DateTime;

/*****************************   Functions   *******************************/
extern void init_rtc(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Initialize real-time clock
******************************************************************************/

extern void set_time(INT8U hour, INT8U minute, INT8U second);
/****************************************************************************
*   Input    : Hour (0-23), Minute (0-59), Second (0-59)
*   Output   : -
*   Function : Set current time of day
******************************************************************************/

extern TimeOfDay get_time(void);
/****************************************************************************
*   Input    : -
*   Output   : Current time
*   Function : Get current time of day
******************************************************************************/

extern void increment_rtc(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Increment RTC by one second (called from timer)
******************************************************************************/

extern void rtc_task(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : FreeRTOS task that increments the RTC once per second
******************************************************************************/

extern INT32U get_uptime_seconds(void);
/****************************************************************************
*   Input    : -
*   Output   : Uptime in seconds
*   Function : Get machine operating time in seconds
******************************************************************************/

/****************************** End Of Module *******************************/
