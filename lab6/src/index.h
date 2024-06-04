#include <stdint.h>

typedef struct index_s
{
    double   time_mark;
    uint64_t recno;
} index_record_t;

typedef struct index_hdr_s
{
    uint64_t        records;
    index_record_t* idx;
} index_hdr_t;