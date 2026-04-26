#include <stdio.h>
#include <stdbool.h>
#include "drink.h"
#include "lcd_i2c.h"

/**
 * Ingredients
 * 1) Orange Juice (OJ)
 * 2) Cranberry Juice (CJ)
 * 3) Peach Flavoring (Pch)
 * 4) Spicy Sauce (Vdk)
 *
 * Drinks
 * 1) Vdk + OJ = Screwdriver (Screw)
 * 2) Vdk + CJ = Cape Codder (Codder)
 * 3) Vdk + Pch = Fuzzy Navel (Fuzz)
 * 4) Vdk + OJ + CJ = Madras (Madras)
 * 5) Vdk + CJ + Pch = Woo Woo (Woo)
 * 6) Vdk + OJ + CJ + Pch = Sex on the Beach (Beachy)
 */

// Information for Each Drink

Drink screwdriver = {"Screw", {true, false, false, true}};
Drink cape_codder = {"Codder", {false, true, false, true}};
Drink fuzzy_navel = {"Fuzz", {false, false, true, true}};
Drink madras = {"Madras", {true, true, false, true}};
Drink woo_woo = {"Woo", {false, true, true, true}};
Drink beachy = {"Beachy", {true, true, true, true}};

Drink *drinks[6] = {&screwdriver, &cape_codder, &fuzzy_navel, &madras, &woo_woo, &beachy};

const char *getName(const Drink *d)
{
    return d->name;
}

const bool *getIngredients(const Drink *d)
{
    return d->ingredients;
}

void displayMenu(Drink **drinks, int drink, bool *updateDrink)
{
    /* This function writes the drink to the lcd screen
       If updateDrink is True, then do not write to the lcd screen again
    */

    /**
     * Ingredients
     * 1) Orange Juice (OJ)
     * 2) Cranberry Juice (CJ)
     * 3) Peach Flavoring (Pch)
     * 4) Spicy Sauce (Vdk)
     */

    Drink *curr = drinks[drink];
    char *ingr[4] = {"OJ", "CJ", "Pch", "Vdk"};

    if (!(*updateDrink))
        return;

    lcd_writecommand(1);
    lcd_writecommand(2);
    lcd_stringout(curr->name);
    lcd_moveto(1, 0);
    for (int i = 0; i < 4; i++)
    {
        if (curr->ingredients[i])
        {
            lcd_stringout(ingr[i]);
            if (i != 3)
                lcd_stringout(", ");
        }
    }

    *updateDrink = 0;
}
