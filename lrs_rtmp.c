#include "lrs_rtmp.h"


static int lrs_parse_rtmp_basic_header1(struct LRS_RTMP_BASIC_HEADER *header, const char *buf, int size, int *parsedsize)
{
    int ret = 0, csid = 0, lcsid, hcsid;
    char *p = buf;
    if(size < 1){
        printf("lrs_parse_rtmp_basic_header wrong basic0 size!\n");
        return -1;
    }
    header->fmt = *p >> 6 & 0x03;
    
    csid = *p & 0x3f;
    p++;

    if(csid > 0 && csid < 64){
        header->csid = csid;
        header->csidtype = LRS_RTMP_BASIC_CSID0;
    } else if (csid == 0){
        if(size < 1){
            printf("lrs_parse_rtmp_basic_header wrong basic1 size!\n");
            return -1;
        }
        header->csid = *p++;
        header->csidtype = LRS_RTMP_BASIC_CSID1;
    } else if (csid == 1 ){
        if(size < 1){
            printf("lrs_parse_rtmp_basic_header wrong basic2 size!\n");
            return -1;
        }
        lcsid = *p++;
        hcsid = *p++;
        header->csid = hcsid << 8 | lcsid;
        header->csidtype = LRS_RTMP_BASIC_CSID2;
    }

    *parsedsize = p - buf;

    return 0;
}

static int lrs_parse_rtmp_basic_header(struct LRS_RTMP_BASIC_HEADER *header, struct LRS_BUFFER 
    *buffer, struct LRS_SOCKET *sock)
{
    int ret = 0, csid = 0, lcsid, hcsid;
    if(buffer->lenght(buffer) < 1 && sock->read() ){
        printf("lrs_parse_rtmp_basic_header wrong basic0 size!\n");
        return -1;
    }
    
    header->fmt = *buffer->pread >> 6 & 0x03;
    csid = *buffer->pread & 0x3f;
    buffer->pread++;
    if(buffer->pread == buffer->end){
        buffer->pread = buffer->start;
    }

    if(csid > 0 && csid < 64){
        header->csid = csid;
        header->csidtype = LRS_RTMP_BASIC_CSID0;
    } else if (csid == 0){
        if(buffer->lenght(buffer) < 1){
            printf("lrs_parse_rtmp_basic_header wrong basic1 size!\n");
            return -1;
        }
        header->csid = *buffer->pread++;
        if(buffer->pread == buffer->end){
            buffer->pread = buffer->start;
        }
        header->csidtype = LRS_RTMP_BASIC_CSID1;
    } else if (csid == 1 ){
        if(buffer->lenght(buffer) < 1){
            printf("lrs_parse_rtmp_basic_header wrong basic2 size!\n");
            return -1;
        }
        lcsid = *buffer->pread++;
        if(buffer->pread == buffer->end){
            buffer->pread = buffer->start;
        }
        hcsid = *buffer->pread++;
        if(buffer->pread == buffer->end){
            buffer->pread = buffer->start;
        }
        header->csid = hcsid << 8 | lcsid;
        header->csidtype = LRS_RTMP_BASIC_CSID2;
    }

    return 0;
}


static int lrs_encoder_rtmp_basic_header1(struct LRS_RTMP_BASIC_HEADER *header, char *buf, int *encodersize)
{
    char *p = buf;

    switch(header->csidtype){
        case LRS_RTMP_BASIC_CSID0 : {
            *p++ = header->fmt << 6 | header->csid;
        }break;
        case LRS_RTMP_BASIC_CSID1 : {
            *p++ = header->fmt << 6 | 0x00;
            *p++ = header->csid;
        }break;
        case LRS_RTMP_BASIC_CSID2 : {
            *p++ = header->fmt << 6 | 0x01;
            *p++ = header->csid & 0xff;
            *p++ = header->csid >> 8 & 0xff;
        }break;
        default : break;
    };

    *encodersize = p - buf;

    return 0;
}

