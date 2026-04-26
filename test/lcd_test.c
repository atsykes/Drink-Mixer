#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "../lcd_i2c.h"
#include "../adc.h"

int main(void)
{

    // Initialize the LCD
    lcd_init();

    // Initialize the ADC
    adc_init();

    // Write splash screen and delay for 1 second
    lcd_writecommand(1);
    lcd_writecommand(2);
    char splash[12] = "EE109 Lab 5";
    char name[14] = "Andrew Sykes";
    lcd_stringout(splash);
    lcd_moveto(1, 0);
    lcd_stringout(name);
    _delay_ms(1000);

    lcd_writecommand(1);

    while (1)
    {
        // lcd_writecommand(1);
        lcd_writecommand(2);
        char buf[4];
        // Convert ADC channel for buttons to 0-255
        uint8_t adc_buttons = adc_sample(0);

        // lcd_stringout("Hello");

        snprintf(buf, 4, "%3d", adc_buttons);
        lcd_stringout(buf);

        _delay_ms(5);
    }

    return 0; /* never reached */
}