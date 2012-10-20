#ifndef __NODE_PROCESS_PAYLOAD_H__
#define __NODE_PROCESS_PAYLOAD_H__
#include "message_type.h"
#include "list.h"
#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include "../node/node_send_info.h"
#include "../logging/log.h"
#include "../failure_detector/failure_detector.h"
#include <time.h>
#define NUM_HEARTBEAT_NEIGHBOURS 2

typedef struct {
    char   ipAddr[16];
    time_t latestTimeStamp;
}neighbourHeartbeat;


//extern neigbourHeartbeat savedHeartbeat[NUM_HEARTBEAT_NEIGHBOURS]; 
void processHeartbeat(char *ipAddr);
void sendTopologyResponse(int socket, int numOfNodes, char *buf);
void processHeartbeatPayload(heartbeatPayload *payload); 
void processNodeAddDeletePayload(addDeleteNodePayload *payload, int payload_size) ;
void sendTopologyJoinRequest(int socket);
void sendAddNodePayload(char *ipAddrList, int numOfNodesToSend, char ID[ID_SIZE] );
void sendDeleteNodePayload(char *ipAddrList, int numOfNodesToSend, char ID[ID_SIZE], int ttl, char flags );
void populate_ipAddrList(char **ipList, int *noOfNodes, char **nodesToSendTo, int *numOfNodesToSendTo, int flag);
#endif
