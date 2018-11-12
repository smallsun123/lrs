#include "lrs_handshake.h"

static uint32_t lrs_get_time()
{
    static int clk_tck;
    struct tms t;
    if (!clk_tck) 
        clk_tck = sysconf(_SC_CLK_TCK);
	return times(&t) * 1000 / clk_tck;
}

static int lrs_handshake_C0C1_s2c(struct LRS_HAND_SHAKE *handshake, const char *buf, int size, char *outbuf, int outsize)
{
    int ret, i;
    char *pr = buf, *pw = outbuf;
    
    if(size < RTMP_HANDSHAKE_C0S0_SIZE){
        printf("C0 data lenght wrong\n");
        ret = -1;
        goto fail;
    }

    //C0 == S0
    memcpy(handshake->C0, pr, RTMP_HANDSHAKE_C0S0_SIZE);
    uint32_t version = AMF_Decode_Int32(pr);
    if(version != 0x03){
        printf("lrs_server_handshake_to_client failed! version : %d\n", version);
        ret = -1;
        goto fail;
    }
    
    if(size != RTMP_HANDSHAKE_C0S0_SIZE + RTMP_HANDSHAKE_C1S1C2S2_SIZE){
        printf("C1 data lenght wrong\n");
        ret = -1;
        goto fail;
    }
    pr += RTMP_HANDSHAKE_C0S0_SIZE;

    //create handshake S0S1S2
    //S0 == C0
    pw = AMF_Encode_Int32(pw, outbuf+outsize, version);
    //S1
    pw = AMF_Encode_Int32(pw, outbuf+outsize, lrs_get_time());
    pw = AMF_Encode_Int32(pw, outbuf+outsize, 0x00);
    for (i = 0; i < RTMP_HANDSHAKE_C1S1C2S2_SIZE - 8; i++)
		*pw++ = (char)(rand() % 256);
    memcpy(handshake->S1, outbuf+RTMP_HANDSHAKE_C0S0_SIZE, RTMP_HANDSHAKE_C1S1C2S2_SIZE);
    //S2 == C1
    memcpy(pw, pr, RTMP_HANDSHAKE_C1S1C2S2_SIZE);

    return 0;

fail:
    return ret;
}

static int lrs_handshake_C2_s2c(struct LRS_HAND_SHAKE *handshake, const char *buf, int size)
{
    int ret;
    if((ret = memcmp(handshake->S1, buf, RTMP_HANDSHAKE_C1S1C2S2_SIZE)) != 0){
        ret = -1;
        printf("C2 check failed\n");
    }
    return ret;
}


static int lrs_handshake_C0C1_c2s(struct LRS_HAND_SHAKE *handshake, char *outbuf, int outsize)
{
    int ret, i;
    char *pw = outbuf;
    //C0
    pw = AMF_Encode_Int32(pw, outbuf+outsize, 0x03);
    memcpy(handshake->C0, outbuf, RTMP_HANDSHAKE_C0S0_SIZE);
    //C1
    pw = AMF_Encode_Int32(pw, outbuf+outsize, lrs_get_time());
    pw = AMF_Encode_Int32(pw, outbuf+outsize, 0x00);
    for (i = 0; i < RTMP_HANDSHAKE_C1S1C2S2_SIZE - 8; i++)
		*pw++ = (char)(rand() % 256);
    memcpy(handshake->C1, outbuf+RTMP_HANDSHAKE_C0S0_SIZE, RTMP_HANDSHAKE_C1S1C2S2_SIZE);
    
    return 0;
}

static int lrs_handshake_S0S1S2_c2s(struct LRS_HAND_SHAKE *handshake, const char *buf, int size, char *outbuf, int outsize)
{
    int ret, i;
    char *pr = buf, *pw = outbuf;
    if(size < RTMP_HANDSHAKE_C0S0_SIZE + RTMP_HANDSHAKE_C1S1C2S2_SIZE*2){
        printf("S0S1S2 data lenght wrong\n");
        ret = -1;
        goto fail;
    }

    //S0
    if(memcmp(handshake->C0, buf, RTMP_HANDSHAKE_C0S0_SIZE) != 0){
        ret = -1;
        printf("S0 version wrong\n");
        goto fail;
    }
    pr += RTMP_HANDSHAKE_C0S0_SIZE;

    //S1
    memcpy(handshake->S1, pr, RTMP_HANDSHAKE_C1S1C2S2_SIZE);
    pr += RTMP_HANDSHAKE_C1S1C2S2_SIZE;

    //S2
    if(memcmp(handshake->C1, pr, RTMP_HANDSHAKE_C1S1C2S2_SIZE) != 0){
        ret = -1;
        printf("C1 != S2, check wrong\n");
        goto fail;
    }

    //C2
    memcpy(outbuf, handshake->S1, RTMP_HANDSHAKE_C1S1C2S2_SIZE);
    
    return 0;
fail:
    return ret;
}


struct LRS_HAND_SHAKE* lrs_handshake_alloc()
{
    struct LRS_HAND_SHAKE* handshake = NULL;
    handshake = (struct LRS_HAND_SHAKE*)malloc(sizeof(struct LRS_HAND_SHAKE));
    if(!handshake){
        printf("lrs_handshake_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(handshake, 0, sizeof(struct LRS_HAND_SHAKE));

    handshake->lrs_C0C1_s2c = lrs_handshake_C0C1_s2c;
    handshake->lrs_C2_s2c = lrs_handshake_C2_s2c;
    handshake->lrs_C0C1_c2s = lrs_handshake_C0C1_c2s;
    handshake->lrs_S0S1S2_c2s = lrs_handshake_S0S1S2_c2s;

    return handshake;
}

void* lrs_handshake_free(struct LRS_HAND_SHAKE *handshake)
{
    if(handshake){
        handshake->lrs_C0C1_s2c = NULL;
        handshake->lrs_C2_s2c = NULL;
        handshake->lrs_C0C1_c2s = NULL;
        handshake->lrs_S0S1S2_c2s = NULL;
        free(handshake);
        handshake = NULL;
    }
    return NULL;
}

