#ifndef __JOIN_TOPOLOGY__
#define __JOIN_TOPOLOGY__

#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/poll.h>
#include "../commons/list.h"
#include "../logging/log.h"

RC_t join_topology();
RC_t tell();

#endif
