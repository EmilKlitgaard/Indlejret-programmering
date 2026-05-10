/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: ApplicationManager.c
*
* PROJECT....: Coffee Vending Machine
*
* DESCRIPTION: Application state machine and task management
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
#include <task.h>
#include <semphr.h>
#include <stdio.h>
#include "ApplicationManager.h"
#include "Display.h"
#include "GPIO.h"
#include "Print.h"
#include "Encoder.h"
#include "Numpad.h"
#include "InputManager.h"
#include "QueueManager.h"
#include "LedManager.h"
#include "Sleep.h"
#include "UARTLogger.h"

/*****************************   Constants   *******************************/

/* Product display strings with prices */
static const char *const PRODUCT_TITLES[] = {
    "Espresso: 15",
    "Latte: 27",
    "Filter: 3/cl"
};

static const char *const PAYMENT_TITLES[] = {
    "Cash",
    "Card"
};

/*****************************   Variables   *******************************/

/* Global application state context */
static AppStateContext app;

/*****************************   Helper Functions   *******************************/

/**
 * Format a display line with text and a numeric value
 * Combines a text prefix with an INT8U number for LCD display
 */
static void format_display_line(char *buffer, const char *text, INT8U value) {
    INT8U i = 0;
    
    /* Copy text to buffer */
    while (text[i] != '\0' && i < 15) {
        buffer[i] = text[i];
        i++;
    }
    
    /* Convert number to string and append */
    if (value < 10) {
        buffer[i++] = '0' + value;
    } else if (value < 100) {
        buffer[i++] = '0' + (value / 10);
        buffer[i++] = '0' + (value % 10);
    } else {
        buffer[i++] = '0' + (value / 100);
        buffer[i++] = '0' + ((value / 10) % 10);
        buffer[i++] = '0' + (value % 10);
    }
    
    buffer[i] = '\0';
}

/**
 * Reset all state machine context to initial values for next transaction
 */
static void reset_app_context(void) {
    app.system_state = STATE_IDLE;
    app.selected_product = INVALID_INPUT;
    app.payment_method = INVALID_INPUT;
    
    app.product_menu.current_index = 0;
    app.product_menu.input_latch = INVALID_INPUT;
    app.product_menu.last_rendered_index = INVALID_INPUT;
    
    app.payment_menu.current_index = 0;
    app.payment_menu.input_latch = INVALID_INPUT;
    app.payment_menu.last_rendered_index = INVALID_INPUT;
    
    app.payment_state.cash_amount = 0;
    app.payment_state.input_index = 0;
    app.payment_state.input_stage = 0;
    
    app.production_state.start_ticks = 0;
    app.production_state.filter_start_ticks = 0;
    app.production_state.filter_last_activity_ticks = 0;
    app.production_state.filter_dispensed_cl_x100 = 0;
    app.production_state.production_stage = PROD_IDLE;
    
    app.interaction_state.cup_detected = false;
    app.interaction_state.start_pressed = false;
    
    app.transaction = NULL;
}

/**
 * Render a menu line with < title >
 */
static void render_slider_line(char *line, const char *title) {
    INT8U i, j;
    for (i = 0; i < 16; i++) {
        line[i] = ' ';
    }
    line[0] = '<';
    line[15] = '>';
    for (i = 2, j = 0; (i < 15) && (title[j] != '\0'); i++, j++) {
        line[i] = title[j];
    }
    line[16] = '\0';
}

/**
 * Step menu index left (with wrap-around)
 */
static void menu_step_left(INT8U *index, INT8U count) {
    if (*index == 0) {
        *index = count - 1;
    } else {
        (*index)--;
    }
}

/**
 * Step menu index right (with wrap-around)
 */
static void menu_step_right(INT8U *index, INT8U count) {
    (*index)++;
    if (*index >= count) {
        *index = 0;
    }
}

/**
 * Check if button is pressed (active low: 0 means pressed)
 */
static BOOLEAN button_pressed(INT8U button_queue_id) {
    INT8U value = get_queue(button_queue_id);
    if (value == INVALID_QUEUE_ID) return false;  /* No new button event */
    return !value;
}

/**
 * Read menu input with latch to prevent repeated input
 */
static INT8U read_menu_input(MenuState *menu) {
    INT8U value = get_queue(NUMPAD_INPUT);
    
    if (value == menu->input_latch) {
        return INVALID_INPUT;
    }
    
    menu->input_latch = value;
    return value;
}


