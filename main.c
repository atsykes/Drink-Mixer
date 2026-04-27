#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include <stdbool.h>

#include "lcd_i2c.h"
#include "adc.h"
#include "drink.h"
#include "sonar.h"
#include "queue.h"
#include "orders.h"

// #include "queue.h" // Circular queue for orders

#include "rfid.h"

// ADC Button Values
#define RIGHT 0
#define LEFT 130
#define NOMINAL 255

// How many drinks do we have
#define MAXDRINKID 5
#define MINDRINKID 0

#define NUM_PUMPS 4
#define MAX_DISTANCE 15
#define DISPENSE_TIME 15625

// Functions
void displayOrder(int);
bool equal_uid(uint8_t uid[5], uint8_t old_uid[5]);
void set_uid(uint8_t uid[5], uint8_t old_uid[5]);
bool callSensors(bool *);
void timer3_init(void);
void updateBelt(bool state);
void startDispense(bool *pump_en, int *pump1, int *pump2, int *pump3, int *pump4);
void endDispense(bool *pump_en);

// void updateBelt(bool state)

volatile int drink = 0; // index of the drink menu (to display on screen and to be added to the orders)

enum states
{
    IDLE,
    START,
    DISPENSE, // If sonar sensors are activated, dispense ingredient into cup
    TIMEOUT   // Timeout period, wait for drinks to pass the pumps
};

int main(void)
{

    bool updateDrink = true;

    // RFID reader init ----------------------
    uint8_t status;
    uint8_t tag_type[2];
    uint8_t uid[5];
    uint8_t old_uid[5] = {0, 0, 0, 0, 0};

    hardware_init();
    rc522_init();

    // Pumps init ----------------------------
    int state = IDLE;

    // These store the index to the orders queue
    int pump1 = 0;
    int pump2 = 0;
    int pump3 = 0;
    int pump4 = 0;

    bool rbutton = 0;
    bool lbutton = 0;

    bool pump_en[4] = {0, 0, 0, 0};                            // Is there a cup in front of a pump?
    DDRA |= (1 << PA4) | (1 << PA5) | (1 << PA6) | (1 << PA7); // Pumps
    DDRD |= (1 << PD7);                                        // Motor

    lcd_init();
    adc_init();
    timer1_pcint_init();
    timer3_init();

    lcd_writecommand(1);
    lcd_writecommand(2);
    char splash[16] = "The 24Hr Social";
    char name[14] = "Andrew & Le";
    lcd_stringout(splash);
    lcd_moveto(1, 0);
    lcd_stringout(name);
    _delay_ms(300);

    lcd_writecommand(1);
    bool displayingOrder = false;
    int displayCnt = 1;
    int cnt = 0;

    int found = -1;
    int old_found = -2;

    bool inFront = 0;

    bool dispense = 0;
    uint16_t dispenseStartTime;

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
                // Queue Functionality
                found = findRFID(uid);
                if (found == -1) // Meaning this cup is not already in the queue
                    enqueue(uid, drinks[drink]);
                else
                {
                    if (!equal_uid(uid, old_uid))
                    {
                        displayOrder(found);
                        set_uid(uid, old_uid);
                        displayingOrder = true;
                    }
                }
            }
        }
        if (displayingOrder && displayCnt % 100 == 0)
        {
            displayingOrder = false;
            displayCnt = 1;
            updateDrink = true;
        }
        else if (!displayingOrder)
            displayCnt = 1;

        if (!displayingOrder)
            displayMenu(drinks, drink, &updateDrink);

        /* ADC Button LOGIC */
        uint8_t button = adc_sample(0);
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

        // State Actions
        if (state == IDLE)
        {
            updateBelt(0);
        }
        else if (state == START)
        {
            updateBelt(1);
        }
        else if (state == DISPENSE)
        {
            updateBelt(0);
            if (!dispense)
            {
                dispense = true;
                dispenseStartTime = TCNT3;
                startDispense(pump_en, &pump1, &pump2, &pump3, &pump4);
            }
            else if (dispense && (uint16_t)(TCNT3 - dispenseStartTime) >= DISPENSE_TIME)
            {
                dispense = false;
                endDispense(pump_en);
            }
        }

        // Sonar Sensors
        if ((cnt % 12 == 0) && ((state == TIMEOUT) || (state == START)))
        {
            cnt = 0;
            inFront = callSensors(pump_en);
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
            if (!isEmpty())
                state = START;
            else
                state = IDLE;

            // Reset pump enables due to nature of callSensors
            pump_en[0] = 0;
            pump_en[1] = 0;
            pump_en[2] = 0;
            pump_en[3] = 0;
        }

        if (state == TIMEOUT || state == START) // Don't need to worry about cnt being initialized 0, when we start counting again
            cnt++;
        displayCnt++;
        _delay_ms(5);
    }

    return 0;
}

