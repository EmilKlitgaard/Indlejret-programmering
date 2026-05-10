/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: UARTLogger.c
*
* PROJECT....: Coffee Vending Machine
*
* DESCRIPTION: UART logging for transactions and PC interface
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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "UARTLogger.h"
#include "Print.h"
#include "tm4c123gh6pm.h"

/*****************************   Variables   *******************************/
static INT8U uart_rx_buffer[64];
static INT8U uart_rx_index = 0;
static BOOLEAN command_ready = false;

/*****************************   Functions   *******************************/
void init_uart_logger(void) {
    uart_rx_index = 0;
    command_ready = false;
    memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
}

static BOOLEAN uart_data_available(void) {
    return (UART0_FR_R & UART_FR_RXFE) == 0;
}

static INT8U uart_read_char(void) {
    while ((UART0_FR_R & UART_FR_RXFE) != 0) {
        // Wait for data
    }
    return (INT8U)(UART0_DR_R & 0xFF);
}

static void uart_write_char(INT8U ch) {
    while ((UART0_FR_R & UART_FR_TXFF) != 0) {
        // Wait for transmit buffer to be ready
    }
    UART0_DR_R = ch;
}

static void uart_write_string(const char *str) {
    while (*str) {
        uart_write_char((INT8U)*str);
        str++;
    }
}

void log_transaction_uart(INT8U product_id, INT16U price, INT16U amount_paid,
                          INT8U payment_type) {
    TimeOfDay time = get_time();
    char log_msg[64];
    const char *product_name = "UNKNOWN";
    const char *payment_name = "UNKNOWN";
    INT8U quantity = 1;
    
    if (product_id == PRODUCT_ESPRESSO) {
        product_name = "ESPRESSO";
    } else if (product_id == PRODUCT_LATTE) {
        product_name = "LATTE";
    } else if (product_id == PRODUCT_FILTER) {
        product_name = "FILTER";
    }
    
    if (payment_type == PAYMENT_METHOD_CASH) {
        payment_name = "CASH";
    } else if (payment_type == PAYMENT_METHOD_CARD) {
        payment_name = "CARD";
    }
    
    snprintf(log_msg, sizeof(log_msg),
             "%04d-%02d-%02d %02d:%02d:%02d,%s,%d,%d,%s\n",
             time.year, time.month,  time.day,
             time.hour, time.minute, time.second,
             product_name, price, quantity, payment_name);
    
    uart_write_string(log_msg);
}

void handle_uart_command(void) {
    INT8U ch;
    INT8U cmd;
    INT8U hour, minute, second;
    INT8U product_id;
    INT16U price;
    MachineReport *report;
    TimeOfDay time;
    char response[64];
    INT8U idx;
    
    /* Read available characters into buffer until newline */
    while (uart_data_available() && uart_rx_index < 63) {
        ch = uart_read_char();
        
        if (ch == '\n' || ch == '\r') {
            /* End of command - mark as ready */
            uart_rx_buffer[uart_rx_index] = '\0';
            command_ready = true;
            uart_rx_index = 0;
            break;
        } else if (ch >= 32 && ch < 127) {
            /* Printable character - add to buffer */
            uart_rx_buffer[uart_rx_index++] = ch;
        }
    }
    
    /* Process command if complete */
    if (!command_ready) {
        return;
    }
    
    command_ready = false;
    
    if (uart_rx_index == 0) {
        return;  /* Empty command */
    }
    
    cmd = uart_rx_buffer[0];
    
    switch (cmd) {
        case '1':
            /* SET CLOCK: format "1 HH MM SS" (space-separated) */
            idx = 1;
            
            /* Parse hour */
            if (uart_rx_index > idx + 1) {
                hour = (uart_rx_buffer[idx] - '0') * 10;
                hour += (uart_rx_buffer[idx + 1] - '0');
                idx += 2;
                
                /* Skip space */
                if (idx < uart_rx_index && uart_rx_buffer[idx] == ' ') idx++;
                
                /* Parse minute */
                if (uart_rx_index > idx + 1) {
                    minute = (uart_rx_buffer[idx] - '0') * 10;
                    minute += (uart_rx_buffer[idx + 1] - '0');
                    idx += 2;
                    
                    /* Skip space */
                    if (idx < uart_rx_index && uart_rx_buffer[idx] == ' ') idx++;
                    
                    /* Parse second */
                    if (uart_rx_index > idx + 1) {
                        second = (uart_rx_buffer[idx] - '0') * 10;
                        second += (uart_rx_buffer[idx + 1] - '0');
                        
                        set_time(hour, minute, second);
                        uart_write_string("OK\n");
                    } else {
                        uart_write_string("ERR\n");
                    }
                } else {
                    uart_write_string("ERR\n");
                }
            } else {
                uart_write_string("ERR\n");
            }
            break;
            
        case '2':
            /* GET CLOCK: return "2 HH MM SS" */
            time = get_time();
            snprintf(response, sizeof(response),
                     "2 %02d %02d %02d\n",
                     time.hour, time.minute, time.second);
            uart_write_string(response);
            break;
            
        case '3':
            /* SET PRICE: format "3 PRODUCT_ID PRICE" (space-separated) */
            idx = 1;
            
            /* Skip space */
            if (idx < uart_rx_index && uart_rx_buffer[idx] == ' ') idx++;
            
            /* Parse product ID (single digit) */
            if (idx < uart_rx_index && uart_rx_buffer[idx] >= '0' && uart_rx_buffer[idx] <= '9') {
                product_id = uart_rx_buffer[idx] - '0';
                idx++;
                
                /* Skip space */
                if (idx < uart_rx_index && uart_rx_buffer[idx] == ' ') idx++;
                
                /* Parse price (variable digits) */
                price = 0;
                while (idx < uart_rx_index && uart_rx_buffer[idx] >= '0' && uart_rx_buffer[idx] <= '9') {
                    price = price * 10 + (uart_rx_buffer[idx] - '0');
                    idx++;
                }
                
                if (price > 0 && product_id < MAX_PRODUCTS) {
                    set_product_price(product_id, price);
                    uart_write_string("OK\n");
                } else {
                    uart_write_string("ERR\n");
                }
            } else {
                uart_write_string("ERR\n");
            }
            break;
            
        case '4':
            /* GET REPORT */
            report = get_machine_report();
            snprintf(response, sizeof(response),
                     "4 ESP:%d LAT:%d FLT:%d CASH:%d CARD:%d\n",
                     (int)report->total_sales_espresso,
                     (int)report->total_sales_latte,
                     (int)report->total_sales_filter_cl,
                     (int)report->total_cash_sales,
                     (int)report->total_card_sales);
            uart_write_string(response);
            break;
            
        default:
            uart_write_string("ERR\n");
            break;
    }
}



void uart_logger_task(void) {
    while(true) {
        handle_uart_command();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/****************************** End Of Module *******************************/
