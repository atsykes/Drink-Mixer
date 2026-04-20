#include <stdio.h>
#include <stdlib.h>
#include "orders.h"

#define SIZE 8

Order *queue[SIZE];
int front = -1, rear = -1;

// Check if queue is full
int isFull()
{
    return (rear + 1) % SIZE == front;
}

// Check if queue is empty
int isEmpty()
{
    return front == -1;
}

Order *getFront(void)
{
    if (isEmpty())
        return -1; // or some error value
    return queue[front];
}

Order *getRear(void)
{
    if (isEmpty())
        return -1; // or some error value
    return queue[rear];
}

// Enqueue operation
int enqueue(int value)
{
    if (isFull())
    {
        return -1;
    }

    if (isEmpty())
    {
        front = rear = 0;
    }
    else
    {
        rear = (rear + 1) % SIZE;
    }

    queue[rear] = value;
    return 1;
}

// Dequeue operation
int dequeue()
{
    if (isEmpty())
    {
        return -1;
    }

    int value = queue[front];

    if (front == rear)
    {
        // Queue becomes empty
        front = rear = -1;
    }
    else
    {
        front = (front + 1) % SIZE;
    }

    return value;
}

int findRFID(uint8_t uid)
{
    /* Returns the index if found, -1 otherwise */
    if (isEmpty())
        return -1;

    int i = front;

    while (1)
    {
        if (queue[i]->uid == uid)
            return i; // return index where found

        if (i == rear)
            break;

        i = (i + 1) % SIZE;
    }

    return -1; // not found
}