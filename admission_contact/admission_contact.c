#include "admission_contact.h"
#define MAX_NUM_IPS 10
#define IP_FILE_PATH "./IPs.txt"
extern char myIP[16];
extern pthread_mutex_t node_list_mutex;
//Iterate over IPs file and get the topology



RC_t parseIPsFile(char maybeIPs[MAX_NUM_IPS][16], uint16_t *numIPs) {

	FILE *fp = NULL;
	char line[50];
	int i = 0;
	fp = fopen(IP_FILE_PATH, "r");

	*numIPs = 0;
	while(fgets(line, sizeof(line), fp) != NULL) {
		sscanf(line, "IP: %s", maybeIPs[i]);
		i++;
	}

	fclose(fp);
	
	*numIPs = i;
	
	return RC_SUCCESS;
}

RC_t writeIPsToFile(char *ipList, uint16_t numIPs) {

	FILE *fp = NULL;
	fp = fopen(IP_FILE_PATH, "w");
	if(fp == NULL) printf("\nNULL\n");
	while((numIPs > 0) && (fp != NULL)) {
		fprintf(fp, "IP: %s\n", ipList);
		numIPs --;
		ipList += 16;
	}

	fclose(fp);
	
	return RC_SUCCESS;
}

int iHadCrashed() {
	return (!system("ls IPs.txt"));
}

RC_t getTopologyFromSomeNode() {
	char maybeIPs[MAX_NUM_IPS][16];
	uint16_t numIPs = 0;
	int i, ret, sock;
	RC_t rc;
	
	struct sockaddr_in nodeAddress;
	
	if(parseIPsFile(maybeIPs, &numIPs) != RC_SUCCESS) {
		printf("\nError parsing IPs.txt\n");
		return RC_FAILURE;
	}
	
	i = 0; 
	while(i < numIPs) {		
		memset(&nodeAddress, 0, sizeof(nodeAddress));
		nodeAddress.sin_family        = AF_INET;
		nodeAddress.sin_addr.s_addr   = inet_addr(maybeIPs[i]);
		nodeAddress.sin_port          = htons(TCP_LISTEN_PORT);
	
		if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			//LOG(ERROR, "IP : %s Unable to create TCP Socket. Dying...\n", IP);
			printf("IP : %s Unable to create TCP Socket. Dying...\n");
        	}

		if((ret = connect(sock, (struct sockaddr *) &nodeAddress,   sizeof(nodeAddress))) < 0) { 
   	     		printf("Unable to connect to %s. Trying next IP", maybeIPs[i]);
			i++;
			continue;
		}else if(ret == 0){ 
			printf("\nConnected to %s. Now requesting for topology\n",maybeIPs[i]);
			break;
		}
	}

	if(i == numIPs) {
		printf("\nNo more IPs left to try. And none of them connected.\n. Treating this as a fresh run\n");
		return RC_FAILURE;
	}
	
	topologyRequestPayload *topoPayload = calloc(1, sizeof(topologyRequestPayload));
	int size = sizeof(topologyRequestPayload);
	
        sendPayload(sock, MSG_TOPOLOGY_REQUEST, topoPayload, size);
	
	payloadBuf *packet;
	rc = message_decode(sock, &packet);
	if(rc = RC_SUCCESS) processPacket(sock, packet);
	
	
	char *buf, *ipAddrList, *ptr, *ptr1;
	int nodes_to_send_to, total_nodes;
	buf = ipAddrList = NULL;
	nodes_to_send_to = total_nodes = 0;
	pthread_mutex_lock(&node_list_mutex);
   	populate_ipAddrList(&buf, &total_nodes, &ipAddrList, &nodes_to_send_to, 0);
	pthread_mutex_unlock(&node_list_mutex);
	ptr = buf + 4;
        ptr1 = buf; 
        for (i =0; i < total_nodes; i++) {
        	memcpy(ptr1, ptr, 16);
                ptr1 += 16;
                ptr += 20;
        } 	
	writeIPsToFile(buf, total_nodes);
	
	free(topoPayload);	
	close(sock);
	return RC_SUCCESS;
}

