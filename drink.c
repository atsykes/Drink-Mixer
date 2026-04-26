#include <stdio.h>
#include <stdbool.h>
#include "drink.h"

// Information for Each Drink

const char *getName(const Drink *d)
{
    return d->name;
}

const bool *getIngredients(const Drink *d)
{
    return d->ingredients;
}