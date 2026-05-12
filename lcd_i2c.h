// Source: Adapted from USC EE109 - Intro to Embedded Systems

#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void lcd_init(void);

void lcd_writecommand(unsigned char cmd);
void lcd_writedata(unsigned char data);

void lcd_moveto(unsigned char row, unsigned char col);
void lcd_stringout(char *str);
void lcd_print_hex(uint8_t v);

#endif