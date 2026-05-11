/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: LedManager.c
*
* PROJECT....:
*
* DESCRIPTION: Unified LED management task handling both status indicator
*              and mode/state indicator LEDs
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260415  User  Module created (merged status_led and led_controller)
*
*****************************************************************************/

/***************************** Include files *******************************/
#include "LedManager.h"
#include "GPIO.h"
#include "InputManager.h"
#include "Sleep.h"
#include <FreeRTOS.h>
#include <task.h>
#include "tm4c123gh6pm.h"

/*****************************   Defines    *******************************/
#define STATUS_LED_PIN  0x40        // PD6 for status indicator
#define BLINK_INTERVAL  500         // Status LED blink interval (ms)
#define STATE_UPDATE_INTERVAL 100   // Mode LED update interval (ms)

/*****************************   Functions   *******************************/

/**
 * Initialize GPIO for status LED (PD6)
 */
static void init_status_led_gpio(void) {
    // Enable the GPIO port D
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD;

    // Do a dummy read to insert a few cycles after enabling the peripheral
    int dummy = SYSCTL_RCGC2_R;

    // Configure PD6 as output for status LED
    GPIO_PORTD_DIR_R |= STATUS_LED_PIN;
    GPIO_PORTD_DEN_R |= STATUS_LED_PIN;
}


/**
 * Toggle status LED
 */
static void toggle_status_led(void) {
    GPIO_PORTD_DATA_R ^= STATUS_LED_PIN;
}


/**
 * Main LED Manager Task
 * Handles:
 * 1. Status LED (PD6) - Blinks continuously
 * 2. Mode LED (PF1-3) - Changes color based on system state
 */
void led_manager_task(void *pvParameters) {
    INT8U system_mode;
    uint32_t status_tick = 0;
    uint32_t state_tick = 0;
    
    // Initialize status LED GPIO
    init_status_led_gpio();
    
    while (true) {
        // --- STATUS LED MANAGEMENT ---
        // Blink status LED every BLINK_INTERVAL ms
        status_tick++;
        if (status_tick >= (BLINK_INTERVAL / 10)) {
            toggle_status_led();
            status_tick = 0;
        }
        
        // Sleep for 10ms tick
        sleep_ms(10);
    }
}
