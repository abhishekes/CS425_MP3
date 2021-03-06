#ifndef __NODE_SEND_INFO_H__
#define __NODE_SEND_INFO_H__
#include <arpa/inet.h>
#include "../commons/message_type.h"
#include <stdlib.h>
#include <string.h>
#include "../failure_detector/failure_detector.h"
typedef struct {
    char ip[16];
    int payload_size;
    int msg_type;
    char *payload;
    uint16_t flags;
    #define WAIT_FOR_RESPONSE     0x01
    #define RETURN_VALUE_REQUIRED 0x02
    #define FREE_PAYLOAD          0x04
    #define USE_DFS_PORT          0x08
    RC_t status;
    void *return_data;

}thread_data; 

void send_node_update_payload(void *tdata);
extern pthread_mutex_t timestamp_mutex;
#endif
