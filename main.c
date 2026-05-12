/****************************   FREERTOS RTOS VERSION   ****************************/
/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: main.c (FreeRTOS version)
*
* PROJECT....: Tiva Microcontroller with FreeRTOS
*
* DESCRIPTION: Main application with FreeRTOS task scheduler
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260415  User  Complete FreeRTOS conversion
*
*****************************************************************************/

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdbool.h>

#include "tm4c123gh6pm.h"
#include "data_type.h"

// FreeRTOS includes
#include <FreeRTOS.h>
#include <task.h>
#include "systick_frt.h"

// Hardware modules
#include "GPIO.h"
#include "Print.h"
#include "Sleep.h"
#include "Display.h"

// Input modules
#include "Button.h"
#include "Numpad.h"
#include "Encoder.h"

// System modules
#include "InputManager.h"
#include "QueueManager.h"
#include "LedManager.h"

// Application modules
#include "VendingMachine.h"
#include "RealTimeClock.h"
#include "UARTLogger.h"
#include "ApplicationManager.h"

/*****************************    Defines    *******************************/
#define USERTASK_STACK_SIZE     configMINIMAL_STACK_SIZE
#define IDLE_PRIO               0
#define LOW_PRIO                1
#define MED_PRIO                2
#define HIGH_PRIO               3

/*****************************   Functions   *****************************/
static void init_hardware(void) {
    // Initialize GPIO (LEDs and buttons)
    init_gpio();
    
    // Initialize UART for printing
    init_print();

    // Initialize LCD display
    init_display();
    
    // Initialize hardware modules
    init_button();
    init_numpad();
    init_encoder();
    
    // Initialize system managers
    init_rtc();
    init_input_manager();
    init_queue_manager();
    init_uart_logger();
    
    // Initialize application
    init_vending_machine();
    init_application_manager();

    // Initialize system clock and SysTick for FreeRTOS
    init_systick();
}



int main(void) {
    // Setup all hardware
    init_hardware();

    // Print startup message
    print_str("\n===== COFFEE VENDING MACHINE STARTING =====\n");

    // Create FreeRTOS Tasks:
    print_str("Creating tasks...\n");
    
    // Input handling task    
    xTaskCreate(button_task, "Button", USERTASK_STACK_SIZE, NULL, MED_PRIO, NULL);
    xTaskCreate(numpad_task, "Numpad", USERTASK_STACK_SIZE * 2, NULL, MED_PRIO, NULL);                  
    xTaskCreate(encoder_task, "Encoder", USERTASK_STACK_SIZE * 2, NULL, MED_PRIO, NULL);                
    
    // System management tasks
    xTaskCreate(led_manager_task, "LED_Manager", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);            
    xTaskCreate(rtc_task, "RTC", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
    
    // Application tasks
    xTaskCreate(application_task, "Application", USERTASK_STACK_SIZE * 2, NULL, MED_PRIO, NULL);

    // UART interface task for PC communication
    xTaskCreate(uart_logger_task, "UART_Logger", USERTASK_STACK_SIZE * 2, NULL, LOW_PRIO, NULL);

    // Start the FreeRTOS scheduler
    print_str("Starting scheduler...\n");
    vTaskStartScheduler();

    return 0;
}
