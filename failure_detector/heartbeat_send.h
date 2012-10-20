#ifndef __HEARTBEAT_SEND__
#define __HEARTBEAT_SEND__

#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../commons//message_type.h"
#include "../commons/list.h"
#include "../logging/log.h"
#include <time.h>
#include "heartbeat_receive.h"
#include "../commons/fdetect_payload_process.h"
void* heartbeat_send(void* t);
void heartbeat_send_cleanup(void *);
#endif 
