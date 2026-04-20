#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include <stdbool.h>

#include "lcd.h" // Todo: Update for new microcontroller
#include "adc.h" // Todo: Update for new microcontroller

#include "queue.h" // Circular queue for orders

#include "RFID/rfid.h"

// ADC Button Values
#define RIGHT [value]
#define LEFT [value]
#define NOMINAL // Nominal state (no buttons being pressed)

// How many drinks do we have
#define MAXDRINKID [value]
#define MINDRINKID 0

#define NUM_PUMPS 2

// Functions
// void displayMenu(int);
// char *_getDrink(int);
// void updateBelt(bool state)
// bool pollSensors(bool *pump_en)

bool updateDrink = 0;
volatile int drink = 0; // index of the drink menu (to display on screen and to be added to the orders)

Drink *drink_menu[MAXDRINKID + 1] = {}; // List of available drinks

// TODO: Issue - during the Timeout state, it is possible that a cup gets in front of another sensor
// TODO: To combat this, we should just enforce equal spacing (or find some other SW alternative)
enum states
{
    IDLE,
    START,
    DISPENSE, // If sonar sensors are activated, dispense ingredient into cup
    TIMEOUT   // Timeout period, wait for drinks to pass the pumps
};

int main(void)
{
    // RFID reader init ----------------------
    uint8_t status;
    uint8_t tag_type[2];
    uint8_t uid[5];

    hardware_init();
    rc522_init();

    uart_print("RC522 ready\r\n");
    // Ultrasonic sensor init ----------------
    ultrasonic_init();
    uart_print("Ultrasonic ready\r\n");
    // Pumps init ----------------------------
    int state = IDLE;

    // These store the index to the orders queue
    int pump1 = 0;
    int pump2 = 0;
    int pump3 = 0;
    int pump4 = 0;

    bool rbutton = 0;
    bool lbutton = 0;

    bool pump_en[4] = {0, 0, 0, 0}; // Is there a cup in front of a pump?
    while (1)
    {
        // RFID Stuff
        status = rc522_request(PICC_REQIDL, tag_type);
        // uart_print("Requesting ID\r\n"); // for debugging
        if (status == MI_OK)
        {
            status = rc522_anticoll(uid);
            if (status == MI_OK)
            {
                // uart_print("ID received: ");
                // print_uid(uid, 4);
                // uart_print("Checking queue . . .\r\n");
                // Check queue
                //_delay_ms(500);

                // Queue Functionality
                if (findRFID(uid) == -1)
                    enqueue(uid, drink_menu[drink])
            }
        }

        // // ULTRASONIC SENSOR
        // // NOTE: Not timers nor interupt
        // // To-do: implement timer and interupt
        // if (ultrasonic_object_within_2cm()) {
        //     if (!object_present) {
        //         uart_print("Object detected within 2 cm\r\n");
        //     }
        //     object_present = 1;
        // } else {
        //     object_present = 0;
        // }

        displayMenu(drink);

        /* ADC Button LOGIC
            Todo: Find a way to sample the buttons without using the delay
            Todo: Find the adc values from the buttons
            Todo: Make the button press update the lcd screen
        */
        uint8_t button = adc_sample(0);
        // _delay_ms(100);

        if (button == RIGHT) // Add +- ADC range
            rbutton = 1;
        else if (button == LEFT) // Add +- ADC range
            lbutton = 1;

        if (rbutton && button == NOMINAL)
        {
            rbutton = 0;
            drink++;
        }
        else if (lbutton && button == NOMINAL)
        {
            lbutton = 0;
            drink--;
        }

        if (drink < MINDRINKID)
            drink = MINDRINKID;
        else if (drink > MAXDRINKID)
            drink = MAXDRINKID;

        // Sonar Sensor
        bool inFront = 0;

        // State Actions
        if (state == IDLE)
        {
            updateBelt(0);
        }
        else if (state == START)
        {
            updateBelt(1);
            inFront = pollSensors(pump_en); // Checks if there are any cups in front of the sensors
        }
        else if (state == DISPENSE)
        {
            // Todo: Add some logic to dispense from pumps to each cup
            // Todo: After each cup is dispense, the pump_en should be reset to 0 for each pump
            // Todo: The last pump should remove the order from the queue
            updateBelt(0);
        }
        else if (state == TIMEOUT)
        {
            inFront = pollSensors(pump_en)
        }

        // State Transitions
        if ((state == IDLE) && (!isEmpty()))
        {
            state = START;

            pump1 = getFront();
            pump2 = getFront();
            pump3 = getFront();
            pump4 = getFront();
        }
        else if (state == START)
        {
            if (isEmpty())
            {
                state = IDLE;
            }
            else if (inFront)
            {
                state = DISPENSE;
            }
        }
        else if ((state == DISPENSE) && !(pump_en[0] | pump_en[1] | pump_en[2] | pump_en[3])) // Check pump enables (should go to 0 after dispensing)
        {
            state = TIMEOUT;
        }
        else if ((state == TIMEOUT) && !inFront)
        {
            if (!isEmpty)
                state = START;
            else
                state = IDLE;
        }
    }

    return 0;
}

bool pollSensors(bool *pump_en)
{
    // Todo: If ANY of sensors have a drink in front of it, it should make inFront to true
    bool inFront = 0;
    for (int i = 0; i < NUM_PUMPS; i++)
    {
        if (/* we detect a sensor for some pump */)
            pump_en[i] == 1;
    }

    return inFront;
}

void updateBelt(bool state)
{
    if (state)
        ; // Turn on belt (LED for now?)
    else
        ; // Turn off belt
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