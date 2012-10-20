#ifndef __DGREP_SERVER__
#define __DGREP_SERVER__

#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<poll.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include"../commons/message_decode.h"
#include"../commons/packet_process.h"
#include"../commons/debug.h"

#define MY_PORT 1100
#define LISTEN_Q 10
#define MAX_FDS 25

#define READ_TYPE 	0x01
#define READ_LENGTH 	0x02
#define READ_PAYLOAD	0x03

#endif //__DGREP_SERVER__
