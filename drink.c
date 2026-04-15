#include <stdio.h>
#include <stdbool.h>
#include "drink.h"

struct Drink
{
    char name[16];
    bool *ingredients[4];
};

char *getName(Drink *d)
{
    return d->name;
}

const bool *getIngredients(const Drink *d)
{
    return d->ingredients;
}