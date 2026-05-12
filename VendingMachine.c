/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: VendingMachine.c
*
* PROJECT....: Coffee Vending Machine
*
* DESCRIPTION: Coffee vending machine business logic
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
#include "VendingMachine.h"
#include "RealTimeClock.h"

/*****************************   Constants   *******************************/
const Product products[MAX_PRODUCTS] = {
    {"Espresso", 15, PRODUCT_ESPRESSO},
    {"Latte", 27, PRODUCT_LATTE},
    {"Filter", 3, PRODUCT_FILTER},
};

/*****************************   Variables   *******************************/
static VendingTransaction current_transaction = {0};
static MachineReport machine_report = {0};
static INT16U product_prices[MAX_PRODUCTS] = {15, 27, 3};

/*****************************   Functions   *******************************/
void init_vending_machine(void) {
    current_transaction.current_product = 0xFF;
    current_transaction.amount_paid = 0;
    current_transaction.amount_required = 0;
    current_transaction.payment_method = 0;
    current_transaction.change_dkk = 0;
    
    machine_report.total_sales_espresso = 0;
    machine_report.total_sales_latte = 0;
    machine_report.total_sales_filter_cl = 0;
    machine_report.total_cash_sales = 0;
    machine_report.total_card_sales = 0;
    machine_report.operating_time_seconds = 0;
    machine_report.transaction_count = 0;
}

INT16U get_product_price(INT8U product_id) {
    if (product_id < MAX_PRODUCTS) {
        return product_prices[product_id];
    }
    return 0;
}

void set_product_price(INT8U product_id, INT16U price) {
    if (product_id < MAX_PRODUCTS) {
        product_prices[product_id] = price;
    }
}

/**
 * Check if both last card_number and last PIN have the same parity (all odd or all even)
 * Returns true if valid, false otherwise
 */
BOOLEAN validate_card_payment(INT8U *card_number, INT8U *pin) {
    INT8U i;
    BOOLEAN card_parity_even = false;
    BOOLEAN pin_parity_even = false;
    
    // Check card number parity (last digit determines)
    if ((card_number[15] % 2) == 0) {
        card_parity_even = true;
    }

    // Check PIN parity (last digit determines)
    if ((pin[3] % 2) == 0) {
        pin_parity_even = true;
    }
    
    // Both must have same parity (both odd or both even)
    return (card_parity_even == pin_parity_even);
}

void start_transaction(INT8U product_id) {
    current_transaction.current_product = product_id;
    current_transaction.amount_required = get_product_price(product_id);
    current_transaction.amount_paid = 0;
    current_transaction.change_dkk = 0;
}

void add_cash_payment(INT16U amount) {
    if (current_transaction.current_product != 0xFF) {
        current_transaction.amount_paid += amount;
        if (current_transaction.amount_paid > current_transaction.amount_required) {
            current_transaction.change_dkk = current_transaction.amount_paid - current_transaction.amount_required;
        }
    }
}

BOOLEAN is_payment_complete(void) {
    if (current_transaction.current_product == 0xFF) {
        return false;
    }
    return (current_transaction.amount_paid >= current_transaction.amount_required);
}

void log_transaction(INT8U payment_type, INT16U amount_paid) {
    if (current_transaction.current_product == PRODUCT_ESPRESSO) {
        machine_report.total_sales_espresso += current_transaction.amount_required;
    } else if (current_transaction.current_product == PRODUCT_LATTE) {
        machine_report.total_sales_latte += current_transaction.amount_required;
    } else if (current_transaction.current_product == PRODUCT_FILTER) {
        /* Filter sales tracking - dispensed amount handled in ApplicationManager */
        machine_report.total_sales_filter_cl += current_transaction.amount_required;
    }
    
    if (payment_type == PAYMENT_METHOD_CASH) {
        machine_report.total_cash_sales += current_transaction.amount_required;
    } else {
        machine_report.total_card_sales += current_transaction.amount_required;
    }
    
    machine_report.transaction_count++;
}

VendingTransaction* get_current_transaction(void) {
    return &current_transaction;
}

MachineReport* get_machine_report(void) {
    machine_report.operating_time_seconds = get_uptime_seconds();
    return &machine_report;
}

void reset_transaction(void) {
    current_transaction.current_product = 0xFF;
    current_transaction.amount_paid = 0;
    current_transaction.amount_required = 0;
    current_transaction.change_dkk = 0;
}

/****************************** End Of Module *******************************/