static int lrs_encoder_rtmp_basic_header(struct LRS_RTMP_BASIC_HEADER *header, struct LRS_BUFFER *buffer)
{
    switch(header->csidtype){
        case LRS_RTMP_BASIC_CSID0 : {
            *buffer->pwrite++ = header->fmt << 6 | header->csid;
            if(buffer->pwrite == buffer->end){
                buffer->pwrite = buffer->start;
            }
        }break;
        case LRS_RTMP_BASIC_CSID1 : {
            *buffer->pwrite++ = header->fmt << 6 | 0x00;
            if(buffer->pwrite == buffer->end){
                buffer->pwrite = buffer->start;
            }
            *buffer->pwrite++ = header->csid;
            if(buffer->pwrite == buffer->end){
                buffer->pwrite = buffer->start;
            }
        }break;
        case LRS_RTMP_BASIC_CSID2 : {
            *buffer->pwrite++ = header->fmt << 6 | 0x01;
            if(buffer->pwrite == buffer->end){
                buffer->pwrite = buffer->start;
            }
            *buffer->pwrite++ = header->csid & 0xff;
            if(buffer->pwrite == buffer->end){
                buffer->pwrite = buffer->start;
            }
            *buffer->pwrite++ = header->csid >> 8 & 0xff;
            if(buffer->pwrite == buffer->end){
                buffer->pwrite = buffer->start;
            }
        }break;
        default : break;
    };

    return 0;
}