/**
 * Display IDLE state with welcome message
 */
static void display_idle_menu(void) {
    clear_display();
    write_str_at(0, 0, "Welcome!");
    write_str_at(0, 1, "Press button");
    sleep_ms(LCD_UPDATE_DELAY);
}

/**
 * Display product selection menu
 */
static void display_product_menu(MenuState *menu) {
    char line[17];
    
    if (menu->current_index == menu->last_rendered_index) return;  /* Already rendered, skip */
    
    clear_display();
    write_str_at(0, 0, "Select coffee:");
    render_slider_line(line, PRODUCT_TITLES[menu->current_index]);
    write_str_at(0, 1, line);
    
    menu->last_rendered_index = menu->current_index;
}

/**
 * Display payment method menu
 */
static void display_payment_menu(MenuState *menu) {
    char line[17];
    
    if (menu->current_index == menu->last_rendered_index) return;  /* Already rendered, skip */
    
    clear_display();
    write_str_at(0, 0, "Select payment:");
    render_slider_line(line, PAYMENT_TITLES[menu->current_index]);
    write_str_at(0, 1, line);
    
    menu->last_rendered_index = menu->current_index;
}

/**
 * Display cup detection prompt
 */
static void display_cup_detection_menu(void) {
    clear_display();
    write_str_at(0, 0, "Ready to brew!");
    write_str_at(0, 1, "Place cup");
    sleep_ms(LCD_UPDATE_DELAY);
}

static void display_removal_message(INT8U product_id) {
    clear_display();
    if (product_id == PRODUCT_ESPRESSO) {
        write_str_at(0, 0, "ESPRESSO READY!");
    } else if (product_id == PRODUCT_LATTE) {
        write_str_at(0, 0, "LATTE READY!");
    } else if (product_id == PRODUCT_FILTER) {
        write_str_at(0, 0, "FILTER READY!");
    }
    write_str_at(0, 1, "Remove cup");
    sleep_ms(LCD_UPDATE_DELAY);
}

/**
 * Display production status based on product and elapsed time
 */
static void display_production_status(INT8U product, INT32U elapsed_ms) {
    clear_display();

    if (product == PRODUCT_ESPRESSO) {
        if (elapsed_ms < LED_GRIND_TIME) {
            write_str_at(0, 0, "GRINDING...");
            write_str_at(0, 1, "7.5 seconds");
        } else if (elapsed_ms < (LED_GRIND_TIME + LED_BREW_TIME)) {
            write_str_at(0, 0, "BREWING...");
            write_str_at(0, 1, "14 seconds");
        } else {
            write_str_at(0, 0, "ESPRESSO READY");
            write_str_at(0, 1, "Remove cup");
        }
    } else if (product == PRODUCT_LATTE) {
        if (elapsed_ms < LED_GRIND_TIME) {
            write_str_at(0, 0, "GRINDING...");
            write_str_at(0, 1, "7.5 seconds");
        } else if (elapsed_ms < (LED_GRIND_TIME + LED_BREW_TIME)) {
            write_str_at(0, 0, "BREWING...");
            write_str_at(0, 1, "14 seconds");
        } else if (elapsed_ms < (LED_GRIND_TIME + LED_BREW_TIME + LED_FROTH_TIME)) {
            write_str_at(0, 0, "FROTHING...");
            write_str_at(0, 1, "6.2 seconds");
        } else {
            write_str_at(0, 0, "LATTE READY");
            write_str_at(0, 1, "Remove cup");
        }
    } else if (product == PRODUCT_FILTER) {
        write_str_at(0, 0, "FILTER COFFEE");
        write_str_at(0, 1, "Press for more");
    }
    sleep_ms(100);
}

void init_application_manager(void) {
    app.transaction_semaphore = xSemaphoreCreateBinary();
    configASSERT(app.transaction_semaphore != NULL);
    xSemaphoreGive(app.transaction_semaphore);
    
    reset_app_context();
}


/*****************************   State Handlers   *******************************/

/**
 * IDLE State: Waiting for user to start
 */
static void handle_state_idle(void) {
    set_input(INPUT_MODE, MODE_BUTTON);
    
    display_idle_menu();
    
    if (button_pressed(BUTTON_INPUT_SW1) || button_pressed(BUTTON_INPUT_SW2)) {
        /* Initialize for new transaction */
        app.transaction = get_current_transaction();
        app.transaction->current_product = INVALID_INPUT;
        
        app.system_state = STATE_SELECTION;
    }
}

