#include "lrs_buffer.h"

struct LRS_BUFFER* lrs_buffer_alloc(int size)
{
    LRS_BUFFER *buffer = NULL;
    buffer = (LRS_BUFFER*)malloc(sizeof(struct LRS_BUFFER));
    if(!buffer){
        printf("lrs_buffer_alloc buffer failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(buffer, 0, sizeof(struct LRS_BUFFER));

    if(!size)
        buffer->size = LRS_DEFAULT_SIZE;
    buffer->data = (char*)malloc(sizeof(char)*buffer->size);
    if(!buffer->data){
        printf("lrs_buffer_alloc data failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    
    buffer->start = buffer->data;
    buffer->end = buffer->start + buffer->size;
    buffer->pfree = buffer->pread = buffer->pwrite = buffer->start;

    buffer->realloc = lrs_buffer_realloc;
    buffer->reset = lrs_buffer_reset;
    buffer->lenght = lrs_buffer_get_lenght;

    return buffer;
}

void lrs_buffer_free(struct LRS_BUFFER *buffer)
{
    if(buffer){
        if(buffer->data){
            free(buffer->data);
            buffer->data = NULL;
        }
        buffer->end = buffer->start = buffer->pfree = buffer->pread = buffer->pwrite = NULL;

        free(buffer);
        buffer = NULL;
    }
    return;
}

void lrs_buffer_realloc(struct LRS_BUFFER *buffer)
{
    char *newdata = NULL;
    int oldsize = buffer->size;
    buffer->size *= 2;
    newdata = (char*)malloc(sizeof(char)*buffer->size);
    if(!newdata){
        printf("lrs_buffer_realloc newdata failed! error msg : %s\n", strerror(errno));
        return NULL;
    }

    memcpy(newdata, buffer->data, oldsize);
    free(buffer->data);
    
    buffer->data = newdata;
    buffer->start = buffer->data;
    buffer->end = buffer->start + buffer->size;
    
    return;
}

void lrs_buffer_reset(struct LRS_BUFFER *buffer)
{
    if(buffer){
        if(buffer->data){
            buffer->pfree = buffer->pread = buffer->pwrite = buffer->start = buffer->data;
        }
    }

    return;
}

static int lrs_buffer_get_lenght(struct LRS_BUFFER *buffer)
{
    int ret;
    if(buffer->pread == buffer->pwrite){
        return 0;
    }
    
    if(buffer->pread < buffer->pwrite){
        ret = buffer->pwrite - buffer->pread;
    } else {
        ret = buffer->end - buffer->read;
        ret += buffer->pwrite - buffer->start;
    }
    return ret;
}

