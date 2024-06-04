#define _POSIX_C_SOURCE 200112L

#include "index.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <threads.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

typedef struct thrd_params_s
{
    index_record_t* map;
    int             size_block;
    int             id;
} thrd_params_t;

typedef struct thrd_data_s
{
    int         size_block;
    int         threads_num;
    const char* file_name;
} thrd_data_t;

int buffer_size, blocks_num;

pthread_barrier_t barrier;                     
pthread_mutex_t   mutex;
index_record_t*   cur_map_pos;

int compare(const void* first, const void* second) 
{           
    if (((index_record_t*)first)->time_mark < ((index_record_t*)second)->time_mark) return -1;
    else if (((index_record_t*)first)->time_mark > ((index_record_t*)second)->time_mark) return 1;
    else return 0;
}

void* sort_routine(void* thread) 
{
    thrd_params_t* params = (thrd_params_t*)thread;  

    pthread_barrier_wait(&barrier);

    while(true) 
    {
        pthread_mutex_lock(&mutex);

        if(cur_map_pos < params->map + buffer_size) 
        {
            index_record_t* temp_map_ptr = cur_map_pos;
            cur_map_pos += params->size_block;

            pthread_mutex_unlock(&mutex);

            qsort(temp_map_ptr, params->size_block, sizeof(index_record_t), compare);
        } 
        else 
        {
            pthread_mutex_unlock(&mutex);

            pthread_barrier_wait(&barrier);
            break;
        }
    }

    int merge_step = 2;
    
    while(merge_step <= blocks_num) 
    {                       
        pthread_barrier_wait(&barrier);

        cur_map_pos = params->map;

        while (cur_map_pos < params->map + buffer_size) 
        {
            pthread_mutex_lock(&mutex);

            if(cur_map_pos < params->map + buffer_size) 
            {
                index_record_t* temp_map_ptr = cur_map_pos;
                cur_map_pos += merge_step * params->size_block;

                pthread_mutex_unlock(&mutex);

                int buf_size = (merge_step / 2) * params->size_block;

                index_record_t* left = (index_record_t*) malloc(buf_size * sizeof(index_record_t));
                memcpy(left, temp_map_ptr, (merge_step / 2) * params->size_block * sizeof(index_record_t));

                index_record_t* right = (index_record_t*) malloc(buf_size * sizeof(index_record_t));
                memcpy(right, temp_map_ptr + (merge_step / 2) * params->size_block,(merge_step / 2) * params->size_block * sizeof(index_record_t));

                int i = 0, j = 0;
                while (i < buf_size && j < buf_size) 
                {
                    if (left[i].time_mark < right[j].time_mark) 
                    {
                        temp_map_ptr[i + j].time_mark = left[i].time_mark;
                        temp_map_ptr[i + j].recno = left[i].recno;
                        i++;
                    } 
                    else 
                    {
                        temp_map_ptr[i + j].time_mark = right[j].time_mark;
                        temp_map_ptr[i + j].recno = right[j].recno;
                        j++;
                    }
                }
                if (i == buf_size) 
                {
                    while (j > buf_size) 
                    {
                        temp_map_ptr[i + j].time_mark = right[j].time_mark;
                        temp_map_ptr[i + j].recno = right[j].recno;
                        j++;
                    }
                }
                if (j == buf_size) 
                {
                    while (i < buf_size) 
                    {
                        temp_map_ptr[i + j].time_mark = left[i].time_mark;
                        temp_map_ptr[i + j].recno = left[i].recno;
                        i++;
                    }
                }
            } 
            else 
            {
                pthread_mutex_unlock(&mutex);
                break;
            }
        }

        merge_step *= 2;
    }

    pthread_mutex_unlock(&mutex);
    pthread_barrier_wait(&barrier);
}

void* start_routine(void* thrd_data) 
{
    thrd_data_t* data = (thrd_data_t*)thrd_data;      

    FILE* f_ptr = NULL;      
    if ((f_ptr = fopen(data->file_name, "rb+")) == NULL)
    {
        printf("Error while open file.\n");
        exit(-1);
    }

    fseek(f_ptr, 0, SEEK_END);
    size_t file_size = ftell(f_ptr);
    fseek(f_ptr, 0, SEEK_SET);

    int fd = fileno(f_ptr);

    void* map_ptr;                                
    if((map_ptr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) 
    {
        printf("Error mapping!");
        exit(-2);
    }

    map_ptr += sizeof(uint64_t);                     
    cur_map_pos = (index_record_t*)map_ptr;

    pthread_t threads_pull[data->threads_num - 1];

    for(int i = 1; i < data->threads_num; i++) 
    {
        thrd_params_t* params = (thrd_params_t*)malloc(sizeof(thrd_params_t));
        params->size_block    = data->size_block;
        params->id            = i;
        params->map           = (index_record_t*)map_ptr;

        pthread_create(&threads_pull[i - 1], NULL, sort_routine, params);
    }

    thrd_params_t* params = (thrd_params_t*)malloc(sizeof(thrd_params_t));
    params->size_block    = data->size_block;
    params->id            = 0;
    params->map           = (index_record_t*)map_ptr;

    sort_routine(params);

    for(int i = 1; i < data->threads_num; i++)
        pthread_join(threads_pull[i - 1], NULL);   

    munmap(map_ptr, file_size);

    fclose(f_ptr);
}

int main(int argc, char* argv[]) 
{
    if(argc < 1) 
    {
        printf("No args!\n");
        exit(-1);
    }

    buffer_size = atoi(argv[1]);
    blocks_num = atoi(argv[2]);

    pthread_barrier_init(&barrier, NULL, atoi(argv[3]));
    pthread_mutex_init(&mutex, NULL);

    thrd_data_t* thrd_data = (thrd_data_t*)malloc(sizeof(thrd_data_t));
    thrd_data->size_block  = buffer_size / blocks_num; 
    thrd_data->threads_num = atoi(argv[3]);
    thrd_data->file_name   = argv[4];

    pthread_t thread_id; 
    pthread_create(&thread_id, NULL, start_routine, thrd_data);

    pthread_join(thread_id, NULL);

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);

    return 0;
}