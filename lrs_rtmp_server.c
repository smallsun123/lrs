#include "lrs_rtmp_server.h"

static int abort_request;

static void sig_handler(int sig)
{
    abort_request = 1;
	return;
}

static void sig_regester(void)
{
    signal(SIGINT , sigterm_handler); /* Interrupt (ANSI).    */
    signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */
	return;
}


int main(int argc, void **argv){

    sig_regester();

    struct LRS_MASTER *master = lrs_master_alloc();

    for(;;){
        if(abort_request == 1){
            break;
        }
    }

    lrs_master_free(master);

    return 0;
}
