/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: RealTimeClock.c
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

/***************************** Include files *******************************/
#include <FreeRTOS.h>
#include <semphr.h>
#include "RealTimeClock.h"

/*****************************   Variables   *******************************/
static TimeOfDay current_time = {0, 0, 0};
static INT32U uptime_seconds = 0;
static xSemaphoreHandle rtc_mutex = NULL;

/*****************************   Functions   *******************************/
void init_rtc(void) {
    rtc_mutex = xSemaphoreCreateMutex();
    configASSERT(rtc_mutex != NULL);
    
    current_time.hour = 0;
    current_time.minute = 0;
    current_time.second = 0;
    uptime_seconds = 0;
}

void set_time(INT8U hour, INT8U minute, INT8U second) {
    if (xSemaphoreTake(rtc_mutex, portMAX_DELAY)) {
        if (hour < 24) {
            current_time.hour = hour;
        }
        if (minute < 60) {
            current_time.minute = minute;
        }
        if (second < 60) {
            current_time.second = second;
        }
        xSemaphoreGive(rtc_mutex);
    }
}

TimeOfDay get_time(void) {
    TimeOfDay result = {0, 0, 0};
    
    if (xSemaphoreTake(rtc_mutex, portMAX_DELAY)) {
        result = current_time;
        xSemaphoreGive(rtc_mutex);
    }
    
    return result;
}

void increment_rtc(void) {
    if (xSemaphoreTake(rtc_mutex, portMAX_DELAY)) {
        current_time.second++;
        uptime_seconds++;
        
        if (current_time.second >= 60) {
            current_time.second = 0;
            current_time.minute++;
            
            if (current_time.minute >= 60) {
                current_time.minute = 0;
                current_time.hour++;
                
                if (current_time.hour >= 24) {
                    current_time.hour = 0;
                }
            }
        }
        
        xSemaphoreGive(rtc_mutex);
    }
}

INT32U get_uptime_seconds(void) {
    INT32U result = 0;
    
    if (xSemaphoreTake(rtc_mutex, portMAX_DELAY)) {
        result = uptime_seconds;
        xSemaphoreGive(rtc_mutex);
    }
    
    return result;
}

/****************************** End Of Module *******************************/
