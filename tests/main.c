#include "test_logger.h"
#include "test_udp.h"
#include "test_trtp.h"
#include "test_utils.h"
#include "test_command.h"
#include "test_integration.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("==================== test cases ==================\n");
    printf("\n=========== Utils tests\n");
    test_write2First();
    test_writeThird();
    test_write5Last();
    test_read2First();
    test_readThird();
    test_read5Last();
    test_encode_decodeFrame();

    printf("\n=========== UDP tests\n");
    test_udp_open();
    test_udp_send_receive();

    printf("\n=========== TRTP tests\n");
    
    printf("\n=========== Command tests\n");
    test_init_sender();
    test_init_receiver();

    printf("\n=========== Integration tests\n");
    test_send_receive_file();

    printf("\n=========== logger tests\n");
    test_initStat();
    test_printStat();
    test_initStdout(); 
    test_initLog();

    return 0;
}