/**
 * SELECTION State: User selects product via numpad
 */
static void handle_state_selection(void) {
    set_input(INPUT_MODE, MODE_NUMPAD);
    
    display_product_menu(&app.product_menu);
    
    INT8U numpad_input = read_menu_input(&app.product_menu);
    
    if (numpad_input == 10) {
        /* Button 10: Move left */
        menu_step_left(&app.product_menu.current_index, MAX_PRODUCTS);
    } else if (numpad_input == 11) {
        /* Button 11: Select product */
        app.selected_product = app.product_menu.current_index;
        start_transaction(app.selected_product);
        
        app.payment_menu.current_index = 0;
        app.payment_menu.input_latch = INVALID_INPUT;
        app.payment_menu.last_rendered_index = INVALID_INPUT;
        
        app.system_state = STATE_PAYMENT_METHOD;
    } else if (numpad_input == 12) {
        /* Button 12: Move right */
        menu_step_right(&app.product_menu.current_index, MAX_PRODUCTS);
    }
}

/**
 * PAYMENT_METHOD State: User selects payment method
 */
static void handle_state_payment_method(void) {
    set_input(INPUT_MODE, MODE_NUMPAD);
    
    display_payment_menu(&app.payment_menu);
    
    INT8U numpad_input = read_menu_input(&app.payment_menu);
    
    if (numpad_input == 10) {
        /* Button 10: Move left */
        menu_step_left(&app.payment_menu.current_index, 2);
    } else if (numpad_input == 11) {
        /* Button 11: Select payment method */
        app.payment_method = app.payment_menu.current_index;
        
        /* Reset payment state */
        app.payment_state.cash_amount = 0;
        app.payment_state.input_index = 0;
        app.payment_state.input_stage = 0;
        
        clear_display();
        if (app.payment_method == PAYMENT_METHOD_CASH) {
            write_str_at(0, 0, "Cash payment:");
            write_str_at(0, 1, "Turn encoder");
        } else if (app.payment_method == PAYMENT_METHOD_CARD) {
            write_str_at(0, 0, "Card payment:");
            write_str_at(0, 1, "Enter number");
        }
        
        app.system_state = STATE_PAYMENT_PROCESS;
        sleep_ms(TRANSITION_DELAY);
    } else if (numpad_input == 12) {
        /* Button 12: Move right */
        menu_step_right(&app.payment_menu.current_index, 2);
    }
}

/**
 * PAYMENT_PROCESS State: Handle cash input or card entry
 */
