#include "dfs_payload_process.h"
#include "fdetect_payload_process.h"


extern struct Head_Node *server_topology;
extern struct Node *myself;

RC_t processFileInfoPayload(fileInfoPayload *infoPayload, void *return_data) {
    RC_t rc;
	if (infoPayload->flags & FILE_INFO_UPDATE) {
    	 if (server_topology && server_topology->node) {
    		 if (myself) {
    			 if (server_topology->node != myself) {
    				 printf("Received file information even when node is not master");
    				 LOG(DEBUG, "Received file information for the file %s even when though I am not the master ", infoPayload->fileName);
    			 }else {
                  //Add information to database
    			 }
    		 }
    	 }
     }else if (infoPayload->flags & FILE_INFO_RESPONSE) {
     //Allocate memory and return the response
    	 LOG(DEBUG, "Received file information from master for file %s", infoPayload->fileName);
         //return_data = malloc()
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
    if (payload->flags & GET_FILE_REQUEST) {
    	//Look for file entry and return corresponding entry
    }else if (payload->flags & PUT_FILE_REQUEST ) {
    	populateFileInfoPayload(infoPayload, payload->fileName);
    	rc = sendPayload(socket, MSG_FILE_INFO, infoPayload, sizeof(infoPayload));
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

RC_t processNodeFileInfoRequest(int socket, requestNodeFileInfo *payload)
{
     RC_t rc;
     //Look for the entry corresponding to the IP address.
     //Send the response


     return rc;

}

RC_t processChunkReplicationPayload(int socket, chunkReplicatePayload* payload)
{
     RC_t rc;
     dfs_thread_info *thread_data;
     pthread_t thread;
     char *IP;
     if ( payload->flags & REPLICATE_RESPONSE) {
         if (server_topology && server_topology->node && server_topology->node != myself) {
             LOG(ERROR, "Received Replicate %s even though I am not the leader. Dropping the message", "Response");
         }
         else {
        	 //Update the fields.
         }
     }else if (payload->flags & REPLICATE_INSTRUCTION) {
         strcpy(thread_data->fileName, payload->chunkName);
         thread_data->ip = IP; //TODO
         thread_data->numOfAddresses = payload->numOfReplicas;
         pthread_create(&thread, NULL, receiveFileWrapper, (thread_data));
         pthread_join(thread, NULL);
         if (thread_data->rc != RC_SUCCESS) {
        	 LOG(ERROR, "Could not send replicate Chunk Payload to %s ", thread_data->ip);
         }

     }
     return rc;
}

