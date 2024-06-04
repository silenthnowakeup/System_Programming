#include "index.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char* argv[]) 
{
    if (argc < 1) 
    {
        printf("No args!\n");
        exit(-1);
    }
    
    srand(time(NULL));

    index_hdr_t header;

    header.records = atoi(argv[1]); 

    if (header.records % 256 != 0)
    {
        printf("Wrong size, it should be multiple by 256\n");
        exit(-1);
    }

    header.idx = (index_record_t*)malloc(header.records * sizeof(index_record_t));

    for (int i = 0; i < header.records; i++)
    {
        header.idx[i].recno = i + 1; // first index
        header.idx[i].time_mark = 15020 + rand() % (60449 - 15020 + 1);
        header.idx[i].time_mark +=
            0.5 * ((rand() % 24) * 60 * 60 + (rand() % 60) * 60 + rand() % 60) / (12 * 60 * 60);
    }

    FILE* fd = NULL;
    if ((fd = fopen(argv[2], "wb")) == NULL)
    {
        printf("Open file error\n");
        exit(-1);
    }

    fwrite(&header.records, sizeof(header.records), 1, fd);
    fwrite(header.idx, sizeof(index_record_t), header.records, fd); 
    
    fclose(fd);

    free(header.idx);

    return 0;
}