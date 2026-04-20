#include <stdint.h>
#include <stdbool.h>
#include "drink.h"
#include "orders.h"

// Information for Each Drink
struct Order
{
    uint8_t uid[5];
    Drink *drink;
};

const uint8_t *getID(const Order *o)
{
    return o->uid;
}

const Drink *getDrink(const Order *o)
{
    return o->drink;
}