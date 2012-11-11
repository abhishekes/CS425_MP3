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
    	sendFileDelete(char *IP)


    }else  {
    	populateFileInfoPayload(&infoPayload, payload);
    	printf("Sending %s\n. Length = %d", infoPayload->fileName,Look for file entry and return corresponding entrychar  sizeof(fileInfoPayload) + (infoPayload->noOfReplicas * infoPayload->noOfSplits * 16 ));
    	rc = sendPayload(socket, MSG_FILE_INFO, infoPayload, sizeof(fileInfoPayload) + (infoPayload->noOfReplicas * infoPayload->noOfSplits * 16 ));
    	if (rc != RC_SUCCESS) {
    		printf("\n Failed to send file information to requesting node");
    		LOG(DEBUG, "Problem sending information about file %s to requesting node ", payload->fileName);
    	    rc = RC_FILE_INFO_SEND_FAILURE;
    	}
    //}
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
     dfs_thread_info *thread_data;
     pthread_t thread;
     char (*IP)[16];
     char chunkName[500];
     IP = payload->ip;
     if ( payload->flags & REPLICATE_RESPONSE) {
         if (server_topology && server_topology->node && server_topology->node != myself) {
             LOG(ERROR, "Received Replicate %s even though I am not the leader. Dropping the message", "Response");
         }
         else {
        	 //Update the fields.
         }
     }else if (payload->flags & REPLICATE_INSTRUCTION) {
         strcpy(thread_data->destFileName, payload->chunkName);
         thread_data->ip = IP; //TODO
         thread_data->numOfAddresses = payload->numOfReplicas;
         pthread_create(&thread, NULL, receiveFileWrapper, (thread_data));
         pthread_join(thread, NULL);
         if (thread_data->rc != RC_SUCCESS) {
        	 LOG(ERROR, "Could not replicate Chunk Payload %s", thread_data->ip);
         }

     }else if (payload->flags & DELETE_REPLICA) {
    	 sprintf(chunkName, "rm %s????", payload->chunkName);
    	 command(chunkName);

     }
     return rc;
}

