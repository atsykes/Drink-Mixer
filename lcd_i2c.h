#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/* ---------------- LCD API ---------------- */

void lcd_init(void);

void lcd_writecommand(unsigned char cmd);
void lcd_writedata(unsigned char data);

void lcd_moveto(unsigned char row, unsigned char col);
void lcd_stringout(char *str);

/* ---------------- Optional (if you want later) ---------------- */

// Clear display
void lcd_clear(void);

// Return cursor home
void lcd_home(void);

#endif