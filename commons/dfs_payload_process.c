#include "dfs_payload_process.h"
#include "fdetect_payload_process.h"


extern struct Head_Node *server_topology;
extern struct Node *myself;
extern char myIP[16];
RC_t processFileInfoPayload(fileInfoPayload *infoPayload, void ** return_data) {
    RC_t rc;
    int size;
	LOG(DEBUG, "Received file info payload for %s", infoPayload->fileName);
	printf("Received file info payload for %s, flags = %0x ", infoPayload->fileName, infoPayload->flags);

	if (infoPayload->flags & FILE_INFO_UPDATE) {
    	 if (server_topology && server_topology->node) {
    		 if (myself) {
    			 if (server_topology->node != myself) {
    				 printf("Received file information even when node is not master");
    				 LOG(DEBUG, "Received file information for the file %s even when though I am not the master ", infoPayload->fileName);
    			 }else {
                     //Add information to database
    				 processFileInfoUpdatePayload(infoPayload);
    			 }
    		 }
    	 }
     }else if (infoPayload->flags & FILE_INFO_RESPONSE) {
     //Allocate memory and return the response
    	 LOG(DEBUG, "Received file information from master for file %s", infoPayload->fileName);
    	 LOG(DEBUG, " File Info for %s. Number of splits : %d, Number of replicas : %d", infoPayload->fileName,  infoPayload->noOfSplits, infoPayload->noOfReplicas);
    	 size = (sizeof(fileInfoPayload) + infoPayload->noOfSplits * infoPayload->noOfReplicas * 16);
         *return_data = (fileInfoPayload*)calloc(1, size); //Allocate memory for all the entries
         //addFileMetaInfo(infoPayload->fileName, 0, infoPayload->flags, infoPayload->noOfSplits, "0.0.0.0");

         memcpy(*return_data, infoPayload, size);
         rc = RC_SUCCESS;
     }
     return rc;
}

RC_t sendFileInfoRequest(int socket, char *fileName) {
    int rc = RC_FAILURE;
    fileOperationRequestPayload *requestBuf;
    if (fileName == NULL) {
    	return RC_INVALID_INPUT;
    }
    requestBuf = (fileOperationRequestPayload *)calloc(1, sizeof(fileOperationRequestPayload));
    strcpy(requestBuf->fileName, fileName);
    requestBuf->flags |= GET_FILE_REQUEST;
    rc = sendPayload(socket, MSG_FILE_OPERATION_REQUEST, requestBuf, sizeof(fileRequestPayload));
    free(requestBuf);
    return rc;
}

RC_t sendFileRequest(int socket, char *fileName) {
    int rc = RC_FAILURE;
    fileRequestPayload *requestBuf;
    if (fileName == NULL) {
    	return RC_INVALID_INPUT;
    }
    requestBuf = (fileRequestPayload *)calloc(1, sizeof(fileRequestPayload));
    strcpy(requestBuf->fileName, fileName);
    rc = sendPayload(socket, MSG_FILE_REQUEST, requestBuf, sizeof(fileRequestPayload));
    if (requestBuf) {
        free(requestBuf);
    }
    return rc;
}

