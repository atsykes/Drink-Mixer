#include "ultrasonic.h"
#include <avr/io.h>
#include <util/delay.h>

/*
 * HC-SR04 wiring:
 * TRIG -> PD6 (Arduino D6)
 * ECHO -> PD7 (Arduino D7)
 *
 * Change these two pins if you want different ones.
 */
#define US_TRIG_DDR   DDRD
#define US_TRIG_PORT  PORTD
#define US_TRIG_PIN   PD6

#define US_ECHO_DDR   DDRD
#define US_ECHO_PORT  PORTD
#define US_ECHO_PINR  PIND
#define US_ECHO_PIN   PD7

#define ULTRASONIC_TIMEOUT_US 30000U

static void ultrasonic_trigger_pulse(void)
{
    US_TRIG_PORT &= ~(1 << US_TRIG_PIN);
    _delay_us(2);

    US_TRIG_PORT |= (1 << US_TRIG_PIN);
    _delay_us(10);

    US_TRIG_PORT &= ~(1 << US_TRIG_PIN);
}

void ultrasonic_init(void)
{
    US_TRIG_DDR |= (1 << US_TRIG_PIN);
    US_ECHO_DDR &= ~(1 << US_ECHO_PIN);

    US_TRIG_PORT &= ~(1 << US_TRIG_PIN);
    US_ECHO_PORT &= ~(1 << US_ECHO_PIN); /* no pull-up needed for HC-SR04 */
}

uint16_t ultrasonic_read_cm(void)
{
    uint16_t timeout;

    ultrasonic_trigger_pulse();

    /* Wait for ECHO to go high */
    timeout = ULTRASONIC_TIMEOUT_US;
    while (!(US_ECHO_PINR & (1 << US_ECHO_PIN))) {
        _delay_us(1);
        if (timeout == 0) {
            return 0xFFFF;
        }
        timeout--;
    }

    /* Measure how long ECHO stays high */
    {
        uint16_t pulse_us = 0;
        timeout = ULTRASONIC_TIMEOUT_US;

        while (US_ECHO_PINR & (1 << US_ECHO_PIN)) {
            _delay_us(1);
            pulse_us++;
            if (timeout == 0) {
                break;
            }
            timeout--;
        }

        /* HC-SR04: distance in cm ~= pulse_us / 58 */
        return (uint16_t)((pulse_us + 29) / 58);
    }
}

uint8_t ultrasonic_object_within_2cm(void)
{
    uint16_t cm = ultrasonic_read_cm();

    if (cm == 0xFFFF) {
        return 0;
    }

    return (cm > 0 && cm <= 2) ? 1 : 0;
}