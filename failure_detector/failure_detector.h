#ifndef __FAILURE_DETECTOR__
#define __FAILURE_DETECTOR__

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
#include "join_topology.h"
#include "heartbeat_send.h"
#include "heartbeat_receive.h"
#include "topology_update.h"
#include "../logging/log.h"
#include "../ui/ui.h"

#define ADMISSION_CONTACT_IP "192.17.11.33"	//This is not decided yet
#define ADMISSION_CONTACT_PORT 2000		//This is not decided yet
#define TCP_LISTEN_PORT 18000

int node_init();
RC_t node_exit(); 

#endif
