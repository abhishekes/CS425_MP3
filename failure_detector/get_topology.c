#include "get_topology.h"

extern struct Head_Node *server_topology;
extern struct Node *myself;
extern int topology_version;
char myIPs[10][16];
char myIP[16];
extern neighbourHeartbeat savedHeartbeat[NUM_HEARTBEAT_NEIGHBOURS]; 

RC_t get_topology() {	
	struct sockaddr_in master;
	struct Node* node;
	int mSocket, numIPs, nodeFound, i;
	payloadBuf *packet;
	RC_t rc;

	LOG(INFO,"Contacting admission control to join group %s", " ");
	
	if((mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("Unable to create socket. Dying.... Press any key to continue\n");
	        getchar();	
                //exit(0);
                return RC_FAILURE;
	}
        //printf("\nAfter socket\n");	
	memset(&master, 0, sizeof(master));
	master.sin_family 	= AF_INET;
	master.sin_addr.s_addr 	= inet_addr(ADMISSION_CONTACT_IP);
	master.sin_port		= htons(ADMISSION_CONTACT_PORT);

	//First get my IPAddress so that we can get the pointer to it
	//numIPs = getMyIPAddrs(myIPs);
	
	if((connect(mSocket, (struct sockaddr *)&master, sizeof(master))) < 0) {
		printf("Unable to connect with the Master. Dying...\nPress any key to continue");
	        getchar();
                return RC_FAILURE;
        }
		
	//Tell the master I want to join the topology
	
	sendTopologyJoinRequest(mSocket);
                
	rc = message_decode(mSocket,&packet);
	//printf("\nAfter message decode, packet : %lu\n", (long)packet);	

        processPacket(mSocket, packet);
	
	//process the incoming packet	
			
	
	//Set the topology version received from master.
	//Get a pointer to the node containing my IP. Set it to *myself.
	
	printf("\nIP = %s\n", myIP);
	nodeFound = 0;
	node = server_topology->node;
	do {
		
		if(!strcmp(node->IP, myIP)) {
		    nodeFound = 1;
		    printf("\nFound. Setting myself\n");
		    break;	
                }
		printf("Node IP = %s", node->IP);					
		node = node->next;
	
	}while(node != server_topology->node);
	
	if(nodeFound) {	
		myself = node;
		pthread_mutex_lock(&timestamp_mutex);
		strcpy(savedHeartbeat[0].ipAddr, myself->prev->IP);
		pthread_mutex_unlock(&timestamp_mutex);
	} else { 
		LOG(ERROR, "My IP not found in server topology%s\n","");
		printf("Error. My IP node found in server_topology. Press any key to continue");
                getchar();
		return RC_FAILURE;	
	}
	//By this time, topology is formed and is present in the server_topology pointer
	//Any changes in the topology will cause a change in the version number of the topology
	
	LOG(INFO,"Received topology from admission %s ", "contact");	
	return RC_SUCCESS;
}
