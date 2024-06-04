#include "index.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) 
{
    if(argc < 1) 
    {
        printf("No args!\n");
        exit(-1);
    }

    FILE* fd = NULL;
    if ((fd = fopen(argv[1], "rb")) == NULL)
    {
        printf("Open file error\n");
        exit(-1);
    }

    index_hdr_t* data = (index_hdr_t*)malloc(sizeof(index_hdr_t));

    fread(&data->records, sizeof(uint64_t), 1, fd);

    data->idx = (index_record_t*)malloc(data->records * sizeof(index_record_t));

    fread(data->idx, sizeof(index_record_t), data->records, fd);

    fclose(fd);

    printf("Records: %d\n", data->records);

    for (int i = 0; i < data->records; i++)
        printf("%d : %lf\n", i + 1, data->idx[i].time_mark);

    return 0;
}