#include <avr/io.h>

#include "adc.h"

#define MASKBITS 0x0f

void adc_init(void)
{
    // Step 1
    ADMUX &= ~(1 << REFS1);
    ADMUX |= (1 << REFS0);
    
    // Step 2
    ADCSRA |= ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));

    // Step 3
    ADMUX |= (1 << ADLAR);

    // Step 4
    ADCSRA |= (1 << ADEN);
}

uint8_t adc_sample(uint8_t channel)
{
    // Set ADC input mux bits to 'channel' value
    // ADMUX |= ((1 << MUX0) | (1 << MUX1));
    // ADMUX &= ~((1 << MUX2) | (1 << MUX3));

    // Convert an analog input and return the 8-bit result
    ADMUX &= ~MASKBITS;
    ADMUX |= (MASKBITS & channel);
    ADCSRA |= (1 << ADSC);

    while ((ADCSRA & (1 << ADSC)) != 0) {}
    unsigned char result = ADCH;
    return result;
}
