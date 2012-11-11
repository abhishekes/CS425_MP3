#include "topology_update.h"

extern struct Head_Node *server_topology;
extern struct Node* myself;
int listenSocket = 0;
#define NUM_WORKERS 15

pthread_attr_t attr;


void* handle_topo_request(void* t);
void* topology_update(void* t) {

	int connectSocket, socketFlags, ret;
	socklen_t clientSize;
	struct sockaddr_in myAddress, clientAddress;
	int i ,j, bytes, numBytes, pid;
	pthread_t workers[NUM_WORKERS];
	listener_thread_data connectedSocket[NUM_WORKERS] = {0};
	payloadBuf *packet;
	int rc;
	i = 0;
	clientSize = sizeof(clientAddress);
        if(listenSocket) {
            close(listenSocket);
        }
	//Create a listening socket..
	if((listenSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error creating server socket. Dying .... Press any ket to continue\n");
                getchar();
		return 0;
	}
	
	int so_reuseaddr = 1;
	setsockopt(listenSocket,SOL_SOCKET,SO_REUSEADDR, &so_reuseaddr, sizeof so_reuseaddr);
	
	//Init the sockaddr structure..
	memset(&myAddress, 0, sizeof(myAddress));
	myAddress.sin_family	  = AF_INET;
	myAddress.sin_addr.s_addr = INADDR_ANY;
	myAddress.sin_port	  = htons(TCP_LISTEN_PORT);
	
	//Now bind the socket..
	if((bind(listenSocket, (struct sockaddr *)&myAddress, sizeof(myAddress))) < 0) {
		printf("Error binding socket. Dying.... Press any key to continue\n");	
                getchar();	
		return 0;
	}
	
	//Listen on the socket for incoming connections..	
	if((listen(listenSocket, 10)) < 0) {
		printf("Error listening on socket. Dying...Press any key to continue\n");
                getchar();
		return 0;
	}
	
	for(;;) {
		pthread_testcancel();
		if ((connectSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &clientSize)) < 0) {
			printf("Error accepting connection. Dying...\nPress any key to continue\n");
                        getchar();
			return 0;
		} 
		
		DEBUG(("\n**********************************Client %d.%d.%d.%d connected******************************************\n", 
				(clientAddress.sin_addr.s_addr & 0xFF),  
				(clientAddress.sin_addr.s_addr & 0xFF00) >> 8,  
				(clientAddress.sin_addr.s_addr & 0xFF0000) >> 16,  
				(clientAddress.sin_addr.s_addr & 0xFF000000) >> 24 
			));

		//A client has connected. 
		//It will send me updates regarding the topology - either someone has joined or someone has left

/*		while( connectedSocket[i].state) {
				i = (i + 1) % NUM_WORKERS;
		}

		connectedSocket[i].socket	= connectSocket;
		connectedSocket[i].state = 1;
		pthread_create(&workers[i], NULL, handle_topo_request, &connectedSocket[i]);
		i = (i + 1) % NUM_WORKERS;
*/
		do {
			rc = message_decode(connectSocket, &packet);
			if (rc != RC_SUCCESS) {
				break;
			}
			processPacket(connectSocket, packet, NULL);
		}while(1);

		//printf("\n\nDid I succeed?  rc = %d\n\n", rc);

		//for(i = 0; i < packet->length; i++) {
		//	printf("   ****  %d, ", *(packet + i));
		//}	
		
		//close(connectSocket);
	}

	pthread_exit(NULL);
}

void *handle_topo_request(void* t) {
	//int connectSocket = *((int*)t);
	payloadBuf *packet;
	listener_thread_data *thread_info = (listener_thread_data *)(t);
	RC_t rc;

	do {
		rc = message_decode(thread_info->socket, &packet);
		if (rc != RC_SUCCESS) {
			break;
		}
		processPacket(thread_info->socket, packet, NULL);
	}while(1);

	thread_info->state = 0;
	pthread_exit(NULL);
}
