#include <stdio.h>
#include <stdlib.h>
#include "orders.h"

#define SIZE 8

int isFull();
int isEmpty();
Order *getFront(void);
Order *getRear(void);
int enqueue(const uint8_t *, const Drink *);
int dequeue();
int findRFID(const uint8_t *);
int getPosition(int);

extern Order *queue[SIZE];