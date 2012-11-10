#include "dfs_listener.h"

#define NUM_WORKER_THREADS 30

void* handle_request(void* t);

void* dfs_listener(void* t) {

	int connectSocket, socketFlags, ret, listenSocket;
	socklen_t clientSize;
	struct sockaddr_in myAddress, clientAddress;
	int i,j, bytes, numBytes, pid;
	void *tData;
	pthread_t workers[NUM_WORKER_THREADS];

	payloadBuf *packet;
	int rc;
	clientSize = sizeof(clientAddress);
        if(listenSocket) {
            close(listenSocket);
        }
	//Create a listening socket..
	if((listenSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error creating server socket. Dying .... Press any key to continue\n");
        getchar();
		return 0;
	}

	int so_reuseaddr = 1;
	setsockopt(listenSocket,SOL_SOCKET,SO_REUSEADDR, &so_reuseaddr, sizeof so_reuseaddr);

	//Init the sockaddr structure..
	memset(&myAddress, 0, sizeof(myAddress));
	myAddress.sin_family	 	= AF_INET;
	myAddress.sin_addr.s_addr 	= INADDR_ANY;
	myAddress.sin_port	  	= htons(DFS_LISTEN_PORT);

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

		while( i >= NUM_WORKER_THREADS && !pthread_cancel(workers[i])) {
			i = (i + 1) % NUM_WORKER_THREADS;
		}

		pthread_create(&workers[i], handle_request, NULL, &connectSocket);
		i = (i + 1) % NUM_WORKER_THREADS;
	}

	pthread_exit(NULL);
}

void* handle_request(void *tData) {
	RC_t rc;
	payloadBuf *packet;
	int connectSocket = *((int*)tData);

	do {
		rc = message_decode(connectSocket, &packet);
		if (rc != RC_SUCCESS) {
			break;
		}
		processPacket(connectSocket, packet, NULL);
	}while(1);

	pthread_testcancel();

	pthread_exit(NULL);
}
