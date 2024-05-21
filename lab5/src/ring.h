#ifndef RING_H
#define RING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

extern int MAX_MESSAGE_COUNT;

typedef struct {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    uint8_t* data;
} Message;

typedef struct Node {
    Message* message;
    struct Node* next;
    struct Node* prev;
} Node;

typedef struct {
    Node* ringHead;
    Node* ringTail;
    int countAdded;
    int countDeleted;
} Queue;

void push(Node** head, Node** tail);
void pop(Node** head, Node** tail);

void initMessage(Message*);
void printMessage(Message*);

#endif
