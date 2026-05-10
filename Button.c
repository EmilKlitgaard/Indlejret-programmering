/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: Button.c
*
* PROJECT....:
*
* DESCRIPTION: Button handling using FreeRTOS tasks
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 080219  MoH    Module created.
* 260415  User   Converted to FreeRTOS task
*
*****************************************************************************/

/***************************** Include files *******************************/
#include "Button.h"
#include "Print.h"
#include "GPIO.h"
#include "InputManager.h"
#include "QueueManager.h"
#include <FreeRTOS.h>
#include <task.h>

/*****************************   Constants   *******************************/
/*****************************   Variables   *******************************/
static INT8U last_sw1 = 1;  // PF4 button state (active low)
static INT8U last_sw2 = 1;  // PF0 button state (active low)

/*****************************   Functions   *******************************/
// Initialize button GPIO and states
void init_button(void) {
    // Unlocks the GPIO_CR register (FP0).
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;   // From tm4c123gh6pm.h
    GPIO_PORTF_CR_R |= 0x01;             // Allow changes to PF0 (bit 0)
    
    // Configure button pins (PF4, PF0) as inputs
    GPIO_PORTF_DIR_R &= ~(0x01 | 0x10);          // Input
    GPIO_PORTF_DEN_R |= (0x01 | 0x10);           // Digital enable
    GPIO_PORTF_PUR_R |= (0x01 | 0x10);           // Enable pull-up
}


// Polling-based button reading function
INT8U read_button_sw1(void) {
    return GPIO_PORTF_DATA_R & BUTTON_SW1;  // Read PF4 (SW1)
}
INT8U read_button_sw2(void) {
    return GPIO_PORTF_DATA_R & BUTTON_SW2;  // Read PF0 (SW2)
}


/**
 * Button task - Polls buttons periodically for state changes
 * 
 * Button behavior:
 * - Active low: 1 = not pressed, 0 = pressed
 * - On press (transition from 1 to 0), send event to QueueManager
 * - Only active when SYSTEM_MODE is MODE_BUTTON
 */
void button_task(void *pvParameters) {
    INT8U current_sw1, current_sw2;
    INT8U last_sw1, last_sw2;
    INT8U system_mode;

    // Add button input queues to QueueManager
    create_queue(BUTTON_INPUT_SW1, 0);
    create_queue(BUTTON_INPUT_SW2, 0);
    
    // Initialize button states (active low = 1, pressed = 0)
    last_sw1 = read_button_sw1();
    last_sw2 = read_button_sw2();
    
    while (true) {
        system_mode = read_input(INPUT_MODE);
        
        if (system_mode == MODE_BUTTON) {
            // Read current button states
            current_sw1 = read_button_sw1();
            current_sw2 = read_button_sw2();
            
            /* Detect button press: transition from non-zero (not pressed) to 0 (pressed) */
            if ((last_sw1 != 0) && (current_sw1 == 0)) {
                last_sw1 = current_sw1;
                put_queue(BUTTON_INPUT_SW1, current_sw1);
                print_var(current_sw1);
            } else if ((last_sw1 == 0) && (current_sw1 != 0)) {
                /* Button released: update state */
                last_sw1 = current_sw1;
            }
            
            if ((last_sw2 != 0) && (current_sw2 == 0)) {
                last_sw2 = current_sw2;
                put_queue(BUTTON_INPUT_SW2, current_sw2);
                print_var(current_sw2);
            } else if ((last_sw2 == 0) && (current_sw2 != 0)) {
                /* Button released: update state */
                last_sw2 = current_sw2;
            }
        }

        // Scan buttons periodically
        sleep_ms(BUTTON_SCAN_MS);
    }
}