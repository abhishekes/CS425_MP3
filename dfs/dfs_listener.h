#ifndef __DFS_LISTENER__
#define __DFS_LISTENER__

#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "../commons/message_type.h"
#include "../commons/debug.h"
#include "../logging/log.h"
#include "../commons/list.h"
#include "../logging/log.h"
#include "../ui/ui.h"

#define ADMISSION_CONTACT_IP "192.168.2.7"	//This is not decided yet
#define ADMISSION_CONTACT_PORT 2000		//This is not decided yet
#define DFS_LISTEN_PORT 28000

void* dfs_listener(void*);

#endif