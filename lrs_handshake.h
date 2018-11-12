#ifndef __LRS_HAND_SHAKE__H__
#define __LRS_HAND_SHAKE__H__

#include <time.h>

#define LRS_HANDSHAKE_VERSION0 0x03
#define LRS_HANDSHAKE_VERSION1 0x06

#define RTMP_HANDSHAKE_C0S0_SIZE 4
#define RTMP_HANDSHAKE_C1S1C2S2_SIZE 1536

#define RTMP_SIG_SIZE 1536

typedef int (*lrs_handshake_C0C1_s2c)(struct LRS_HAND_SHAKE*, const char*, int, char*, int);
typedef int (*lrs_handshake_C2_s2c)(struct LRS_HAND_SHAKE*, const char*, int);

typedef int (*lrs_handshake_C0C1_c2s)(struct LRS_HAND_SHAKE*, char*, int);
typedef int (*lrs_handshake_S0S1S2_c2s)(struct LRS_HAND_SHAKE*, const char*, int, char*, int);


typedef struct LRS_HAND_SHAKE{
    uint8_t C0[RTMP_HANDSHAKE_C0S0_SIZE];
    uint8_t C1[RTMP_HANDSHAKE_C1S1C2S2_SIZE];

    uint8_t S0[RTMP_HANDSHAKE_C0S0_SIZE];
    uint8_t S1[RTMP_HANDSHAKE_C1S1C2S2_SIZE];

    lrs_handshake_C0C1_s2c lrs_C0C1_s2c;
    lrs_handshake_C2_s2c lrs_C2_s2c;
    lrs_handshake_C0C1_c2s lrs_C0C1_c2s;
    lrs_handshake_S0S1S2_c2s lrs_S0S1S2_c2s;
    
}LRS_HAND_SHAKE;

struct LRS_HAND_SHAKE* lrs_handshake_alloc();
void* lrs_handshake_free(struct LRS_HAND_SHAKE *handshake);

#endif
