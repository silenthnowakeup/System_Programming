#include "ring.h"

void push(Node** head, Node** tail) {
    if (*head != NULL) {
        Node *temp = (Node *) malloc(sizeof(Node));
        temp->message = (Message*)malloc(sizeof(Message));
        initMessage(temp->message);
        temp->next = *head;
        temp->prev = *tail;
        (*tail)->next = temp;
        (*head)->prev = temp;
        *tail = temp;
    } else {
        *head = (Node *) malloc(sizeof(Node));
        (*head)->message = (Message*)malloc(sizeof(Message));
        initMessage((*head)->message);
        (*head)->prev = *head;
        (*head)->next = *head;
        *tail = *head;
    }
}

void pop(Node** head, Node** tail) {
    if (*head != NULL) {
        if (*head != *tail) {
            Node *temp = *head;
            (*tail)->next = (*head)->next;
            *head = (*head)->next;
            (*head)->prev = *tail;
            free(temp);
        } else {
            free(*head);
            *head = NULL;
            *tail = NULL;
        }
    }
}

void initMessage(Message* message) {
    message->type = 0;
    message->hash = 0;
    message->size = rand() % 257;
    message->data = (uint8_t*)malloc(message->size*sizeof(uint8_t));
    for (size_t i = 0; i < message->size; i++) {
        message->data[i] = rand() % 256;
        message->hash += message->data[i];
    }
    message->hash = ~message->hash;
}

void printMessage(Message* message) {
    printf("Message type: %d, hash: %d, size: %d, data: ", message->type, message->hash, message->size);
    for(size_t i = 0; i<message->size; i++)
        printf("%d", message->data[i]);
    printf("\n");
}
