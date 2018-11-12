#ifndef __LRS_RTMP__H__
#define __LRS_RTMP__H__

#define LRS_RTMP_DEFAULT_CHUNK_SIZE 128
#define LRS_RTMP_MAX_HEADER 16

#define LRS_RTMP_SET_CHUNK_SIZE 0x01
#define LRS_RTMP_ABORT_MESSAGE 0x02
#define LRS_RTMP_ACKNOWLEDGEMENT 0x03
#define LRS_RTMP_CONTROL 0x04
#define LRS_RTMP_SET_WINDOW_ACK_SIZE 0x05
#define LRS_RTMP_SET_PEER_BW 0x06

#define LRS_RTMP_AUDIO 0x08
#define LRS_RTMP_VIDEO 0x09

#define LRS_RTMP_FLEX_STREAM_SEND 0x0f
#define LRS_RTMP_FLEX_SHARED_OBJECT 0x10
#define LRS_RTMP_FLEX_MESSAGE 0x11
#define LRS_RTMP_INFO 0x12
#define LRS_RTMP_SHARED_OBJECT 0x13
#define LRS_RTMP_INVOKE 0x14

#define LRS_RTMP_FLASH_VIDEO 0x16


#define LRS_RTMP_EXTENDED_TIMESTAMP_LENGTH 4

typedef int (*lrs_parse_basic)(struct LRS_RTMP_BASIC_HEADER*, struct LRS_BUFFER*);
typedef int (*lrs_encoder_basic)(struct LRS_RTMP_BASIC_HEADER*, struct LRS_BUFFER*);

typedef int (*lrs_parse_message)(struct LRS_RTMP_MESSAGE_HEADER*, struct LRS_BUFFER*, LRS_RTMP_BASIC_FMT_TYPE, struct LRS_AMF*);
typedef int (*lrs_encoder_message)(struct LRS_RTMP_MESSAGE_HEADER*, struct LRS_BUFFER*, struct LRS_AMF*);

typedef int (*lrs_parse_extended_timestamp)(struct LRS_RTMP_EXTENED_TIMESTAMP*, struct LRS_BUFFER*, struct LRS_AMF*);
typedef int (*lrs_encoder_extended_timestamp)(struct LRS_RTMP_EXTENED_TIMESTAMP*, struct LRS_BUFFER*, struct LRS_AMF*);

typedef int (*lrs_set_chunksize)(struct LRS_RTMP_CHUNK*, int);

typedef int (*lrs_parse_chunk_header)(struct LRS_RTMP_CHUNK*, struct LRS_BUFFER*, struct LRS_AMF*);
typedef int (*lrs_parse_chunk_data)(struct LRS_RTMP_CHUNK*, struct LRS_BUFFER*);
typedef int (*lrs_encoder_chunk_header)(struct LRS_RTMP_CHUNK*, struct LRS_BUFFER*, struct LRS_AMF*);

typedef int (*lrs_encoder_chunkdata)(struct LRS_RTMP_CHUNK*, char*, int, int*);

typedef int (*lrs_parse_packet)(struct LRS_RTMP_PACKET*, struct LRS_BUFFER*);
typedef int (*lrs_encoder_packet)(struct LRS_RTMP_PACKET*, struct LRS_BUFFER*);


typedef enum {
    LRS_RTMP_BASIC_FMT3 = 0,
    LRS_RTMP_BASIC_FMT2 = 3,
    LRS_RTMP_BASIC_FMT1 = 7,
    LRS_RTMP_BASIC_FMT0 = 11
}LRS_RTMP_BASIC_FMT_TYPE;

typedef enum {
    LRS_RTMP_BASIC_CSID0 = 1,
    LRS_RTMP_BASIC_CSID1,
    LRS_RTMP_BASIC_CSID2
}LRS_RTMP_CSID_TYPE;

typedef struct LRS_RTMP_BASIC_HEADER{
    int fmt;
    int csid;
    LRS_RTMP_CSID_TYPE csidtype;

    lrs_parse_basic parser;
    lrs_encoder_basic encoder;
}LRS_RTMP_BASIC_HEADER;

struct LRS_RTMP_BASIC_HEADER* lrs_basic_header_alloc();
void lrs_basic_header_free(struct LRS_RTMP_BASIC_HEADER *header);


typedef struct LRS_RTMP_MESSAGE_HEADER{
    int timestamp;
    int messagelenght;
    int messagetype;
    int messagestreamid;

    LRS_RTMP_BASIC_FMT_TYPE type;

    int extendedtimestamp;
    
    lrs_parse_message parser;
    lrs_encoder_message encoder;
}LRS_RTMP_MESSAGE_HEADER;

struct LRS_RTMP_MESSAGE_HEADER* lrs_message_header_alloc();
void lrs_message_header_free(struct LRS_RTMP_MESSAGE_HEADER *header);


typedef struct LRS_RTMP_EXTENED_TIMESTAMP{
    int timestamp;

    lrs_parse_extended_timestamp parser;
    lrs_encoder_extended_timestamp encoder;
}LRS_RTMP_EXTENED_TIMESTAMP;

struct LRS_RTMP_EXTENED_TIMESTAMP* lrs_rtmp_extended_timestamp_alloc();
void lrs_rtmp_extended_timestamp_free(struct LRS_RTMP_EXTENED_TIMESTAMP* header);


typedef struct LRS_RTMP_CHUNK{
    struct list_head 	node;
    
    LRS_RTMP_BASIC_HEADER *basicheader;
    LRS_RTMP_MESSAGE_HEADER *messageheader;
    LRS_RTMP_EXTENED_TIMESTAMP *extendedtimestamp;

    int chunksize;
    char *start;
    char *chunkdata;
    char *curpos;

    struct LRS_AMF *amf;

    lrs_set_chunksize setchunksize;
    lrs_parse_chunk_header parserheader;
    lrs_parse_chunk_data parserdata;
    lrs_encoder_chunk_header encoderheader;
}LRS_RTMP_CHUNK;

struct LRS_RTMP_CHUNK* lrs_rtmp_chunk_alloc();
void lrs_rtmp_chunk_free(struct LRS_RTMP_CHUNK *chunk);


typedef struct LRS_RTMP_PACKET{
    struct list_head 	node;
    struct list_head 	list;

    int in_chunksize;
    int out_chunksize;

    int type;

    lrs_parse_packet parser;
    lrs_encoder_packet encoder;
}LRS_RTMP_PACKET;

struct LRS_RTMP_PACKET* lrs_rtmp_packet_alloc();
void lrs_rtmp_packet_free(struct LRS_RTMP_PACKET *pkt);

int lrs_create_protocol_ctrl_msg(struct LRS_RTMP_CHUNK *chunk, char *buf, int data, int *size);
int lrs_create_user_ctrl_msg(struct LRS_RTMP_CHUNK *chunk, char *buf, int data, int *size);
int lrs_create_result_msg(struct LRS_RTMP_CHUNK *chunk, char *buf, char *data, int *size);
int lrs_create_onstatus_msg(struct LRS_RTMP_CHUNK *chunk, char *buf, char *data, int *size);


#endif
