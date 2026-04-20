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

void updateOrder(Order *order, uint8_t *uid, Drink *drink)
{
    /**
     * @brief Update the order variable with the new uid and drink for the queue
     */
    for (int i = 0; i < 5; i++)
    {
        order->uid[i] = uid[i];
    }
    order->drink = drink;
}