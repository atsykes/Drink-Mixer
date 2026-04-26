#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd_i2c.h"

#define LCD_ADDR 0x27 // change to 0x3F if needed

// PCF8574 bit mapping
#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_EN 0x04
#define LCD_BL 0x08

// ---------------- I2C ----------------

void i2c_init(void)
{
    TWSR = 0x00;
    TWBR = 32; // ~100kHz @ 8MHz
}

void i2c_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
        ;
}

void i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
        ;
}

void i2c_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

// ---------------- LCD LOW LEVEL ----------------

void lcd_write_i2c(uint8_t data)
{
    i2c_start();
    i2c_write(LCD_ADDR << 1);
    i2c_write(data | LCD_BL);
    i2c_stop();
}

void lcd_pulse_enable(uint8_t data)
{
    lcd_write_i2c(data | LCD_EN);
    _delay_us(1);
    lcd_write_i2c(data & ~LCD_EN);
    _delay_us(50);
}

void lcd_writenibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble & 0xF0);
    if (rs)
        data |= LCD_RS;

    lcd_write_i2c(data);
    lcd_pulse_enable(data);
}

// ---------------- LCD API ----------------

void lcd_writecommand(unsigned char cmd)
{
    lcd_writenibble(cmd, 0);
    lcd_writenibble(cmd << 4, 0);
    _delay_ms(2);
}

void lcd_writedata(unsigned char data)
{
    lcd_writenibble(data, 1);
    lcd_writenibble(data << 4, 1);
    _delay_ms(2);
}

void lcd_init(void)
{
    i2c_init();
    _delay_ms(50);

    lcd_writenibble(0x30, 0);
    _delay_ms(5);

    lcd_writenibble(0x30, 0);
    _delay_us(120);

    lcd_writenibble(0x30, 0);
    lcd_writenibble(0x20, 0);

    lcd_writecommand(0x28); // 4-bit, 2-line
    lcd_writecommand(0x0C); // display on
    lcd_writecommand(0x01); // clear
    _delay_ms(2);
}

void lcd_moveto(unsigned char row, unsigned char col)
{
    unsigned char pos = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_writecommand(pos);
}

void lcd_stringout(char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        lcd_writedata(str[i]);
        i++;
    }
}