static void handle_state_payment_process(void) {
    char display_buffer[17];
    
    if (app.payment_method == PAYMENT_METHOD_CASH) {
        set_input(INPUT_MODE, MODE_ENCODER);
        
        INT8U encoder_value = get_queue(ENCODER_INPUT);
        if (encoder_value != INVALID_QUEUE_ID) {
            /* Encoder returns +5 or +20 (or negative equivalents) */
            app.payment_state.cash_amount += encoder_value;
            
            /* Update transaction with accumulated amount */
            app.transaction->amount_paid = app.payment_state.cash_amount;
        }
        
        /* Always display current accumulated amount */
        clear_display();
        write_str_at(0, 0, "Inserted cash:");
        format_display_line(display_buffer, "DKK: ", app.transaction->amount_paid);
        write_str_at(0, 1, display_buffer);
        format_display_line(display_buffer, "/  ", app.transaction->amount_required);
        write_str_at(9, 1, display_buffer);
        
        /* Check if payment is complete */
        if (is_payment_complete()) {
            /* Dispense change if an amount is owed */
            INT8U change_dkk = app.transaction->amount_paid - app.transaction->amount_required;
            if (change_dkk > 0) {
                clear_display();
                write_str_at(0, 0, "Dispensing!");
                format_display_line(display_buffer, "Change: ", change_dkk);
                write_str_at(0, 1, display_buffer);
                
                /* Flash green LED once per DKK coin */
                INT8U i;
                for (i=0; i<change_dkk; i++) {
                    set_led(GREEN);
                    sleep_ms(200);  /* LED on for 200ms */
                    clear_led();
                    sleep_ms(200);  /* LED off for 200ms */
                }
            }

            clear_display();
            write_str_at(0, 0, "Payment accepted!");
            write_str_at(0, 1, "Proceeding...");
            sleep_ms(TRANSITION_DELAY);
            display_cup_detection_menu();

            app.system_state = STATE_WAITING_CUP;
        }

    } else if (app.payment_method == PAYMENT_METHOD_CARD) {
        set_input(INPUT_MODE, MODE_NUMPAD);
        
        INT8U numpad_input = get_queue(NUMPAD_INPUT);
        if (numpad_input != INVALID_QUEUE_ID && numpad_input != 0) {
            if (app.payment_state.input_stage == 0) {
                if (app.payment_state.input_index < 16) {
                    /* Store the digit */
                    app.payment_state.card_input[app.payment_state.input_index] = numpad_input;
                    app.payment_state.input_index++;
                    
                    /* Display card entry progress */
                    clear_display();
                    write_str_at(0, 0, "Card number:");
                    format_display_line(display_buffer, "Typed: ", app.payment_state.input_index);
                    write_str_at(0, 1, display_buffer);
                    
                    /* If card is complete, move to PIN entry */
                    if (app.payment_state.input_index >= 16) {
                        app.payment_state.input_stage = 1;  /* Move to PIN stage */
                        app.payment_state.input_index = 0;
                        clear_display();
                        write_str_at(0, 0, "Enter PIN:");
                        sleep_ms(LCD_UPDATE_DELAY);
                    }
                }
            } else if (app.payment_state.input_stage == 1) {
                if (app.payment_state.input_index < 4) {
                    /* Store the PIN digit */
                    app.payment_state.pin_input[app.payment_state.input_index] = numpad_input;
                    app.payment_state.input_index++;
                    
                    /* Display PIN entry progress */
                    clear_display();
                    write_str_at(0, 0, "Enter PIN:");
                    format_display_line(display_buffer, "Typed: ", app.payment_state.input_index);
                    write_str_at(0, 1, display_buffer);
                    
                    /* If PIN is complete, validate */
                    if (app.payment_state.input_index >= 4) {

                        /* Validate card and PIN */
                        if (validate_card_payment(app.payment_state.card_input, app.payment_state.pin_input)) {
                            /* Payment valid */
                            app.transaction->amount_paid = app.transaction->amount_required;
                            
                            clear_display();
                            write_str_at(0, 0, "Card accepted!");
                            write_str_at(0, 1, "Proceeding...");
                            sleep_ms(TRANSITION_DELAY);
                            display_cup_detection_menu();
                            
                            app.system_state = STATE_WAITING_CUP;
                        } else {
                            /* Payment invalid - reject and restart */
                            clear_display();
                            write_str_at(0, 0, "Invalid card!");
                            write_str_at(0, 1, "Try again");
                            sleep_ms(TRANSITION_DELAY);
                            
                            /* Reset card input */
                            app.payment_state.input_stage = 0;
                            app.payment_state.input_index = 0;
                            clear_display();
                            write_str_at(0, 0, "Card number:");
                            write_str_at(0, 1, "Typed: 0");
                        }
                    }
                }
            }
        }
    }
    sleep_ms(LCD_UPDATE_DELAY);
}

/**
 * WAITING_CUP State: Wait for cup to be placed and start to be pressed
 */
static void handle_state_cup_detection(void) {
    set_input(INPUT_MODE, MODE_BUTTON);
    
    if (button_pressed(BUTTON_INPUT_SW1)) {
        app.interaction_state.cup_detected = true;
        clear_display();
        write_str_at(0, 0, "Cup detected!");
        write_str_at(0, 1, "Press START");

    } else if ((button_pressed(BUTTON_INPUT_SW2)) && (app.interaction_state.cup_detected)) {        
        /* Now start production */
        app.interaction_state.start_pressed = true;
        app.system_state = STATE_PRODUCTION;
        app.production_state.start_ticks = xTaskGetTickCount();
        app.production_state.filter_start_ticks = xTaskGetTickCount();
        app.production_state.filter_last_activity_ticks = xTaskGetTickCount();
        
        clear_display();
        write_str_at(0, 0, "Starting");
        write_str_at(0, 1, "production...");
        sleep_ms(TRANSITION_DELAY);
        
    } else if ((button_pressed(BUTTON_INPUT_SW2)) && (!app.interaction_state.cup_detected)) {
        /* Start pressed without cup */
        clear_display();
        write_str_at(0, 0, "No cup detected!");
        write_str_at(0, 1, "Place cup first");
    }
}

/**
 * PRODUCTION State: Handle product production with timing
 */