struct LRS_RTMP_BASIC_HEADER* lrs_basic_header_alloc()
{
    struct LRS_RTMP_BASIC_HEADER* header = NULL;
    header = (struct LRS_RTMP_BASIC_HEADER*)malloc(sizeof(struct LRS_RTMP_BASIC_HEADER));
    if(!header){
        printf("lrs_basic_header_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(header, 0, sizeof(struct LRS_RTMP_BASIC_HEADER));

    header->parser = lrs_parse_rtmp_basic_header;
    header->encoder = lrs_encoder_rtmp_basic_header;

    return header;
}

void lrs_basic_header_free(struct LRS_RTMP_BASIC_HEADER *header)
{
    if(header){
        header->parser = NULL;
        header->encoder = NULL;
        free(header);
        header = NULL;
    }
    return;
}

static int lrs_rtmp_parse_message_header1(struct LRS_RTMP_MESSAGE_HEADER* header, const 
    char *buf, int size, int *parsedsize, LRS_RTMP_BASIC_FMT_TYPE type)
{
    char *p = buf;
    header->type = type;
    if(type == LRS_RTMP_BASIC_FMT3){
        return 0;
    }

    switch(type){
    case LRS_RTMP_BASIC_FMT0: {
        if(size < 11){
            printf("lrs_rtmp_parse_message_header wrong fmt0 size!\n");
            return -1;
        }
        header->timestamp = AMF_Decode_Int24(p);
        p+=3;
        if(header->timestamp == 0xffffff){
            header->extendedtimestamp = 1;
        }
        header->messagelenght = AMF_Decode_Int24(p);
        p+=3;
        header->messagetype = *p++;
        header->messagestreamid = AMF_Decode_Int32(p);
        p+=4;
    }break;
    case LRS_RTMP_BASIC_FMT1: {
        if(size < 7){
            printf("lrs_rtmp_parse_message_header wrong fmt1 size!\n");
            return -1;
        }
        header->timestamp = AMF_Decode_Int24(p);
        p+=3;
        header->messagelenght = AMF_Decode_Int24(p);
        p+=3;
        header->messagetype = *p++;
    }break;
    case LRS_RTMP_BASIC_FMT2: {
        if(size < 3){
            printf("lrs_rtmp_parse_message_header wrong fmt2 size!\n");
            return -1;
        }
        header->timestamp = AMF_Decode_Int24(p);
        p+=3;
    }break;
    default : break;
    }

    *parsedsize = p - buf;

    return 0;
}

static int lrs_rtmp_parse_message_header(struct LRS_RTMP_MESSAGE_HEADER* header, 
    struct LRS_BUFFER *buffer, LRS_RTMP_BASIC_FMT_TYPE type, struct LRS_AMF *amf)
{
    header->type = type;
    if(type == LRS_RTMP_BASIC_FMT3){
        return 0;
    }

    switch(type){
    case LRS_RTMP_BASIC_FMT0: {
        if(buffer->lenght(buffer) < 11){
            printf("lrs_rtmp_parse_message_header wrong fmt0 size!\n");
            return -1;
        }
        header->timestamp = amf->dint24(buffer);
        if(header->timestamp == 0xffffff){
            header->extendedtimestamp = 1;
        }
        header->messagelenght = amf->dint24(buffer);
        header->messagetype = *buffer->pread++;
        if(buffer->pread == buffer->end){
            buffer->pread = buffer->start;
        }
        header->messagestreamid = amf->dint32(buffer);
    }break;
    case LRS_RTMP_BASIC_FMT1: {
        if(buffer->lenght(buffer) < 7){
            printf("lrs_rtmp_parse_message_header wrong fmt1 size!\n");
            return -1;
        }
        header->timestamp = amf->dint24(buffer);
        header->messagelenght = amf->dint24(buffer);
        header->messagetype = *buffer->pread++;
        if(buffer->pread == buffer->end){
            buffer->pread = buffer->start;
        }
    }break;
    case LRS_RTMP_BASIC_FMT2: {
        if(buffer->lenght(buffer) < 3){
            printf("lrs_rtmp_parse_message_header wrong fmt2 size!\n");
            return -1;
        }
        header->timestamp = amf->dint24(buffer);
    }break;
    default : break;
    }
    return 0;
}


static int lrs_rtmp_encoder_message_header1(struct LRS_RTMP_MESSAGE_HEADER* header, char *buf, int *encodersize)
{
    char *p = buf;
    switch(header->type){
    case LRS_RTMP_BASIC_FMT0: {
        if(header->extendedtimestamp){
            *p++ = 0xff;
            *p++ = 0xff;
            *p++ = 0xff;
        } else {
            p = AMF_Encode_Int24(p, buf+LRS_RTMP_BASIC_FMT0, header->timestamp);
        }
        p = AMF_Encode_Int24(p, buf+LRS_RTMP_BASIC_FMT0, header->messagelenght);
        *p++ = header->messagetype;
        p = AMF_Encode_Int32(p, buf+LRS_RTMP_BASIC_FMT0, header->messagestreamid);
    }break;
    case LRS_RTMP_BASIC_FMT1: {
        p = AMF_Encode_Int24(p, buf+LRS_RTMP_BASIC_FMT0, header->timestamp);
        p = AMF_Encode_Int24(p, buf+LRS_RTMP_BASIC_FMT0, header->messagelenght);
        *p++ = header->messagetype;
    }break;
    case LRS_RTMP_BASIC_FMT2: {
        p = AMF_Encode_Int24(p, buf+LRS_RTMP_BASIC_FMT0, header->timestamp);
    }break;
    default : break;
    }

    *encodersize = p - buf;

    return 0;
}

static int lrs_rtmp_encoder_message_header(struct LRS_RTMP_MESSAGE_HEADER* header, struct LRS_BUFFER *buffer,
    struct LRS_AMF *amf)
{
    switch(header->type){
    case LRS_RTMP_BASIC_FMT0: {
        if(header->extendedtimestamp){
            *buffer->pwrite++ = 0xff;
            *buffer->pwrite++ = 0xff;
            *buffer->pwrite++ = 0xff;
        } else {
            amf->eint24(buffer, header->timestamp);
        }
        amf->eint24(buffer, header->messagelenght);
        *buffer->pwrite++ = header->messagetype;
        amf->eint24(buffer, header->messagestreamid);
    }break;
    case LRS_RTMP_BASIC_FMT1: {
        amf->eint24(buffer, header->timestamp);
        amf->eint24(buffer, header->messagelenght);
        *buffer->pwrite++ = header->messagetype;
    }break;
    case LRS_RTMP_BASIC_FMT2: {
        amf->eint24(buffer, header->timestamp);
    }break;
    default : break;
    }
    return 0;
}


struct LRS_RTMP_MESSAGE_HEADER* lrs_message_header_alloc()
{
    struct LRS_RTMP_MESSAGE_HEADER* header = NULL;
    header = (struct LRS_RTMP_MESSAGE_HEADER*)malloc(sizeof(struct LRS_RTMP_MESSAGE_HEADER));
    if(!header){
        printf("lrs_message_header_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(header, 0, sizeof(struct LRS_RTMP_MESSAGE_HEADER));

    header->parser = lrs_rtmp_parse_message_header;
    header->encoder = lrs_rtmp_encoder_message_header;

    return header;
}

void lrs_message_header_free(struct LRS_RTMP_MESSAGE_HEADER *header)
{
    if(header){
        header->parser = NULL;
        header->encoder = NULL;
        free(header);
        header = NULL;
    }
    return;
}

static int lrs_rtmp_parse_extended_timestamp1(struct LRS_RTMP_EXTENED_TIMESTAMP *header, 
    const char *buf, int size, int *parsedsize)
{
    char *p = buf;
    if(size < 4){
        printf("lrs_rtmp_parse_extended_timestamp wrong size\n");
        return -1;
    }

    header->timestamp = AMF_Decode_Int32(p);
    p+=4;

    parsedsize = p-buf;

    return 0;
}

static int lrs_rtmp_parse_extended_timestamp(struct LRS_RTMP_EXTENED_TIMESTAMP *header, 
    struct LRS_BUFFER *buffer, struct LRS_AMF *amf)
{
    if(buffer->lenght(buffer) < 4){
        printf("lrs_rtmp_parse_extended_timestamp wrong size\n");
        return -1;
    }

    header->timestamp = amf->dint32(buffer);

    return 0;
}


static int lrs_rtmp_encoder_extended_timestamp1(struct LRS_RTMP_EXTENED_TIMESTAMP *header, char *buf, int *encodersize)
{
    char *p = buf;
    p = AMF_Encode_Int32(p, buf+LRS_RTMP_EXTENDED_TIMESTAMP_LENGTH, header->timestamp);

    encodersize = p-buf;

    return 0;
}

static int lrs_rtmp_encoder_extended_timestamp(struct LRS_RTMP_EXTENED_TIMESTAMP *header, 
    struct LRS_BUFFER *buffer, struct LRS_AMF *amf)
{
    amf->eint32(buffer, header->timestamp);
    return 0;
}


struct LRS_RTMP_EXTENED_TIMESTAMP* lrs_rtmp_extended_timestamp_alloc()
{
    struct LRS_RTMP_EXTENED_TIMESTAMP *header = NULL;
    header = (struct LRS_RTMP_EXTENED_TIMESTAMP*)malloc(sizeof(struct LRS_RTMP_EXTENED_TIMESTAMP));
    if(!header){
        printf("lrs_rtmp_extended_timestamp_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(header, 0, sizeof(struct LRS_RTMP_EXTENED_TIMESTAMP));

    header->parser = lrs_rtmp_parse_extended_timestamp;
    header->encoder = lrs_rtmp_encoder_extended_timestamp;
}

void lrs_rtmp_extended_timestamp_free(struct LRS_RTMP_EXTENED_TIMESTAMP* header)
{
    if(header){
        header->parser = NULL;
        header->encoder = NULL;
        free(header);
        header = NULL;
    }
    return;
}


static int lrs_rtmp_set_chunksize(struct LRS_RTMP_CHUNK *chunk, int size)
{
    chunk->chunksize = size;
    return 0;
}

static int lrs_rtmp_parse_chunk_data(struct LRS_RTMP_CHUNK *chunk, const char *buf, int size, int *parsedsize)
{
    int parsed, left = size;
    LRS_RTMP_BASIC_FMT_TYPE type;
    char *p = buf;
    if(lrs_parse_rtmp_basic_header(chunk->basicheader, p, left, &parsed) < 0){
        return -1;
    }
    if(parsed <= 0){
        printf("lrs_rtmp_parse_chunk_data basic header wrong\n");
        return -1;
    }
    p+=parsed;
    left-=parsed;

    switch(chunk->basicheader->fmt){
    case 0: type = LRS_RTMP_BASIC_FMT0; break;
    case 1: type = LRS_RTMP_BASIC_FMT1; break;
    case 2: type = LRS_RTMP_BASIC_FMT2; break;
    case 3: type = LRS_RTMP_BASIC_FMT3; break;
    default : break;
    }
    if(type && lrs_rtmp_parse_message_header(chunk->messageheader, p, left, &parsed, type) < 0){
        return -1;
    }
    if(type <= 0){
        printf("lrs_rtmp_parse_chunk_data message header wrong\n");
        return -1;
    }
    p+=parsed;
    left-=parsed;

    if(chunk->messageheader->extendedtimestamp){
        if(lrs_rtmp_parse_extended_timestamp(chunk->extendedtimestamp, p, left, &parsed) < 0){
            return -1;
        }
        if(parsed <= 0){
            printf("lrs_rtmp_parse_chunk_data extended timestamp header wrong\n");
            return -1;
        }
        p+=parsed;
        left-=parsed;
    }

    if(left < chunk->chunksize){
        printf("lrs_rtmp_parse_chunk_data chunkdata wrong\n");
        return -1;
    }

    chunk->start = buf;
    chunk->chunkdata = chunk->curpos = p;

    *parsedsize = p-buf+chunk->chunksize;
    return 0;
}

static int lrs_rtmp_parse_chunk_header(struct LRS_RTMP_CHUNK *chunk, struct LRS_BUFFER *buffer, struct LRS_AMF *amf)
{
    char *start = buffer->pread;
    LRS_RTMP_BASIC_FMT_TYPE type;
    if(lrs_parse_rtmp_basic_header(chunk->basicheader, buffer) < 0){
        return -1;
    }

    switch(chunk->basicheader->fmt){
    case 0: type = LRS_RTMP_BASIC_FMT0; break;
    case 1: type = LRS_RTMP_BASIC_FMT1; break;
    case 2: type = LRS_RTMP_BASIC_FMT2; break;
    case 3: type = LRS_RTMP_BASIC_FMT3; break;
    default : break;
    }
    if(type && lrs_rtmp_parse_message_header(chunk->messageheader, buffer, type, amf) < 0){
        return -1;
    }
    
    if(chunk->messageheader->extendedtimestamp){
        if(lrs_rtmp_parse_extended_timestamp(chunk->extendedtimestamp, buffer, amf) < 0){
            return -1;
        }
    }

    chunk->start = start;

    return 0;
}

static int lrs_rtmp_parse_chunk_data(struct LRS_RTMP_CHUNK *chunk, struct LRS_BUFFER *buffer)
{
    if(buffer->lenght(buffer) < chunk->chunksize){
        printf("lrs_rtmp_parse_chunk_data chunkdata wrong\n");
        return -1;
    }
    
    chunk->chunkdata = chunk->curpos = buffer->pread;

    if(buffer->end - buffer->pread <= chunk->size){
        buffer->pread = buffer->start + buffer->end - buffer->pread;
    } else {
        buffer->pread += chunk->size;
    }

    return 0;
}


static int lrs_rtmp_encoder_chunk_header(struct LRS_RTMP_CHUNK *chunk, char *buf, int *encodersize)
{
    char *p = buf;
    int encodered;
    if(chunk->basicheader){
        lrs_encoder_rtmp_basic_header(chunk->basicheader, p, &encodered);
        p += encodered;
    }
    if(chunk->messageheader){
        lrs_rtmp_encoder_message_header(chunk->basicheader, p, &encodered);
        p += encodered;
    }
    if(chunk->extendedtimestamp){
        lrs_rtmp_encoder_extended_timestamp(chunk->basicheader, p, &encodered);
        p += encodered;
    }
    
    *encodersize = p-buf;
    
    return 0;
}

static int lrs_rtmp_encoder_chunk_header(struct LRS_RTMP_CHUNK *chunk, struct LRS_BUFFER *buffer, struct LRS_AMF *amf)
{
    if(chunk->basicheader){
        lrs_encoder_rtmp_basic_header(chunk->basicheader, buffer);
    }
    if(chunk->messageheader){
        lrs_rtmp_encoder_message_header(chunk->basicheader, buffer, amf);
    }
    if(chunk->extendedtimestamp){
        lrs_rtmp_encoder_extended_timestamp(chunk->basicheader, buffer, amf);
    }
    
    return 0;
}


struct LRS_RTMP_CHUNK* lrs_rtmp_chunk_alloc()
{
    struct LRS_RTMP_CHUNK *chunk = NULL;
    chunk = (struct LRS_RTMP_CHUNK*)malloc(sizeof(struct LRS_RTMP_CHUNK));
    if(!chunk){
        printf("lrs_rtmp_chunk_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(chunk, 0, sizeof(struct LRS_RTMP_CHUNK));

    chunk->basicheader = lrs_basic_header_alloc();
    if(!chunk->basicheader){
        free(chunk);
        return NULL;
    }

    chunk->messageheader = lrs_message_header_alloc();
    if(!chunk->messageheader){
        lrs_basic_header_free(chunk->basicheader);
        free(chunk);
        return NULL;
    }

    chunk->extendedtimestamp = lrs_rtmp_extended_timestamp_alloc();
    if(!chunk->extendedtimestamp){
        lrs_basic_header_free(chunk->basicheader);
        lrs_message_header_free(chunk->messageheader);
        free(chunk);
        return NULL;
    }

    INIT_LIST_HEAD(&chunk->node);

    chunk->chunkdata = chunk->curpos = chunk->start = NULL;

    chunk->setchunksize = lrs_rtmp_set_chunksize;
    chunk->parseheader = lrs_rtmp_parse_chunk_header;
    chunk->parserdata = lrs_rtmp_parse_chunk_data;
    chunk->encoderheader = lrs_rtmp_encoder_chunk_header;

    return chunk;
}

void lrs_rtmp_chunk_free(struct LRS_RTMP_CHUNK *chunk)
{
    if(chunk){
        if(chunk->basicheader){
            lrs_basic_header_free(chunk->basicheader);
        }
        if(chunk->messageheader){
            lrs_message_header_free(chunk->messageheader);
        }
        if(chunk->extendedtimestamp){
            lrs_rtmp_extended_timestamp_free(chunk->extendedtimestamp);
        }

        chunk->chunkdata = chunk->curpos = chunk->start = NULL;

        chunk->setchunksize = NULL;
        chunk->parser = NULL;
        chunk->encoder = NULL;

        free(chunk);
        chunk = NULL;
    }

    return;
}

static int lrs_rtmp_parse_packet_header(struct LRS_RTMP_PACKET *pkt, struct LRS_BUFFER *buf, struct LRS_SOCKET *sock)
{
    int left = 0, need = 0, ret = 0;
    struct LRS_RTMP_CHUNK *chunk = lrs_rtmp_chunk_alloc();
    chunk->parserheader(chunk, buf, chunk->amf);
    left = chunk->messageheader->messagelenght;
    list_add_tail(&chunk->node, &pkt->list);
    pkt->type = chunk->messageheader->messagetype;
    
    if(left <= pkt->in_chunksize){   //single chunk message
        chunk->setchunksize(chunk, left);
        chunk->parserdata(chunk, buf);
        return 0;
    } else {    // multi chunk message
        newck = chunk;
        while(left){
            newck->setchunksize(chunk, pkt->in_chunksize < left ? pkt->in_chunksize : left);
            if(newck->chunksize <= buf->lenght(buf)){
                newck->parserdata(chunk, buf);
                left -= chunk->chunksize;

                newck = lrs_rtmp_chunk_alloc();
                list_add_tail(&newck->node, &pkt->list);
            } else {
                need = chunk->chunksize - buf->lenght(buf);
                for(;;){
                    if((ret += sock->read(sock, buf, 0)) >= need){
                        break;
                    }
                }
            }
        }
    }

    return 0;
}

static int lrs_rtmp_parse_packet(struct LRS_RTMP_PACKET *pkt, struct LRS_BUFFER *buf, struct LRS_SOCKET *sock)
{
    int left = 0, need = 0, ret = 0;
    struct LRS_RTMP_CHUNK *newck = NULL;
    struct LRS_RTMP_CHUNK *chunk = lrs_rtmp_chunk_alloc();
    chunk->parserheader(chunk, buf, chunk->amf);
    left = chunk->messageheader->messagelenght;
    list_add_tail(&chunk->node, &pkt->list);
    pkt->type = chunk->messageheader->messagetype;
    
    if(left <= pkt->in_chunksize){   //single chunk message
        chunk->setchunksize(chunk, left);
        chunk->parserdata(chunk, buf);
        return 0;
    } else {    // multi chunk message
        newck = chunk;
        while(left){
            newck->setchunksize(chunk, pkt->in_chunksize < left ? pkt->in_chunksize : left);
            if(newck->chunksize <= buf->lenght(buf)){
                newck->parserdata(chunk, buf);
                left -= chunk->chunksize;

                newck = lrs_rtmp_chunk_alloc();
                list_add_tail(&newck->node, &pkt->list);
            } else {
                need = chunk->chunksize - buf->lenght(buf);
                for(;;){
                    if((ret += sock->read(sock, buf, 0)) >= need){
                        break;
                    }
                }
            }
        }
    }

    return 0;
}

static int lrs_rtmp_encoder_packet(struct LRS_RTMP_PACKET *pkt, struct LRS_BUFFER *buf)
{
    struct LRS_RTMP_CHUNK *chunk = NULL, *data;
    struct list_head *pos, *npos;
    list_for_each_safe(pos, npos, &pkt->list){
        chunk = list_entry(pos, struct LRS_RTMP_CHUNK, node);
        break;
    }
    if(!chunk){
        return 0;
    }
    
    if(pkt->out_chunksize >= chunk->messageheader->messagelenght){ //single message
        chunk->encoderheader(chunk, buf, chunk->amf);

        list_for_each_safe(pos, npos, &pkt->list){
            data = list_entry(pos, struct LRS_RTMP_CHUNK, node);
            memcpy(buf->pwrite, data->chunkdata, data->chunksize);
            buf->pwrite += data->chunksize;
        }
    }

    return 0;
}

struct LRS_RTMP_PACKET* lrs_rtmp_packet_alloc()
{
    struct LRS_RTMP_PACKET *pkt = NULL;
    pkt = (struct LRS_RTMP_PACKET*)malloc(sizeof(struct LRS_RTMP_PACKET));
    if(!pkt){
        printf("lrs_rtmp_packet_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(pkt, 0, sizeof(struct LRS_RTMP_PACKET));

    pkt->parser = lrs_rtmp_parse_packet;
    pkt->encoder = lrs_rtmp_encoder_packet;

    INIT_LIST_HEAD(&pkt->list);
    INIT_LIST_HEAD(&pkt->node);

    pkt->out_chunksize = 4096;

    return pkt;
}

void lrs_rtmp_packet_free(struct LRS_RTMP_PACKET *pkt)
{
    if(pkt){
        struct LRS_RTMP_CHUNK *chunk = NULL;
        struct list_head *pos, *npos;
        list_for_each_safe(pos, npos, &pkt->list){
            chunk = list_entry(pos, struct LRS_RTMP_CHUNK, node);
            
            list_del(pos);
            lrs_rtmp_chunk_free(chunk);
        }

        pkt->parser = NULL;
        pkt->encoder = NULL;
        
        free(pkt);
        pkt = NULL;
    }
    return;
}

