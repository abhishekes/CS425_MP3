#ifndef __TOPOLOGY_UPDATE__
#define __TOPOLOGY_UPDATE__

#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../commons/list.h"
#include "../logging/log.h"
#include "../commons/message_type.h"
#include "failure_detector.h"

void* topology_update(void*);

#endif
