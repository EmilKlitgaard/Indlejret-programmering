/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: GPIO.c
*
* PROJECT....:
*
* DESCRIPTION: GPIO control for LEDs and inputs
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260415  User  Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include "GPIO.h"
#include "Sleep.h"

/*****************************    Defines    *******************************/

/*****************************   Constants   *******************************/

/*****************************   Variables   *******************************/

/*****************************   Functions   *******************************/
void init_gpio(void) {
    // Enable the GPIO port F
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;

    // Do a dummy read to insert a few cycles after enabling the peripheral
    int dummy = SYSCTL_RCGC2_R;

    // Configure LED pins (PF1, PF2, PF3) as outputs (digital)
    GPIO_PORTF_DIR_R |= (RED | BLUE | GREEN);    // Output
    GPIO_PORTF_DEN_R |= (RED | BLUE | GREEN);    // Digital enable
    GPIO_PORTF_DATA_R &= ~(RED | BLUE | GREEN);  // Start with all LEDs off
}


void set_led(INT8U color) {
    GPIO_PORTF_DATA_R = color;
}


void clear_led(void) {
    GPIO_PORTF_DATA_R = 0;
}


void toggle_led(INT8U color) {
    GPIO_PORTF_DATA_R ^= color;
}


void blink_led(INT8U color, INT8U count, INT16U delay_ms) {
    int i;
    for (i=0; i<count; i++) {
        set_led(color);
        sleep_ms(delay_ms);
        clear_led();
        sleep_ms(delay_ms);
    }
}
