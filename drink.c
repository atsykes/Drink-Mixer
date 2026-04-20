#include <stdio.h>
#include <stdbool.h>
#include "drink.h"

// Information for Each Drink
struct Drink
{
    char name[16];
    bool ingredients[4];
};

const char *getName(const Drink *d)
{
    return d->name;
}

const bool *getIngredients(const Drink *d)
{
    return d->ingredients;
}