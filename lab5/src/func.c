#include "func.h"

void *fillingMessages() {
    sem_t* queueAccessSemaphore = sem_open(QUEUE_ACCESS_SEM_NAME, 0);
    if (queueAccessSemaphore == SEM_FAILED) {
        printf("Error while opening queue access semaphore.\n");
        exit(EXIT_FAILURE);
    }

    sem_t* fillSemaphore = sem_open(FILL_SEM_NAME, 0);
    if (fillSemaphore == SEM_FAILED) {
        printf("Error while opening filling semaphore.\n");
        exit(EXIT_FAILURE);
    }

    while (isContinuing) {
        sem_wait(queueAccessSemaphore);
        sem_wait(fillSemaphore);

        if (messageQueue->countAdded - messageQueue->countDeleted < MAX_MESSAGE_COUNT) {
            push(&messageQueue->ringHead, &messageQueue->ringTail);
            messageQueue->countAdded++;
            printf("Added %d message:\n", messageQueue->countAdded);
            printMessage(messageQueue->ringTail->message);
        } else {
            printf("Queue is full!\n");
        }

        sem_post(fillSemaphore);
        sem_post(queueAccessSemaphore);
        sleep(3);
    }

    sem_close(queueAccessSemaphore);
    sem_close(fillSemaphore);
    return NULL;
}

void *extractingMessages() {
    sem_t* queueAccessSemaphore = sem_open(QUEUE_ACCESS_SEM_NAME, 0);
    if (queueAccessSemaphore == SEM_FAILED) {
        printf("Error while opening queue access semaphore.\n");
        exit(EXIT_FAILURE);
    }

    sem_t* extractSemaphore = sem_open(EXTRACT_SEM_NAME, 0);
    if (extractSemaphore == SEM_FAILED) {
        printf("Error while opening extracting semaphore.\n");
        exit(EXIT_FAILURE);
    }

    while (isContinuing) {
        sem_wait(queueAccessSemaphore);
        sem_wait(extractSemaphore);

        if (messageQueue->countAdded - messageQueue->countDeleted > 0) {
            Message* tempMessage = messageQueue->ringHead->message;
            pop(&messageQueue->ringHead, &messageQueue->ringTail);
            messageQueue->countDeleted++;
            printf("Deleted %d message:\n", messageQueue->countDeleted);
            printMessage(tempMessage);
            free(tempMessage->data);
            free(tempMessage);
        } else {
            printf("Queue is empty!\n");
        }

        sem_post(extractSemaphore);
        sem_post(queueAccessSemaphore);
        sleep(3);
    }

    sem_close(queueAccessSemaphore);
    sem_close(extractSemaphore);
    return NULL;
}

void toggleContinuingStatus() {
    isContinuing ^= 1;
}
