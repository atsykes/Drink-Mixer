#ifndef ORDER_H
#define ORDER_H

#include <stdbool.h>
#include "drink.h"

typedef struct
{
    uint8_t uid[5];
    const Drink *drink;
} Order;

const uint8_t *getID(const Order *o);
const Drink *getDrink(const Order *o);
void updateOrder(Order *order, const uint8_t *uid, const Drink *drink);

#endif