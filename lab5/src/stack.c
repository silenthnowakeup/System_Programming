#include "stack.h"

void pushStack(StackNode** stackHead, pthread_t threadId) {
    StackNode *newNode = (StackNode *) malloc(sizeof(StackNode));
    newNode->next = *stackHead;
    newNode->threadId = threadId;
    *stackHead = newNode;
}

void popStack(StackNode** stackHead) {
    if (*stackHead != NULL) {
        StackNode *temp = *stackHead;
        *stackHead = (*stackHead)->next;
        free(temp);
    }
}
