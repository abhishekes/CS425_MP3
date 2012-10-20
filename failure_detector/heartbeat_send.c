#include "heartbeat_send.h"

extern struct Head_Node *server_topology;
extern struct Node* myself;
extern int topology_version;
extern pthread_mutex_t node_list_mutex;
int sendToSocket;
extern char myIP[16];
void* heartbeat_send(void* t) {
	//I need to have access to the topology
	//I need to select the person I am supposed to send the heartbeats to
	//I need to send the heartbeat

	int my_version = 0;
	char sendToIP[16] = {0};
	struct sockaddr_in sendToAddr;
	heartbeatPayload *hbPayload = (heartbeatPayload*)malloc(sizeof(heartbeatPayload));
  	sendToSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);		

	while(1) {
		//if topology has changed, it might be that I need to now send to some other node.
		
		my_version = topology_version;
		pthread_mutex_lock(&node_list_mutex);
		if( (sendToIP[0] == NULL) || strcmp(sendToIP, myself->next->IP) ) {
			//I need to send to a new node now.
			
			strcpy(sendToIP, myself->next->IP);
			memset(&sendToAddr, 0, sizeof(sendToAddr));
			sendToAddr.sin_family 		= AF_INET;
			sendToAddr.sin_port   		= htons(HEARTBEAT_RECV_PORT);
			sendToAddr.sin_addr.s_addr	= inet_addr(sendToIP);
		}
		pthread_mutex_unlock(&node_list_mutex);		
		
		
		LOG(INFO, "Sending heartbeat to Node IP = %s\n", sendToIP);
		//send the heartbeat from here every 400 msec
		//TODO add a function to actually send the heartbeat here
		strcpy(hbPayload->ip_addr, myself->IP);
		//printf("hbPayloadIP = %s\n", hbPayload->ip_addr);
		//printf("\nSending Heartbeat\n");
		sendPayloadUDP(sendToSocket, MSG_HEARTBEAT, hbPayload, sizeof(heartbeatPayload), &sendToAddr);
		pthread_testcancel();
		usleep(400 * 1000); 	
	}
        pthread_exit(NULL);	
}
