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
#include "UARTLogger.h"
#include "Print.h"
#include "tm4c123gh6pm.h"

/*****************************   Variables   *******************************/
static INT8U uart_rx_buffer[64];
static INT8U uart_rx_index = 0;

/*****************************   Functions   *******************************/
void init_uart_logger(void) {
    uart_rx_index = 0;
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
             "%02d:%02d:%02d,%s,%d,%d,%s\n",
             time.hour, time.minute, time.second,
             product_name, price, quantity, payment_name);
    
    uart_write_string(log_msg);
}

void handle_uart_command(void) {
    INT8U cmd;
    INT8U hour, minute, second;
    INT8U product_id;
    INT16U price;
    MachineReport *report;
    TimeOfDay time;
    char response[64];
    
    if (!uart_data_available()) {
        return;
    }
    
    cmd = uart_read_char();
    
    switch (cmd) {
        case '1':
            // SET CLOCK: format '1' HH MM SS
            if (uart_data_available()) {
                hour = uart_read_char() - '0';
                hour = hour * 10 + (uart_read_char() - '0');
            }
            if (uart_data_available()) {
                minute = uart_read_char() - '0';
                minute = minute * 10 + (uart_read_char() - '0');
            }
            if (uart_data_available()) {
                second = uart_read_char() - '0';
                second = second * 10 + (uart_read_char() - '0');
            }
            set_time(hour, minute, second);
            break;
            
        case '2':
            // GET CLOCK: return '2' HH MM SS
            time = get_time();
            snprintf(response, sizeof(response),
                     "2%02d%02d%02d",
                     time.hour, time.minute, time.second);
            uart_write_string(response);
            break;
            
        case '3':
            // SET PRICE: format '3' PRODUCT_ID PRICE
            if (uart_data_available()) {
                product_id = uart_read_char() - '0';
            }
            if (uart_data_available()) {
                price = uart_read_char() - '0';
                price = price * 10 + (uart_read_char() - '0');
            }
            set_product_price(product_id, price);
            break;
            
        case '4':
            // GET REPORT
            report = get_machine_report();
            snprintf(response, sizeof(response),
                     "ESPRESSO:%d,LATTE:%d,FILTER:%d,CASH:%d,CARD:%d,TIME:%d\n",
                     (int)report->total_sales_espresso,
                     (int)report->total_sales_latte,
                     (int)report->total_sales_filter_cl,
                     (int)report->total_cash_sales,
                     (int)report->total_card_sales,
                     (int)get_uptime_seconds());
            uart_write_string(response);
            break;
            
        default:
            break;
    }
}

void uart_logger_task(void) {
    for (;;) {
        handle_uart_command();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/****************************** End Of Module *******************************/