struct Head_Node *server_topology;	
int listenSocket = 0;

int main() {

	int  listenSocket, connectSocket, socketFlags, ret, clientSize;
	struct sockaddr_in myAddress, clientAddress;
	int i,j, bytes, numBytes, pid;

	server_topology = NULL;
        strcpy(myIP, ADMISSION_CONTACT_IP);
	//server_topology = (struct Head_Node*)calloc(1, sizeof(struct Head_Node));
        log_init();
	        
        payloadBuf *packet;
	int rc;
	
	log_init();
        getIpAddr();
	
	if( iHadCrashed() && ( getTopologyFromSomeNode() != RC_SUCCESS )) {
	}	
	
	clientSize = sizeof(clientAddress);

	//Create a listening socket..
	if((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error creating server socket. Dying ...\n");
		LOG(ERROR, "Problem at admission contact in booting up. %s\n", "Exiting");
                return 0;
	}
	printf("Socket Created\n");
	
	int so_reuseaddr = 1;
	setsockopt(listenSocket,SOL_SOCKET,SO_REUSEADDR, &so_reuseaddr, sizeof so_reuseaddr);
	
	//Init the sockaddr structure..
	memset(&myAddress, 0, sizeof(myAddress));
	myAddress.sin_family	  = AF_INET;
	myAddress.sin_addr.s_addr = INADDR_ANY;
	myAddress.sin_port	  = htons(MY_PORT);
	
	
	//Now bind the socket..
	if((bind(listenSocket, (struct sockaddr *)&myAddress, sizeof(myAddress))) < 0) {
		printf("Error binding socket. Dying...\n");		
		return 0;
	}

	printf("Socket bound to IP address. \n");
	
	//Listen on the socket for incoming connections..	
	if((listen(listenSocket, 10)) < 0) {
		printf("Error listening on socket. Dying...\n");
		return 0;
	}
	
	printf("******************************** Ready to admit new nodes into the network **************************\n");
	for(;;) {
		if ((connectSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &clientSize)) < 0) {
			printf("Error accepting connection. Dying...\n");
			return 0;
		} 
		
		printf("Before printing\n");
		
		
		printf("\nClient %d.%d.%d.%d connected\n", 
				(clientAddress.sin_addr.s_addr & 0xFF),  
				(clientAddress.sin_addr.s_addr & 0xFF00) >> 8,  
				(clientAddress.sin_addr.s_addr & 0xFF0000) >> 16,  
				(clientAddress.sin_addr.s_addr & 0xFF000000) >> 24 
			);
		
		//A client has connected.
		//I need to tell the topology to the client
		
		printf("Calling Message Decode\n");	
		rc = message_decode(connectSocket, &packet);
		printf("After message decode\n");
		if(rc == RC_SUCCESS)
			processPacket(connectSocket, packet);	
		
		char *buf, *ipAddrList, *ptr, *ptr1;
		int nodes_to_send_to, total_nodes;
		buf = ipAddrList = NULL;
		nodes_to_send_to = total_nodes = 0;
		
		pthread_mutex_lock(&node_list_mutex);
   		populate_ipAddrList(&buf, &total_nodes, &ipAddrList, &nodes_to_send_to, 0);
		pthread_mutex_unlock(&node_list_mutex);
	        
		ptr = buf + 4;
                ptr1 = buf; 
                for (i =0; i < total_nodes; i++) {
                    memcpy(ptr1, ptr, 16);
                    ptr1 += 16;
                    ptr += 20;
                } 	
		writeIPsToFile(buf, total_nodes);
		
		close(connectSocket);
		if(buf) free(buf);
		if(ipAddrList) free(ipAddrList);
		
	}	
}

