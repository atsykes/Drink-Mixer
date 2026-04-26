#include <avr/io.h>
#include <util/delay.h>


#include <stdio.h>
#include <stdbool.h>

#include "lcd.h" // Todo: Update for new microcontroller
// #include "adc.h" // Todo: Update for new microcontroller

// #include "queue.h" // Circular queue for orders

// #include "RFID/rfid.h"
#include "rfid.h"

int main(void) {
    uint8_t status;
    uint8_t tag_type[2];
    uint8_t uid[5];

    hardware_init();
    rc522_init();
    lcd_init();

    lcd_writecommand(1); // 0x0
    lcd_writecommand(2); //clear screen
    lcd_stringout("EE 459 RFID");
    
    uart_print("RC522 ready\r\n");

    while (1) {
        // lcd_writecommand(1); // 0x0
        // lcd_writecommand(2); //clear screen
        // lcd_stringout("EE 459 RFID");
        // uart_print("RC522 ready\r\n");
        status = rc522_request(PICC_REQIDL, tag_type);
        if (status == MI_OK) {
            uart_print("RC522 REQUESTED\r\n");
            status = rc522_anticoll(uid);
            if (status == MI_OK) {
                uart_print("RC522 RECEIVED\r\n");
                print_uid(uid, 4);
                int i;
                lcd_writecommand(1); // 0x0
                lcd_writecommand(2); //clear screen
                for (i=0; i<4; i++){
                    lcd_print_hex(uid[i]);
                }
                // char uid_char[17];
                // int i;
                // lcd_writecommand(1); // 0x0
                // lcd_writecommand(2); //clear screen
                // for (i=0; i<4; i++){
                //    if (i==0) snprintf(uid_char, 17, "%d", uid[i]);
                //    else snprintf(uid_char, 17, " %d", uid[i]);
                //    lcd_stringout(uid_char);
                // }

                // _delay_ms(500);
            }
        }
        _delay_ms(1000);
    }
}