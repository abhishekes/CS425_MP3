#include "join_topology.h"

extern struct Head_Node *server_topology;
extern struct Node* myself;
extern int topology_version;

RC_t join_topology() {
	//I've come here. This means I have gotten the topology from the admission_contact. 
	//Then I need to tell everyone that I've joined the topology, beginning with my neighbors
	
	struct Node* next, prev;	
	next = prev = NULL;
	
	LOG(INFO, "Joining the topology now %s", " ");
	
	if( myself != NULL ) { 		
		next = myself->next;
		prev = myself->prev;
	}
	
	if((next == NULL) || (prev == NULL)) {
		LOG(ERROR, "next or prev is NULL. It can't be. Check %s", "");
		return RC_FAILURE;
	}

	if( tell(next) /*that I've joined */ != RC_SUCCESS ) {
		//LOG(ERROR, "Could not tell next that I will send him heartbeats");
	}
	
	if(next != prev) {
		if(tell(prev) /* that I've joined */ != RC_SUCCESS) 
			//LOG(ERROR, "Could not tell prev that he has to send me ");
	} 
	else //LOG(INFO, "Next and Prev are the same. We are good");

	return RC_SUCCESS; //This means that we've joined the topology successfully
}

RC_t tell(struct Node* node_ptr) {
	
	struct sockaddr_in node_struct;
	struct Node* node;
	int mSocket, numIPs, nodeFound, i;
	char myIPs[10][16];

	if((mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Unable to create socket. Dying...\n");
		exit(0);
	}
	
	memset(&node_struct, 0, sizeof(node_struct));
	node_struct.sin_family 	= AF_INET;
	node_struct.sin_addr.s_addr 	= inet_addr(node_ptr->IP);
	node_struct.sin_port		= htons(NODE_LISTENER_PORT);
	
	if((connect(mSocket, (struct sockaddr *)&node_struct, sizeof(node_struct))) < 0) {
		//LOG(ERROR, "Unable to connect with the node_struct. Dying...\n");
	}
	
	//TODO
	//Tell the node of the topology	
		
	return RC_SUCCESS;	
}