void updateBelt(bool state)
{
    if (state)
        PORTD |= (1 << PD7); // Turn on belt (LED for now?)
    else
        PORTD &= ~(1 << PD7); // Turn off belt
}

void displayOrder(int i)
{
    int pos = getPosition(i);
    lcd_moveto(0, 0);
    char row1[16];
    snprintf(row1, 16, "Pos: %d", pos);
    lcd_stringout(row1);
    lcd_moveto(1, 0);
    lcd_stringout("                ");
    lcd_moveto(1, 0);
    lcd_stringout(getName(queue[i]->drink));
}

bool equal_uid(uint8_t uid[5], uint8_t old_uid[5])
{
    for (int i = 0; i < 5; i++)
    {
        if (uid[i] != old_uid[i])
            return false;
    }
    return true;
}

void set_uid(uint8_t uid[5], uint8_t old_uid[5])
{
    for (int i = 0; i < 5; i++)
    {
        old_uid[i] = uid[i];
    }
}

bool callSensors(bool *pump_en)
{
    int cnt = 0;
    bool inFront = 0;
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        pollSensors(i);

        // Wait for pulse_done with timeout
        uint16_t start = TCNT1;
        while (!sensors[i].pulse_done && (uint16_t)(TCNT1 - start) < 870)
            ;

        if (sensors[i].pulse_done)
        {
            uint16_t p;
            p = sensors[i].pulse;
            sensors[i].pulse_done = false;

            uint16_t distance = p / 58;
            // display distance for sensor i
            if (distance < MAX_DISTANCE)
                pump_en[i] = true;
            else
                ++cnt;
        }
    }
    if (cnt == NUM_SENSORS)
        return false;
    return true;
}

void startDispense(bool *pump_en, int *pump1, int *pump2, int *pump3, int *pump4)
{
    if (pump_en[0])
    {
        if (queue[*pump1]->drink->ingredients[0])
            PORTA |= (1 << PA4);
        (*pump1)++;
    }
    if (pump_en[1])
    {
        if (queue[*pump2]->drink->ingredients[1])
            PORTA |= (1 << PA5);
        (*pump2)++;
    }
    if (pump_en[2])
    {
        if (queue[*pump3]->drink->ingredients[2])
            PORTA |= (1 << PA6);
        (*pump3)++;
    }
    if (pump_en[3])
    {
        if (queue[*pump4]->drink->ingredients[3])
            PORTA |= (1 << PA7);
        (*pump4)++;
        dequeue();
    }
}

void endDispense(bool *pump_en)
{
    PORTA &= ~((1 << PA4) | (1 << PA5) | (1 << PA6) | (1 << PA7));
    for (int i = 0; i < 4; i++)
    {
        pump_en[i] = 0;
    }
}

void timer3_init(void)
{
    TCCR3A = 0;
    TCCR3B = (1 << CS32) | (1 << CS30); // prescaler /1024, ~128µs per tick
    TCNT3 = 0;
}