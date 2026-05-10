/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: VendingMachine.h
*
* PROJECT....: Coffee Vending Machine
*
* DESCRIPTION: Coffee vending machine types and constants
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
#include <time.h>
#include "data_type.h"

/*****************************    Defines    *******************************/
// Product IDs
#define PRODUCT_ESPRESSO        0
#define PRODUCT_LATTE           1
#define PRODUCT_FILTER          2
#define MAX_PRODUCTS            3

// Payment methods
#define PAYMENT_METHOD_CASH     0
#define PAYMENT_METHOD_CARD     1

// System states
#define STATE_IDLE              0
#define STATE_INSTRUCTION       1
#define STATE_SELECTION         2
#define STATE_PAYMENT_METHOD    3
#define STATE_PAYMENT_PROCESS   4
#define STATE_WAITING_CUP       5
#define STATE_PRODUCTION        6
#define STATE_COMPLETE          7

// Production stage
#define PROD_IDLE               0
#define PROD_GRIND              1
#define PROD_BREW               2
#define PROD_FROTH              3
#define PROD_FILTER_SLOW        4
#define PROD_FILTER_FAST        5
#define PROD_COMPLETE           6

// LED indication timings (in milliseconds)
#define LED_GRIND_TIME          7500    // 7.5 seconds
#define LED_BREW_TIME           14000   // 14 seconds
#define LED_FROTH_TIME          6200    // 6.2 seconds
#define LED_CHANGE_FLASH        200     // 200ms per DKK coin
#define FILTER_SLOW_RATE_MS     3000    // 3 seconds at slow rate
#define FILTER_SLOW_CL          0.6     // cl per second at slow rate
#define FILTER_FAST_RATE_CL     1.45    // cl per second at fast rate
#define FILTER_INACTIVITY_TIME  5000    // 5 seconds inactivity timeout

/*****************************   Types   *******************************/

typedef struct {
    const char *name;
    INT16U price_dkk;
    INT8U product_id;
} Product;

typedef struct {
    INT8U card_number[16];
    INT8U pin[4];
    BOOLEAN valid;
} CardData;

typedef struct {
    INT8U current_product;
    INT16U amount_paid;
    INT16U amount_required;
    INT8U payment_method;
    INT8U change_dkk;
    INT32U timestamp;
} VendingTransaction;

typedef struct {
    INT32U total_sales_espresso;
    INT32U total_sales_latte;
    INT32U total_sales_filter_cl;
    INT32U total_cash_sales;
    INT32U total_card_sales;
    INT32U operating_time_seconds;
    INT16U transaction_count;
} MachineReport;

/*****************************   Constants   *******************************/
extern const Product products[MAX_PRODUCTS];

/*****************************   Functions   *******************************/
extern void init_vending_machine(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Initialize vending machine system
******************************************************************************/

extern INT16U get_product_price(INT8U product_id);
/****************************************************************************
*   Input    : Product ID
*   Output   : Price in DKK
*   Function : Get price for specified product
******************************************************************************/

extern void set_product_price(INT8U product_id, INT16U price);
/****************************************************************************
*   Input    : Product ID, Price in DKK
*   Output   : -
*   Function : Set price for specified product
******************************************************************************/

extern BOOLEAN validate_card_payment(INT8U *card_number, INT8U *pin);
/****************************************************************************
*   Input    : Card number (16 digits), PIN (4 digits)
*   Output   : True if valid, false otherwise
*   Function : Validate card payment (both odd or both even)
******************************************************************************/

extern void start_transaction(INT8U product_id);
/****************************************************************************
*   Input    : Product ID
*   Output   : -
*   Function : Start new transaction for selected product
******************************************************************************/

extern void add_cash_payment(INT16U amount);
/****************************************************************************
*   Input    : Amount in DKK
*   Output   : -
*   Function : Add cash payment to current transaction
******************************************************************************/

extern BOOLEAN is_payment_complete(void);
/****************************************************************************
*   Input    : -
*   Output   : True if payment complete, false otherwise
*   Function : Check if payment is sufficient
******************************************************************************/

extern void log_transaction(INT8U payment_type, INT16U amount_paid);
/****************************************************************************
*   Input    : Payment type (CASH/CARD), Amount paid
*   Output   : -
*   Function : Log transaction to machine report
******************************************************************************/

extern VendingTransaction* get_current_transaction(void);
/****************************************************************************
*   Input    : -
*   Output   : Pointer to current transaction
*   Function : Get current transaction data
******************************************************************************/

extern MachineReport* get_machine_report(void);
/****************************************************************************
*   Input    : -
*   Output   : Pointer to machine report
*   Function : Get machine report data
******************************************************************************/

extern void reset_transaction(void);
/****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Reset current transaction
******************************************************************************/

/****************************** End Of Module *******************************/
