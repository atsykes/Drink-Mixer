#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include <stdbool.h>

#include "../lcd_i2c.h"
#include "../adc.h"
#include "../drink.h"

// #include "queue.h" // Circular queue for orders

// #include "RFID/rfid.h"

// ADC Button Valuesx
#define RIGHT 0
#define LEFT 130
#define NOMINAL 255

// How many drinks do we have
#define MAXDRINKID 5
#define MINDRINKID 0

#define NUM_PUMPS 2

volatile int drink = 0; // index of the drink menu (to display on screen and to be added to the orders)

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

    Drink screwdriver = {"Screw", {true, false, false, true}};
    Drink cape_codder = {"Codder", {false, true, false, true}};
    Drink fuzzy_navel = {"Fuzz", {false, false, true, true}};
    Drink madras = {"Madras", {true, true, false, true}};
    Drink woo_woo = {"Woo", {false, true, true, true}};
    Drink beachy = {"Beachy", {true, true, true, true}};

    Drink *drinks[6] = {&screwdriver, &cape_codder, &fuzzy_navel, &madras, &woo_woo, &beachy};
    bool updateDrink = true;

    // // RFID reader init ----------------------
    // uint8_t status;
    // uint8_t tag_type[2];
    // uint8_t uid[5];

    // hardware_init();
    // rc522_init();

    // uart_print("RC522 ready\r\n");
    // // Ultrasonic sensor init ----------------
    // ultrasonic_init();
    // uart_print("Ultrasonic ready\r\n");
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

    lcd_init();
    adc_init();

    lcd_writecommand(1);
    lcd_writecommand(2);
    char splash[12] = "EE109 Lab 5";
    char name[14] = "Andrew Sykes";
    lcd_stringout(splash);
    lcd_moveto(1, 0);
    lcd_stringout(name);
    _delay_ms(100);

    lcd_writecommand(1);

    while (1)
    {
        // // RFID Stuff
        // status = rc522_request(PICC_REQIDL, tag_type);
        // // uart_print("Requesting ID\r\n"); // for debugging
        // if (status == MI_OK)
        // {
        //     status = rc522_anticoll(uid);
        //     if (status == MI_OK)
        //     {
        //         // uart_print("ID received: ");
        //         // print_uid(uid, 4);
        //         // uart_print("Checking queue . . .\r\n");
        //         // Check queue
        //         //_delay_ms(500);

        //         // Queue Functionality
        //         if (findRFID(uid) == -1) // Meaning this cup is not already in the queue
        //             enqueue(uid, drink_menu[drink])
        //     }
        // }

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
        displayMenu(drinks, drink, &updateDrink);

        /* ADC Button LOGIC */
        uint8_t button = adc_sample(0);
        // _delay_ms(100);

        if (button >= RIGHT - 10 && button <= RIGHT + 10) // Add +- ADC range
            rbutton = 1;
        else if (button >= LEFT - 10 && button <= LEFT + 10) // Add +- ADC range
            lbutton = 1;

        if (rbutton && button >= NOMINAL - 10 && button <= NOMINAL + 10)
        {
            rbutton = 0;
            drink++;
            updateDrink = true;
        }
        else if (lbutton && button >= NOMINAL - 10 && button <= NOMINAL + 10)
        {
            lbutton = 0;
            drink--;
            updateDrink = true;
        }

        if (drink < MINDRINKID)
            drink = MAXDRINKID;
        else if (drink > MAXDRINKID)
            drink = MINDRINKID;

        // // Sonar Sensor
        // bool inFront = 0;

        // // State Actions
        // if (state == IDLE)
        // {
        //     updateBelt(0);
        // }
        // else if (state == START)
        // {
        //     updateBelt(1);
        //     inFront = pollSensors(pump_en); // Checks if there are any cups in front of the sensors
        // }
        // else if (state == DISPENSE)
        // {
        //     // Todo: Add some logic to dispense from pumps to each cup
        //     // Todo: After each cup is dispense, the pump_en should be reset to 0 for each pump
        //     // Todo: The last pump should remove the order from the queue
        //     updateBelt(0);
        // }
        // else if (state == TIMEOUT)
        // {
        //     inFront = pollSensors(pump_en)
        // }

        // // State Transitions
        // if ((state == IDLE) && (!isEmpty()))
        // {
        //     state = START;

        //     pump1 = getFront();
        //     pump2 = getFront();
        //     pump3 = getFront();
        //     pump4 = getFront();
        // }
        // else if (state == START)
        // {
        //     if (isEmpty())
        //     {
        //         state = IDLE;
        //     }
        //     else if (inFront)
        //     {
        //         state = DISPENSE;
        //     }
        // }
        // else if ((state == DISPENSE) && !(pump_en[0] | pump_en[1] | pump_en[2] | pump_en[3])) // Check pump enables (should go to 0 after dispensing)
        // {
        //     state = TIMEOUT;
        // }
        // else if ((state == TIMEOUT) && !inFront)
        // {
        //     if (!isEmpty)
        //         state = START;
        //     else
        //         state = IDLE;
        // }
    }

    return 0;
}