static void handle_state_production(void) {
    INT32U current_ticks = xTaskGetTickCount();
    INT32U elapsed_ms = (current_ticks - app.production_state.start_ticks) * portTICK_PERIOD_MS;
    
    INT8U product = app.selected_product;
    
    if (product == PRODUCT_ESPRESSO || product == PRODUCT_LATTE) {
        INT32U total_time = LED_GRIND_TIME + LED_BREW_TIME;
        if (product == PRODUCT_LATTE) {
            total_time += LED_FROTH_TIME;
        }
        
        /* Control LEDs based on production stage */
        if (elapsed_ms < LED_GRIND_TIME) {
            set_led(YELLOW);  /* Grinding */
        } else if (elapsed_ms < (LED_GRIND_TIME + LED_BREW_TIME)) {
            set_led(RED);     /* Brewing */
        } else if (elapsed_ms < total_time) {
            set_led(GREEN);   /* Frothing (Latte only) */
        } else {
            clear_led();       /* Done */
            app.system_state = STATE_COMPLETE;
        }
        display_production_status(product, elapsed_ms);
        
    } else if (product == PRODUCT_FILTER) {
        INT32U filter_elapsed_ms = (current_ticks - app.production_state.filter_start_ticks) * portTICK_PERIOD_MS;
        INT32U inactivity_ms = (current_ticks - app.production_state.filter_last_activity_ticks) * portTICK_PERIOD_MS;
        
        /* Check for continued dispensing via START button */
        if (button_pressed(BUTTON_INPUT_SW2)) {
            app.production_state.filter_last_activity_ticks = current_ticks;
        }
        
        /* Dispense based on timing */
        if (filter_elapsed_ms < FILTER_SLOW_RATE_MS && app.interaction_state.start_pressed && app.production_state.filter_dispensed_cl_x100 < app.transaction->amount_required * 100) {
            set_led(YELLOW);  /* Slow dispensing */
            app.production_state.filter_dispensed_cl_x100 = (INT16U)((FILTER_SLOW_CL / FILTER_SLOW_RATE_MS * 1000) * 100);
        } else if (inactivity_ms < FILTER_INACTIVITY_TIME && app.interaction_state.start_pressed && app.production_state.filter_dispensed_cl_x100 < app.transaction->amount_required * 100) {
            set_led(YELLOW);  /* Fast dispensing */
            FP32 fast_dispensed = FILTER_FAST_RATE_CL * (inactivity_ms / 1000.0);
            app.production_state.filter_dispensed_cl_x100 = (INT16U)(fast_dispensed * 100);
        } else {
            clear_led();       /* Done */
            app.system_state = STATE_COMPLETE;
        }
        display_production_status(product, filter_elapsed_ms);
    }
}

/**
 * COMPLETE State: Transaction complete, display message and log
 */
static void handle_state_complete(void) {
    set_input(INPUT_MODE, MODE_BUTTON);

    display_removal_message(app.selected_product);
        
    // Reset and return to idle when cup is removed (simulated by pressing SW1 again)
    if (button_pressed(BUTTON_INPUT_SW1)) {
        log_transaction_uart(app.selected_product, app.transaction->amount_required, app.transaction->amount_paid, app.payment_method);
        log_transaction(app.payment_method, app.transaction->amount_paid);
        reset_transaction();
        reset_app_context();
        app.system_state = STATE_IDLE;
    }
}


void application_task(void) {
    while (true) {
        switch (app.system_state) {
            case STATE_IDLE:
                handle_state_idle();
                break;
                
            case STATE_SELECTION:
                handle_state_selection();
                break;
                
            case STATE_PAYMENT_METHOD:
                handle_state_payment_method();
                break;
                
            case STATE_PAYMENT_PROCESS:
                handle_state_payment_process();
                break;
                
            case STATE_WAITING_CUP:
                handle_state_cup_detection();
                break;
                
            case STATE_PRODUCTION:
                handle_state_production();
                break;
                
            case STATE_COMPLETE:
                handle_state_complete();
                break;
                
            default:
                reset_app_context();
                break;
        }
    }
}

void uart_interface_task(void) {
    while (true) {
        /* UART command processing task */
        /* Command format:
         * '1' = SET CLOCK (followed by HH MM SS)
         * '2' = GET CLOCK
         * '3' = SET PRICE (followed by product_id and price)
         * '4' = GET REPORT
         */
        
        sleep_ms(100);
    }
}

/****************************** End Of Module *******************************/
