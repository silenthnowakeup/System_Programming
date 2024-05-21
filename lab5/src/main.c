#include "func.h"

int isContinuing = 1;
int MAX_MESSAGE_COUNT = 20;

StackNode* stackFiller = NULL;
StackNode* stackExtractor = NULL;

Queue* messageQueue;

int main() {
    srand(time(NULL));

    sem_unlink(FILL_SEM_NAME);
    sem_unlink(EXTRACT_SEM_NAME);
    sem_unlink(QUEUE_ACCESS_SEM_NAME);

    signal(SIGUSR1, toggleContinuingStatus);

    messageQueue = (Queue*)malloc(sizeof(Queue));
    messageQueue->ringHead = NULL;
    messageQueue->ringTail = NULL;
    messageQueue->countDeleted = 0;
    messageQueue->countAdded = 0;

    sem_t* fillSemaphore;
    if ((fillSemaphore = sem_open(FILL_SEM_NAME, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) {
        printf("Error while opening filling semaphore, code %d.\n", errno);
        exit(errno);
    }

    sem_t* extractSemaphore;
    if ((extractSemaphore = sem_open(EXTRACT_SEM_NAME, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) {
        printf("Error while opening extracting semaphore, code %d.\n", errno);
        exit(errno);
    }

    sem_t* queueAccessSemaphore;
    if ((queueAccessSemaphore = sem_open(QUEUE_ACCESS_SEM_NAME, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) {
        printf("Error while opening queue semaphore, code %d.\n", errno);
        exit(errno);
    }

    while (isContinuing) {
        char ch = getchar();
        switch (ch) {
            case 'w': {
                pthread_t threadId;
                if (pthread_create(&threadId, NULL, fillingMessages, NULL) != 0) {
                    printf("Error while creating filling thread.");
                    isContinuing = 0;
                } else {
                    pushStack(&stackFiller, threadId);
                }
                break;
            }
            case 's':
                if (stackFiller != NULL) {
                    pthread_kill(stackFiller->threadId, SIGUSR1);
                    pthread_join(stackFiller->threadId, NULL);
                    popStack(&stackFiller);
                    isContinuing = 1;
                } else {
                    printf("There are no fillers.\n");
                }
                break;
            case 'e': {
                pthread_t threadId;
                if (pthread_create(&threadId, NULL, extractingMessages, NULL) != 0) {
                    printf("Error while creating extracting thread.");
                    isContinuing = 0;
                } else {
                    pushStack(&stackExtractor, threadId);
                }
                break;
            }
            case 'd':
                if (stackExtractor != NULL) {
                    pthread_kill(stackExtractor->threadId, SIGUSR1);
                    pthread_join(stackExtractor->threadId, NULL);
                    popStack(&stackExtractor);
                    isContinuing = 1;
                } else {
                    printf("There are no extractors.\n");
                }
                break;
            case '+':
                MAX_MESSAGE_COUNT++;
                break;
            case '-':
                if (messageQueue->countAdded - messageQueue->countDeleted < MAX_MESSAGE_COUNT) {
                    MAX_MESSAGE_COUNT--;
                }
                break;
            case 'q':
                while (stackFiller != NULL) {
                    pthread_kill(stackFiller->threadId, SIGUSR1);
                    pthread_join(stackFiller->threadId, NULL);
                    popStack(&stackFiller);
                    isContinuing = 1;
                }
                while (stackExtractor != NULL) {
                    pthread_kill(stackExtractor->threadId, SIGUSR1);
                    pthread_join(stackExtractor->threadId, NULL);
                    popStack(&stackExtractor);
                    isContinuing = 1;
                }
                printf("All fillers and extractors have been terminated. End of program.\n");
                isContinuing = 0;
                break;
        }
    }

    sem_close(fillSemaphore);
    sem_unlink(FILL_SEM_NAME);
    sem_close(extractSemaphore);
    sem_unlink(EXTRACT_SEM_NAME);
    sem_close(queueAccessSemaphore);
    sem_unlink(QUEUE_ACCESS_SEM_NAME);

    return 0;
}
