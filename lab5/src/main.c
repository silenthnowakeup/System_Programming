#include "func.h"

int isContinuing = 1;  // Переменная для отслеживания продолжения работы программы
int MAX_MESSAGE_COUNT = 20;  // Максимальное количество сообщений в очереди

StackNode* stackFiller = NULL;  // Стек для заполнителей
StackNode* stackExtractor = NULL;  // Стек для извлекателей

Queue* messageQueue;  // Очередь сообщений

int main() {
  srand(time(NULL));  // Инициализация генератора случайных чисел

  // Удаление семафоров, если они были созданы ранее
  sem_unlink(FILL_SEM_NAME);
  sem_unlink(EXTRACT_SEM_NAME);
  sem_unlink(QUEUE_ACCESS_SEM_NAME);

  signal(SIGUSR1, toggleContinuingStatus);  // Установка обработчика сигнала SIGUSR1

  // Инициализация очереди сообщений
  messageQueue = (Queue*)malloc(sizeof(Queue));
  messageQueue->ringHead = NULL;
  messageQueue->ringTail = NULL;
  messageQueue->countDeleted = 0;
  messageQueue->countAdded = 0;

  // Создание семафоров
  sem_t* fillSemaphore;
  if ((fillSemaphore = sem_open(FILL_SEM_NAME, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) {
    printf("Ошибка при открытии семафора заполнения, код %d.\n", errno);
    exit(errno);
  }

  sem_t* extractSemaphore;
  if ((extractSemaphore = sem_open(EXTRACT_SEM_NAME, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) {
    printf("Ошибка при открытии семафора извлечения, код %d.\n", errno);
    exit(errno);
  }

  sem_t* queueAccessSemaphore;
  if ((queueAccessSemaphore = sem_open(QUEUE_ACCESS_SEM_NAME, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) {
    printf("Ошибка при открытии семафора доступа к очереди, код %d.\n", errno);
    exit(errno);
  }

  while (isContinuing) {
    char ch = getchar();
    switch (ch) {
    case 'w': {
      // Создание потока для заполнения сообщений
      pthread_t threadId;
      if (pthread_create(&threadId, NULL, fillingMessages, NULL) != 0) {
        printf("Ошибка при создании потока заполнения.\n");
        isContinuing = 0;
      } else {
        pushStack(&stackFiller, threadId);  // Добавление потока в стек
      }
      break;
    }
    case 's':
      // Остановка последнего потока заполнения
      if (stackFiller != NULL) {
        pthread_kill(stackFiller->threadId, SIGUSR1);
        pthread_join(stackFiller->threadId, NULL);
        popStack(&stackFiller);
        isContinuing = 1;
      } else {
        printf("Заполнителей нет.\n");
      }
      break;
    case 'e': {
      // Создание потока для извлечения сообщений
      pthread_t threadId;
      if (pthread_create(&threadId, NULL, extractingMessages, NULL) != 0) {
        printf("Ошибка при создании потока извлечения.\n");
        isContinuing = 0;
      } else {
        pushStack(&stackExtractor, threadId);  // Добавление потока в стек
      }
      break;
    }
    case 'd':
      // Остановка последнего потока извлечения
      if (stackExtractor != NULL) {
        pthread_kill(stackExtractor->threadId, SIGUSR1);
        pthread_join(stackExtractor->threadId, NULL);
        popStack(&stackExtractor);
        isContinuing = 1;
      } else {
        printf("Извлекателей нет.\n");
      }
      break;
    case '+':
      MAX_MESSAGE_COUNT++;  // Увеличение максимального количества сообщений
      break;
    case '-':
      // Уменьшение максимального количества сообщений, если это возможно
      if (messageQueue->countAdded - messageQueue->countDeleted < MAX_MESSAGE_COUNT) {
        MAX_MESSAGE_COUNT--;
      }
      break;
    case 'q':
      // Завершение всех потоков заполнения и извлечения
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
      printf("Все заполнители и извлекатели завершены. Конец программы.\n");
      isContinuing = 0;
      break;
    }
  }

  // Закрытие и удаление семафоров
  sem_close(fillSemaphore);
  sem_unlink(FILL_SEM_NAME);
  sem_close(extractSemaphore);
  sem_unlink(EXTRACT_SEM_NAME);
  sem_close(queueAccessSemaphore);
  sem_unlink(QUEUE_ACCESS_SEM_NAME);

  return 0;
}