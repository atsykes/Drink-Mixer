#include <stdio.h>
#include <stdlib.h>
#include "orders.h"

int isFull();
int isEmpty();
Order *getFront(void);
Order *getRear(void);
int enqueue(int);
int dequeue();
int findRFID(uint8_t)