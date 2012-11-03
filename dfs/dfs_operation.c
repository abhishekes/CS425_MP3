#include "dfs_operation.h"

fileInfoPayload fileInfo ;
RC_t dfs_file_transfer (fileOperation op, char *localFileName, char *destinationFileName)
{
	FILE *inputFile = NULL;
	inputFile = fopen(localFileName, "r");
	thread_data *my_data;
	pthread_t thread;
	if (!inputFile) {
	    	 printf("\nCould not find source file");
	    	 return RC_INPUT_FILE_NOT_FOUND;
	}else {
	    	 fclose(inputFile);
	}
    if (server_topology && server_topology->node ) {
	    my_data = calloc(1, sizeof(thread_data) + sizeof(fileOperationRequestPayload));
        (*my_data).payload = calloc(1, sizeof(fileOperationRequestPayload));

        memcpy((*my_data).ip, server_topology->node->IP, 16);

        (*my_data).payload_size = sizeof(fileOperationRequestPayload);
        (*my_data).msg_type = MSG_FILE_OPERATION_REQUEST;
        (*my_data).flags = WAIT_FOR_RESPONSE;
        memcpy((*my_data).payload, payloadBuf, sizeof(fileOperationRequestPayload));
        pthread_create(&thread, NULL, send_node_update_payload, (my_data));
        pthread_join(thread, NULL);
    }
	if (my_data->status == RC_SUCCESS) {
    //Sachin's function to do splitting of files.
		//Call all send file wrappers
	}
    return my_data->status;

}

RC_t dfs_file_receive(char *localFileName, char *remoteFileName)
{
	FILE *inputFile = NULL;
	thread_data *my_data;
	pthread_t thread;

    if (server_topology && server_topology->node ) {
	    my_data = calloc(1, sizeof(thread_data) + sizeof(fileInfoPayload));
        (*my_data).payload = calloc(1, sizeof(fileInfoPayload));

        memcpy((*my_data).ip, server_topology->node->IP, 16);

        (*my_data).payload_size = sizeof(fileInfo);
        (*my_data).msg_type = MSG_FILE_INFO;
        (*my_data).flags = WAIT_FOR_RESPONSE;
        memcpy((*my_data).payload, payloadBuf, sizeof(fileInfoPayload));
        pthread_create(&thread, NULL, send_node_update_payload, (my_data));
        pthread_join(thread, NULL);
    }
	if (my_data->status == RC_SUCCESS) {
        //Sachin's function to do splitting of files.
		//Check if you got the reply
		if (my_data->return_data != NULL) {
        //Loop through all splits, creating an entry for each
			//receiveFileWrapper();
		}
	}
    return my_data->status;

}
RC_t receiveFileWrapper(void *tdata) {
	RC_t rc;
	int i = 0;
	int sock;
	dfs_thread_info *dfs_data = (dfs_thread_info *)(tdata);
    struct sockaddr_in nodeAddress;
    char (*IP)[16] = dfs_data->ip;
    void *data = NULL;
    payloadBuf *packet;

    for ( i = 0; i < dfs_data->numOfAddresses ; i++, IP++) {
        if ( createConnection(&nodeAddress, IP, &sock) == RC_SUCCESS) {
            if (sendFileRequest(sock, fileName) == RC_SUCCESS) {
                while ((rc = message_decode(sock, &packet)) == RC_SUCCESS) {
                        processPacket(socket, packet, data);

                }
            }

        }
    }
    if ( i < dfs_data->numOfAddresses) {
	    dfs_data->rc = RC_SUCCESS;
         }else {
        dfs_data->rc = rc;
    }
    return rc;
}



RC_t sendFileWrapper(void *tdata ) {
	FILE *fp;
    dfs_thread_info *dfs_data = (dfs_thread_info *)(tdata);
    struct sockaddr_in nodeAddress;
    char (*IP)[16] = dfs_data->ip;
    int sock;
    int rc = RC_FAILURE;
    struct sockaddr_in nodeAddress;
    int sock;
    int i = 0;
    fp = open(dfs_data->fileName, "r");
	if (fp == NULL) {
	    return RC_FILE_NOT_FOUND;
	}
    for ( i = 0; i < dfs_data->numOfAddresses ; i++, IP++) {
        if ( createConnection(&nodeAddress, IP, &sock) == RC_SUCCESS) {
            if (sendFile(sock, dfs_data->fileName, dfs_data->destFileName) == RC_SUCCESS) {
        	    break;
            }

        }
    }
    if ( i < dfs_data->numOfAddresses) {
       dfs_data->rc = RC_SUCCESS;

    }else {
       dfs_data->rc = rc;
    }
    return rc;
}

RC_t createConnection(struct sockaddr_in *nodeAddress, char *IP, int *sock) {

	memset(nodeAddress, 0, sizeof(nodeAddress));
    nodeAddress.sin_family        = AF_INET;
    nodeAddress.sin_addr.s_addr   = inet_addr(IP);
    nodeAddress.sin_port          = htons(TCP_LISTEN_PORT);
    if((*sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        LOG(ERROR, "IP : %s Unable to create TCP Socket. Dying...\n", IP);
        printf("IP : %s Unable to create TCP Socket. Dying...\n", IP);
        return RC_SOCKET_CREATION_FAILED;

    }

    if((connect(*sock, (struct sockaddr *) nodeAddress,   sizeof(*nodeAddress))) < 0) {
            /*LOG(ERROR, "IP : %s Unable to connect with server %s . Dying ...\n", IP);*/
            return RC_SOCKET_CONNECT_FAILED;

    }
    return RC_SUCCESS;
}
