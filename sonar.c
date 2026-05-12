#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "sonar.h"

Sonar sensors[NUM_SENSORS] = {
    {(1 << PC2), (1 << PC3), 0, 0, false, 0},
    {(1 << PC4), (1 << PC5), 0, 0, false, 0},
    {(1 << PC6), (1 << PC7), 0, 0, false, 0},
    {(1 << PB0), (1 << PB1), 0, 0, false, 0}};

void timer1_pcint_init(void)
{
    TCCR1A = 0;
    TCCR1B = (1 << CS11);
    TCNT1 = 0;

    // PORTC sensors (0-2)
    for (int i = 0; i < NUM_SENSORS - 1; i++)
    {
        DDRC |= sensors[i].trigger_mask;
        DDRC &= ~sensors[i].echo_mask;
        PCMSK2 |= sensors[i].echo_mask;
        sensors[i].prev_state = (PINC & sensors[i].echo_mask);
    }

    // PORTB sensor (3)
    DDRB |= sensors[3].trigger_mask;
    DDRB &= ~sensors[3].echo_mask;
    PCMSK1 |= sensors[3].echo_mask;
    sensors[3].prev_state = (PINB & sensors[3].echo_mask);

    PCICR |= (1 << PCIE2) | (1 << PCIE1);
    sei();
}

void pollSensors(int i)
{
    if (i < NUM_SENSORS - 1)
        PORTC |= sensors[i].trigger_mask;
    else
        PORTB |= sensors[i].trigger_mask;

    _delay_us(50);

    if (i < NUM_SENSORS - 1)
        PORTC &= ~sensors[i].trigger_mask;
    else
        PORTB &= ~sensors[i].trigger_mask;
}

ISR(PCINT2_vect)
{
    for (int i = 0; i < NUM_SENSORS - 1; i++)
    {
        uint8_t current = PINC & sensors[i].echo_mask;

        if (current && !sensors[i].prev_state)
            sensors[i].start_time = TCNT1;
        else if (!current && sensors[i].prev_state)
        {
            sensors[i].pulse = TCNT1 - sensors[i].start_time;
            sensors[i].pulse_done = true;
        }

        sensors[i].prev_state = current;
    }
}

ISR(PCINT1_vect)
{
    uint8_t current = PINB & sensors[3].echo_mask;

    if (current && !sensors[3].prev_state)
        sensors[3].start_time = TCNT1;
    else if (!current && sensors[3].prev_state)
    {
        sensors[3].pulse = TCNT1 - sensors[3].start_time;
        sensors[3].pulse_done = true;
    }

    sensors[3].prev_state = current;
}