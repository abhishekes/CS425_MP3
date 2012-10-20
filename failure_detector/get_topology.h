#ifndef __GET_TOPOLOGY__
#define __GET_TOPOLOGY__

#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/poll.h>
#include "../logging/log.h"
#include "../commons/message_type.h"
#include "../commons/list.h"
#include "failure_detector.h"

RC_t form_topology();

#endif
