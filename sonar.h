#ifndef SONAR_H
#define SONAR_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_SENSORS 2

typedef struct
{
    uint8_t echo_mask;
    uint8_t trigger_mask;
    volatile uint16_t start_time;
    volatile uint16_t pulse;
    volatile bool pulse_done;
    uint8_t prev_state;
} Sonar;

extern Sonar sensors[NUM_SENSORS];

void timer1_pcint_init(void);
void pollSensors(int i);

#endif