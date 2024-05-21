#ifndef FUNC_H
#define FUNC_H

#include "ring.h"
#include "stack.h"
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define FILL_SEM_NAME "/fill_sem"
#define EXTRACT_SEM_NAME "/extract_sem"
#define QUEUE_ACCESS_SEM_NAME "/queue_sem"

extern int isContinuing;
extern Queue* messageQueue;

void *fillingMessages();
void *extractingMessages();

void toggleContinuingStatus();

#endif