RC_t processFileOperationRequest(int socket, fileOperationRequestPayload *payload) {
    RC_t rc = RC_FAILURE;
    fileInfoPayload *infoPayload /*= (infoPayload *)calloc(1, sizeof(fileInfoPayload))*/;
    LOG(DEBUG,"Received File Operation Request from %s for %s", payload->requesterIP, payload->fileName);
    printf("Received file operation request from %s for %s, flags : %0x", payload->requesterIP, payload->fileName, payload->flags);
    if (payload->flags & DEL_FILE_REQUEST) {
    	//Delete the entries
    	LOG(DEBUG, "Got delete file request for %s", payload->fileName);
    	//get all the entries for deleting the file
    	sendFileDelete(payload->fileName);
        removeFileMetaInfo(payload->fileName);
        dfs_write_to_file();
        //sendMetadataToNeighbour();


    }else  {
    	populateFileInfoPayload(&infoPayload, payload);
    	printf("Sending %s\n. Length = %d", infoPayload->fileName,sizeof(fileInfoPayload) + (infoPayload->noOfReplicas * infoPayload->noOfSplits * 16 ));
    	rc = sendPayload(socket, MSG_FILE_INFO, infoPayload, sizeof(fileInfoPayload) + (infoPayload->noOfReplicas * infoPayload->noOfSplits * 16 ));
    	if (rc != RC_SUCCESS) {
    		printf("\n Failed to send file information to requesting node");
    		LOG(DEBUG, "Problem sending information about file %s to requesting node ", payload->fileName);
    	    rc = RC_FILE_INFO_SEND_FAILURE;
    	}
    }
    if (infoPayload) {
    	free(infoPayload);
    }
    return rc;
}


RC_t processFileRequest(int socket, fileRequestPayload *payload)
{
     RC_t rc;
     //Look for the entry corresponding to the IP address.
     //Send the response
     FILE *fp = fopen(payload->fileName, "r");
     if (fp == NULL) {// Close connection. File not found
         close(socket);

     }else {
    	 sendFile(socket, payload->fileName, payload->fileName);
     }
     //close(socket);
     return rc;

}

RC_t processChunkOperationPayload(int socket, chunkOperationPayload* payload)
{
	RC_t rc;
	dfs_thread_info_new thread_data = {0};
	pthread_t thread;
	char ip[16];
	char command[500];
	chunkOperationPayload replicationResponse;
	int status = 0;
	char chunkName[500]= {0};
	int chunkNumber = 0;

	if ( payload->flags & REPLICATE_RESPONSE) {
		if (server_topology && server_topology->node && server_topology->node != myself) {
			LOG(ERROR, "Received Replicate %s even though I am not the leader. Dropping the message", "Response");
		}
		else {
			if (payload->flags & REPLICATE_SUCCESSFUL) {
				status = 1;
			}
		    memcpy(chunkName, payload->chunkName, strlen(payload->chunkName) - 4);
			chunkNumber = atoi(payload->chunkName + strlen(payload->chunkName) - 4);
			payload->chunkName[strlen(payload->chunkName) - 4]= 0;
			update_chunk_info( payload->chunkName, chunkNumber, ip, status );
		    dfs_write_to_file();
		    //sendMetadataToNeighbour();


		}
	}else if (payload->flags & REPLICATE_INSTRUCTION) {
		strcpy(ip, payload->ip);
		strcpy(thread_data.destFileName, payload->chunkName);
		strcpy(thread_data.fileName, payload->chunkName);
		strcpy(thread_data.ip[0],ip);
		thread_data.numOfAddresses = 1;
		printf("\nPushing chunk %s to %s (Received replicate instruction from master\n", thread_data.destFileName, thread_data.ip[0]);
		pthread_create(&thread, NULL, sendFileWrapper, (&thread_data));
		pthread_join(thread, NULL);
		if (thread_data.rc != RC_SUCCESS) {
			LOG(ERROR, "Could not replicate Chunk %s to %s", payload->chunkName, thread_data.ip);
			replicationResponse.flags = REPLICATE_FAILURE;
		}else {
			replicationResponse.flags |= REPLICATE_SUCCESSFUL;
		}
		strcpy(replicationResponse.chunkName, payload->chunkName);
		replicationResponse.flags |= REPLICATE_RESPONSE;
		strcpy(replicationResponse.ip, myself->IP);
        sendPayload(socket, MSG_CHUNK_OPERATION,  &replicationResponse, sizeof(replicationResponse));
        close(socket);


	}else if (payload->flags & DELETE_REPLICA) {
		sprintf(command, "rm %s????", payload->chunkName);
		system(command);

	}
return rc;
}
