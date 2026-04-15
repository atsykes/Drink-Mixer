#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include <stdbool.h>

#include "lcd.h"
#include "adc.h" // This might need to be updated for the new microcontroller

#include "queue.h" // Circular queue for orders

#define RIGHT [value]
#define LEFT [value]
#define MAXDRINKID [value]
#define MINDRINKID 0

void displayMenu(int);
char *_getDrink(int);

bool updateDrink = 0;
volatile bool pumps = 0; // do not enable the sonar sensor interrupts
volatile int drink = 0;  // which drink to display on the screen

char menu[4][16]

    enum states {
        IDLE,
        START,
        PAUSE // When the pumps dispense -- from sonar sensor interrupt
    };

/* Software Todos
    Todo: Figure out how to get interrupts from the Sonar Sensors
    Todo: Finish logic for the pumping states + pump timers
    Todo: Logic for printing the drinks on the lcd screen
    Todo: Dummy Conveyor Belt Logic (either LED or buzzer?)
*/

/* Hardware Todos
    Todo: Everything
*/

int main(void)
{
    int state = IDLE;

    int pump1 = 0;
    int pump2 = 0;
    int pump3 = 0;
    int pump4 = 0;
    while (1)
    {
        displayMenu(drink);

        /* ADC Button LOGIC
            Todo: Find a way to sample the buttons without using the delay
            Todo: Find the adc values from the buttons
            Todo: Make the button press update the lcd screen
        */
        uint8_t button = adc_sample(0);
        // _delay_ms(100);

        if (button == RIGHT)
            drink++;
        else if (button = LEFT)
            drink--;

        if (drink < MINDRINKID)
            drink = MINDRINKID;
        else if (drink > MAXDRINKID)
            drink = MAXDRINKID;

        if (state == IDLE)
        {
        }
        else if (state == START)
        {
        }
        else
        {
        }

        // State Transitions
        if ((state == IDLE) && (!isEmpty()))
        {
            state = START;

            pump1 = getFront();
            pump2 = getFront();
            pump3 = getFront();
            pump4 = getFront();

            pumps = true;
        }
        else if ((state == START) && (isEmpty()))
        {
            state = IDLE;

            pumps = false;
        }
    }

    return 0;
}

void displayMenu(int drink)
{
    /* This function writes the drink to the lcd screen
       If updateDrink is True, then do not write to the lcd screen again
    */

    if (updateDrink)
    {
        updateDrink = 1;
        return
    }
    lcd_writecommand(1);
    lcd_writecommand(2);
    char name[8];
    snprintf(name, 8, "Drink %d", drink) // stopping here to utilize getDrink later
}

char **_getDrink(int drink)
{
    /* Up for debate but might have it return a char ** for the name + ingredients */
    char splash[12] = "EE109 Lab 7";

    char name[16];
    char ingredients;
    lcd_stringout(splash);

    if (drink == 0)
    {
        name = "Name1" ingredients = 5
    }
    if (drink == 1)
        name = "Orange Safness" ingredent s = "Orng Water"

            char display[2][16] = { name,
                                    ingredients } return display;
}