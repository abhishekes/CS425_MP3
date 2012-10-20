#ifndef __DGREP_CLIENT_H__
#define __DGREP_CLIENT_H__

#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<string.h>

//ELLORES2
#include"../commons/message_encode.h"
#include"../commons/message_decode.h"
#include"../commons/message_type.h"
#include"../commons/debug.h"

#define SERVER_PORT 1100
#define MAX_SEND_SIZE 1000
#define MAX_THREADS 4

#define NO_OF_CONNECTIONS 4

struct threadData {
	int threadID;
	char* ip;
        char command[1000];
        int  statusFlag;
        #define SUCCESS                          0x01
        #define FAILURE                          0x02
        #define SOCKET_CREATION_FAILURE          0x04
        #define SOCKET_CONNECT_FAILURE           0x08 
        #define FILE_TRANSFER_FAILURE            0x10
        #define FILE_EXECUTION_FAILURE           0x20
        #define RESPONSE_RECEIVE_FAILURE         0x40
        
        #define SOCKET_WRITE_FAILURE             0x80
        #define SOCKET_READ_FAILURE              0x100  
 
};

#endif
