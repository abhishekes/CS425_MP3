#include "dgrep_server.h"

/*********************************************************
** This program runs as a daemon. 
** The log query application connects to this daemon  
** to execute grep queries. 
***********************************************************/



int main()
{
	int listenSocket, connectSocket, socketFlags, ret, clientSize;
	struct sockaddr_in myAddress, clientAddress;
	int i,j, bytes, numBytes, pid;
	payloadBuf *packet;
	int rc;
	

	//Create a listening socket..
	if((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error creating server socket. Dying ...\n");
		return 0;
	}
	
	//Init the sockaddr structure..
	memset(&myAddress, 0, sizeof(myAddress));
	myAddress.sin_family	  = AF_INET;
	myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddress.sin_port	  = htons(MY_PORT);
	
	//Now bind the socket..
	if((bind(listenSocket, (struct sockaddr *)&myAddress, sizeof(myAddress))) < 0) {
		printf("Error binding socket. Dying...\n");		
		return 0;
	}
	
	//Listen on the socket for incoming connections..	
	if((listen(listenSocket, LISTEN_Q)) < 0) {
		printf("Error listening on socket. Dying...\n");
		return 0;
	}
	
	for(;;) {
		if ((connectSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &clientSize)) < 0) {
			printf("Error accepting connection. Dying...\n");
			return 0;
		} 
		
		printf("\nClient %d.%d.%d.%d connected\n", 
				(clientAddress.sin_addr.s_addr & 0xFF),  
				(clientAddress.sin_addr.s_addr & 0xFF00) >> 8,  
				(clientAddress.sin_addr.s_addr & 0xFF0000) >> 16,  
				(clientAddress.sin_addr.s_addr & 0xFF000000) >> 24 
			);
		
		if ((pid = fork()) > 0) {
			close(connectSocket);                                    //parent has no use for the connection.
		}
		else if(pid == 0) {                                              //Child does the processing
			do {	
				
                                rc = message_decode(connectSocket, &packet);     //Call decode function to deode the recieved packet
				if (rc == RC_SUCCESS)
					processPacket(connectSocket,  packet);		

			} while(rc == RC_SUCCESS);
			return 0;
		}
		else {
			printf("Fork failed");
			return 0;
		}	
				
	}	
}




