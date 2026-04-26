#include <stdio.h>
#include <stdlib.h>
#include "orders.h"
#include "queue.h"

Order o0, o1, o2, o3, o4, o5, o6, o7;

Order *queue[SIZE] = {&o0, &o1, &o2, &o3, &o4, &o5, &o6, &o7};
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
        return NULL; // or some error value
    return queue[front];
}

Order *getRear(void)
{
    if (isEmpty())
        return NULL; // or some error value
    return queue[rear];
}

// Enqueue operation
int enqueue(const uint8_t *uid, const Drink *drink)
{
    /**
     * @brief Add order to the queue
     */
    if (isFull())
        return -1;

    if (isEmpty())
        front = rear = 0;
    else
        rear = (rear + 1) % SIZE;

    updateOrder(queue[rear], uid, drink);

    return 1;
}

// Dequeue operation
int dequeue()
{
    /**
     * @brief Remove order from the queue
     */
    if (isEmpty())
    {
        return -1; // if it is already empty
    }

    if (front == rear)
    {
        // Queue becomes empty
        front = rear = -1;
    }
    else
    {
        front = (front + 1) % SIZE;
    }

    return 1; // if it works
}

int findRFID(const uint8_t *uid)
{
    /**
     * @brief Returns the index if found, -1 otherwise
     */
    if (isEmpty())
        return -1;

    int i = front;
    while (1)
    {

        for (int j = 0; j < 5; j++)
        {
            if (uid[j] == queue[i]->uid[j])
            {
                if (j == 4)
                    return i;
            }
            else
                break;
        }

        if (i == rear)
            break;

        i = (i + 1) % SIZE;
    }

    return -1; // not found
}

int getPosition(int index)
{
    /**
     * @brief Given a queue index, returns the position in line (1-based)
     *        Returns -1 if index is not in the queue
     */
    if (isEmpty())
        return -1;

    int i = front;
    int position = 1;

    while (1)
    {
        if (i == index)
            return position;

        if (i == rear)
            break;

        i = (i + 1) % SIZE;
        position++;
    }

    return -1; // index not found in queue
}