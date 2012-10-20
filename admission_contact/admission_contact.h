#ifndef __MASTER__
#define __MASTER__

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include "../commons/list.h"
#include "../commons/parse_file.h"
#include "../logging/log.h"
#include "../failure_detector/topology_update.h"
#define MY_PORT  2000
#endif
