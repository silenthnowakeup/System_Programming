#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct StackNode {
    pthread_t threadId;
    struct StackNode* next;
} StackNode;

void pushStack(StackNode** stackHead, pthread_t threadId);
void popStack(StackNode** stackHead);

#endif
