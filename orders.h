#ifndef ORDER_H
#define ORDER_H

#include <stdbool.h>
#include "drink.h"

typedef struct
{
    uint8_t *uid;
    Drink *drink;
} Order;

const uint8_t *getID(const Order *o);
const Drink *getDrink(const Order *o);

#endif