/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: UARTLogger.h
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

#pragma once

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdbool.h>
#include "data_type.h"
#include "VendingMachine.h"
#include "RealTimeClock.h"

/*****************************    Defines    *******************************/

/*****************************   Functions   *******************************/
void init_uart_logger(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Initialize UART logging
******************************************************************************/

void send_clock_response(char *response, TimeOfDay time);
/****************************************************************************
*   Input    : Response buffer, TimeOfDay struct
*   Output   : -
*   Function : Format and send clock response over UART
******************************************************************************/

void log_transaction_uart(INT8U product_id, INT16U price, INT16U amount_paid,
                          INT8U payment_type);
/****************************************************************************
*   Input    : Product ID, Price, Amount Paid, Payment Type (CASH/CARD)
*   Output   : -
*   Function : Log transaction to UART
*   Format   : HH:MM:SS,PRODUCT,PRICE,QUANTITY,PAYMENT\n
******************************************************************************/

void handle_uart_command(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Process incoming UART commands from PC
*   Commands :
*     '1' + HH MM SS = SET CLOCK
*     '2'             = GET CLOCK (returns '2' + HH MM SS)
*     '3' + ID PRICE  = SET PRICE
*     '4'             = GET REPORT
******************************************************************************/

void uart_logger_task(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : FreeRTOS task for UART communication
******************************************************************************/

/****************************** End Of Module *******************************/
