#ifndef DRINK_H
#define DRINK_H

#include <stdbool.h>

typedef struct
{
    char name[16];
    bool ingredients[4];
} Drink;

const char *getName(const Drink *);
const bool *getIngredients(const Drink *);

#endif