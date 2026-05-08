/****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: Display.c
*
* PROJECT....:
*
* DESCRIPTION: HD44780U LCD display driver (4-bit mode)
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260422  Copilot  Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include "Display.h"

/*****************************    Defines    *******************************/
#define LCD_DATA_MASK       (LCD_D4_PIN | LCD_D5_PIN | LCD_D6_PIN | LCD_D7_PIN)
#define LCD_CONTROL_MASK    (LCD_RS_PIN | LCD_E_PIN)

/*****************************   Functions   *******************************/
static void lcd_wait_short(void) {
    volatile INT16U i;
    for (i = 0; i < 1000; i++) {
    }
}


static void lcd_wait_ms(INT32U ms) {
    volatile INT16U i;
    while (ms > 0) {
        for (i=0; i<5000; i++) {}
        ms--;
    }
}


static void wr_ctrl_LCD_low(INT8U ch) {
    INT8U temp;

    temp = GPIO_PORTC_DATA_R & 0x0F;
    temp = temp | ((ch & 0x0F) << 4);
    GPIO_PORTC_DATA_R = temp;

    lcd_wait_short();
    GPIO_PORTD_DATA_R &= (INT8U)~LCD_RS_PIN;   // Control mode
    lcd_wait_short();
    GPIO_PORTD_DATA_R |= LCD_E_PIN;            // E high
    lcd_wait_short();
    GPIO_PORTD_DATA_R &= (INT8U)~LCD_E_PIN;    // E low
    lcd_wait_short();
}


static void wr_ctrl_LCD_high(INT8U ch) {
    wr_ctrl_LCD_low((ch & 0xF0) >> 4);
}


static void out_LCD_low(INT8U ch) {
    INT8U temp;

    temp = GPIO_PORTC_DATA_R & 0x0F;
    GPIO_PORTC_DATA_R = temp | ((ch & 0x0F) << 4);

    GPIO_PORTD_DATA_R |= LCD_RS_PIN;           // Data mode
    GPIO_PORTD_DATA_R |= LCD_E_PIN;            // E high
    GPIO_PORTD_DATA_R &= (INT8U)~LCD_E_PIN;    // E low
}


static void out_LCD_high(INT8U ch) {
    out_LCD_low((ch & 0xF0) >> 4);
}


static void wr_ctrl_LCD(INT8U ch) {
    static INT8U mode_4bit = 0;
    INT16U i;

    wr_ctrl_LCD_high(ch);
    if (mode_4bit) {
        for (i = 0; i < 1000; i++) {
        }
        wr_ctrl_LCD_low(ch);
    } else {
        if ((ch & 0x30) == 0x20) {
            mode_4bit = 1;
        }
    }
}


static void out_LCD(INT8U ch) {
    INT16U i;

    out_LCD_high(ch);
    for (i = 0; i < 1000; i++) {
    }
    out_LCD_low(ch);
}


void display_command(INT8U cmd) {
    wr_ctrl_LCD(cmd);

    if ((cmd == 0x01) || (cmd == 0x02)) {
        lcd_wait_ms(2);
    } else {
        lcd_wait_short();
    }
}


void clear_display(void) {
    wr_ctrl_LCD(0x01);
}


void write_char(char ch) {
    out_LCD((INT8U)ch);
    lcd_wait_short();
}


void write_str(const char *str) {
    while ((str != 0) && (*str != '\0')) {
        write_char(*str);
        str++;
    }
}


void set_cursor(INT8U x, INT8U y) {
    INT8U pos;

    if (x > 15) {
        x = 15;
    }
    if (y > 1) {
        y = 1;
    }

    pos = y * 0x40 + x;
    pos |= 0x80;
    wr_ctrl_LCD(pos);
}


void write_str_at(INT8U x, INT8U y, const char *str) {
    set_cursor(x, y);
    write_str(str);
}


void init_display(void) {
    // Enable clock for Port C (data) and Port D (control)
    SYSCTL_RCGC2_R |= (SYSCTL_RCGC2_GPIOC | SYSCTL_RCGC2_GPIOD);

    // Wait until both peripherals are ready
    while ((SYSCTL_PRGPIO_R & (SYSCTL_PRGPIO_R2 | SYSCTL_PRGPIO_R3)) != (SYSCTL_PRGPIO_R2 | SYSCTL_PRGPIO_R3)) {}

    // Unlock and commit LCD pins
    GPIO_PORTC_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTC_CR_R |= LCD_DATA_MASK;
    GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTD_CR_R |= LCD_CONTROL_MASK;

    // Configure Port C upper nibble for LCD data
    GPIO_PORTC_AFSEL_R &= ~LCD_DATA_MASK;
    GPIO_PORTC_PCTL_R &= ~0xFFFF0000;
    GPIO_PORTC_AMSEL_R &= ~LCD_DATA_MASK;
    GPIO_PORTC_DIR_R |= LCD_DATA_MASK;
    GPIO_PORTC_DEN_R |= LCD_DATA_MASK;

    // Configure Port D control pins for RS and E
    GPIO_PORTD_AFSEL_R &= ~LCD_CONTROL_MASK;
    GPIO_PORTD_PCTL_R &= ~0x0000FF00;
    GPIO_PORTD_AMSEL_R &= ~LCD_CONTROL_MASK;
    GPIO_PORTD_DIR_R |= LCD_CONTROL_MASK;
    GPIO_PORTD_DEN_R |= LCD_CONTROL_MASK;

    // Known idle output state
    GPIO_PORTC_DATA_R &= ~LCD_DATA_MASK;
    GPIO_PORTD_DATA_R &= ~LCD_CONTROL_MASK;

    // Init sequence
    lcd_wait_ms(40);
    wr_ctrl_LCD(0x30);
    lcd_wait_ms(5);
    wr_ctrl_LCD(0x30);
    lcd_wait_short();
    wr_ctrl_LCD(0x30);
    lcd_wait_short();
    wr_ctrl_LCD(0x20);   // Set 4-bit mode
    lcd_wait_short();
    wr_ctrl_LCD(0x28);   // 2 line display
    wr_ctrl_LCD(0x0C);   // Display on, cursor off
    wr_ctrl_LCD(0x06);   // Cursor increment
    wr_ctrl_LCD(0x01);   // Clear
    wr_ctrl_LCD(0x02);   // Home
}
