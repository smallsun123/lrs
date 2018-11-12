#ifndef __LRS_BUFFER__H__
#define __LRS_BUFFER__H__

#define LRS_DEFAULT_SIZE 1024*1024*3

typedef void (*lrs_buf_realloc)(struct LRS_BUFFER*);
typedef void (*lrs_buf_reset)(struct LRS_BUFFER*);
typedef static int (*lrs_buf_lenght)(struct LRS_BUFFER *buffer);

typedef struct LRS_BUFFER{
    int size;
    char *data;
    char *start;
    char *end;
    char *pread;
    char *pwrite;
    char *pfree;

    lrs_buf_realloc realloc;
    lrs_buf_reset reset;
    lrs_buf_lenght lenght;
}LRS_BUFFER;

struct LRS_BUFFER* lrs_buffer_alloc(int size = 0);
void lrs_buffer_free(struct LRS_BUFFER *buffer);


#endif
