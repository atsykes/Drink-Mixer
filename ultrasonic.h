#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

void ultrasonic_init(void);
uint16_t ultrasonic_read_cm(void);
uint8_t ultrasonic_object_within_2cm(void);

#endif