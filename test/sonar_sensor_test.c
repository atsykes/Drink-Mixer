#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdbool.h>

#include "../lcd_i2c.h"
#include "../sonar.h"

// ================= MAIN =================

int main(void)
{
    timer1_pcint_init();
    lcd_init();
    int cnt = 0;

    while (1)
    {
        if (cnt % 12 == 0)
        {
            cnt = 0;
            for (int i = 0; i < NUM_SENSORS; i++)
            {
                pollSensors(i);

                // Wait for pulse_done with timeout
                uint16_t start = TCNT1;
                while (!sensors[i].pulse_done && (uint16_t)(TCNT1 - start) < 870)
                    ;

                if (sensors[i].pulse_done)
                {
                    uint16_t p;
                    p = sensors[i].pulse;
                    sensors[i].pulse_done = false;

                    uint16_t distance = p / 58;
                    // display distance for sensor i
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%4d cm", distance);

                    lcd_moveto(i, 0);
                    lcd_stringout("        ");
                    lcd_moveto(i, 0);
                    lcd_stringout(buf);
                }
            }
        }
        ++cnt;
        _delay_ms(5);
    }
}
