#include "dfs_operation.h"


fileInfoPayload fileInfo ;
extern FileMetadata *gFileMetaData;


extern char myIP[16];
extern struct Head_Node *server_topology;
extern struct Node* myself;
//ip is the IP address of the node that has crashed.
//This function is called by the leader

RC_t dfs_replicate_files_of_crashed_node(char *ip) {

	thread_data *my_data;
	pthread_t thread;
	RC_t rc;
    int i = 0;
    chunkReplicatePayload *payloadBuf = NULL;

	for (i = 0 ; i < 1; i++/*Other conditions->To be updated */) {


		my_data = calloc(1, sizeof(thread_data) + sizeof(chunkReplicatePayload));
		(*my_data).payload = calloc(1, sizeof(chunkReplicatePayload));
		payloadBuf = (nodeFileInfo *)((*my_data).payload);
		memcpy( payloadBuf->ip, ip, 16);
		payloadBuf->flags |= REPLICATE_INSTRUCTION;
		my_data->payload_size = sizeof(chunkReplicatePayload);

		(*my_data).msg_type = MSG_CHUNK_REPLICATION;


		pthread_create(&thread, NULL, send_node_update_payload, (my_data));
		pthread_join(thread, NULL);

		if (my_data->status != RC_SUCCESS) {
			LOG(ERROR, "Failed to send replicate chunk payload for chunk present on IP %s", ip);
		}
	}
	return rc;
}
RC_t dfs_file_transfer (fileOperation op, char *localFileName, char *destinationFileName)
{
	FILE *inputFile = NULL;
	inputFile = fopen(localFileName, "r");
	thread_data *my_data;
	pthread_t thread;
	fileOperationRequestPayload *payloadBuf = NULL;
	fileInfoPayload *fileInfo = NULL;

	dfs_thread_info **file_thread;
	pthread_t       *threads;
	int size = 0;
	int i = 0;
	if (!inputFile) {
		     printf("\nCould not find source file");
	    	 return RC_INPUT_FILE_NOT_FOUND;
	}else {
	        fseek(inputFile, 0L, SEEK_END);
	        size = ftell(inputFile);
		    fclose(inputFile);
	}
    if (server_topology && server_topology->node ) {
	    my_data = calloc(1, sizeof(thread_data) + sizeof(fileOperationRequestPayload));
        (*my_data).payload = calloc(1, sizeof(fileOperationRequestPayload));
        payloadBuf = (fileOperationRequestPayload *)((*my_data).payload);
        payloadBuf->fileSize = size;
        memcpy(payloadBuf->requesterIP, myself->IP, 16);
        strcpy(payloadBuf->fileName, destinationFileName);
        memcpy((*my_data).ip, server_topology->node->IP, 16);

        (*my_data).payload_size = sizeof(fileOperationRequestPayload);
        (*my_data).msg_type = MSG_FILE_OPERATION_REQUEST;
        (*my_data).flags = WAIT_FOR_RESPONSE;

        //memcpy((*my_data).payload, payloadBuf, sizeof(fileOperationRequestPayload));
        pthread_create(&thread, NULL, send_node_update_payload, (my_data));
        pthread_join(thread, NULL);

        if (my_data->status == RC_SUCCESS) {
        	//Sachin's function to do splitting of files.
        	//Call all send file wrappers
        	if (my_data->return_data == NULL) {
        		my_data->status = RC_NO_RESPONSE_RECEIVED;
        		//free(thread);
        		free(my_data);
        		return my_data->status;
        	}else if(!(((fileInfoPayload *)((my_data)->return_data))->flags & FILE_NAME_AVAILABLE)) {
        		LOG(DEBUG,"Could not get valid replication details from master. Flags : %0x ", ((fileInfoPayload *)((my_data)->return_data))->flags);
                if (((fileInfoPayload *)((my_data)->return_data))->flags & FILE_ALREADY_PRESENT) {
                	printf("Operation failed. File name already exists");
                }
                //free(thread);
                free(my_data);
                return RC_FAILURE;
        	}else {
        	    //Transfer all parts to destinations
        		create_file_splits(((fileInfoPayload *)((my_data)->return_data))->fileName, ((fileInfoPayload *)((my_data)->return_data))->noOfSplits);
        		fileInfo = (fileInfoPayload *)my_data->return_data;
        		*file_thread = calloc(1, sizeof(dfs_thread_info *) * fileInfo->noOfSplits);
        		threads = calloc(1, sizeof(pthread_t ) * fileInfo->noOfSplits);
        		for (i = 0; i < fileInfo->noOfSplits; i++ ) {
        			file_thread[i] = calloc(1, sizeof(dfs_thread_info));
        			//threads[i] = calloc(1, sizeof(pthread_t));
                    sprintf(file_thread[i]->destFileName, "%s.%d", fileInfo->fileName, i+1);
        			file_thread[i]->numOfAddresses = fileInfo->noOfReplicas;
        			memcpy(file_thread[i]->ip, fileInfo->ipAddr[i][0], fileInfo->noOfReplicas * 16);
        			pthread_create(&threads[i], NULL, sendFileWrapper, file_thread[i]);

        		}
        		for (i = 0; i < fileInfo->noOfSplits; i++ ) {
        			pthread_join(threads[i], NULL);
        		}
        		for (i = 0; i < fileInfo->noOfSplits; i++ ) {
        			if (file_thread[i]->rc != RC_SUCCESS) {
        				LOG(DEBUG,"Could not place replicas for file split %d", i+1);

        				break;
        			}
        		}
    			if (i != fileInfo->noOfSplits) {
    				printf("Failed to Transfer splits to destination");
    				for (i = 0; i <  fileInfo->noOfSplits; i++) {
    					free(file_thread[i]);
    					//free(threads[i]);
    				}
    				free(my_data);
    				free(file_thread);
    				free(threads);
    				free(fileInfo);

                    return RC_FILE_REPLICA_PLACEMENT_FAILURE;

    			}
        		//Send finalize entry to master
        	    my_data = calloc(1, sizeof(thread_data) + sizeof(fileInfoPayload));
                (*my_data).payload = calloc(1, sizeof(fileInfoPayload));
                payloadBuf = (fileInfoPayload *)((*my_data).payload);
                payloadBuf->fileSize = sizeof(fileInfoPayload);
                payloadBuf->flags |= FILE_CHUNKS_PLACED_SUCCESSFULLY;
                strcpy(payloadBuf->fileName, destinationFileName);
                memcpy((*my_data).ip, server_topology->node->IP, 16);

                (*my_data).payload_size = sizeof(fileInfoPayload);
                (*my_data).msg_type = MSG_FILE_INFO;


                memcpy((*my_data).payload, payloadBuf, sizeof(fileInfoPayload));
                pthread_create(&thread, NULL, send_node_update_payload, (my_data));
                pthread_join(thread, NULL);
                if (my_data->status != RC_SUCCESS) {
                    LOG(ERROR, "Failed to send finalize file metadata message to leader IP %s ", server_topology->node->IP);
                }

        	}

        }else {
        	LOG(ERROR,"Failed to get file information from %s", server_topology->node->IP);
        	printf("Failed to get file information from %s", server_topology->node->IP);
        }

    }
	for (i = 0; i <  fileInfo->noOfSplits; i++) {
		free(file_thread[i]);
		//free(threads[i]);
	}
	free(my_data);
	free(file_thread);
	free(threads);
	free(fileInfo);
	return my_data->status;

}

RC_t dfs_file_receive(char *localFileName, char *remoteFileName)
{
	FILE *inputFile = NULL;
	thread_data *my_data;
	pthread_t thread;
	fileOperationRequestPayload *payloadBuf;
	fileInfoPayload *fileInfo;
	pthread_t *threads;
	dfs_thread_info **file_thread;
	int i;

    if (server_topology && server_topology->node ) {
	my_data = calloc(1, sizeof(thread_data) + sizeof(fileOperationRequestPayload));
        (*my_data).payload = calloc(1, sizeof(fileOperationRequestPayload));

        memcpy((*my_data).ip, server_topology->node->IP, 16);

        (*my_data).payload_size = sizeof(fileInfo);
        (*my_data).msg_type = MSG_FILE_OPERATION_REQUEST;
        (*my_data).flags = WAIT_FOR_RESPONSE;
        payloadBuf = my_data->payload;
        //memcpy((*my_data).payload, payloadBuf, sizeof(fileOperationRequestPayload));
        strcpy(payloadBuf->fileName, remoteFileName);
        memcpy(payloadBuf->requesterIP, myself->IP, 16);
        payloadBuf->flags |= GET_FILE_REQUEST;
        pthread_create(&thread, NULL, send_node_update_payload, (my_data));
        pthread_join(thread, NULL);
    }
	if (my_data->status == RC_SUCCESS) {

		//Check if you got the reply
		if (my_data->return_data != NULL) {
        //Loop through all splits, creating an entry for each
			//receiveFileWrapper();
    	    //Transfer all parts to destinations
    		//create_file_splits(((fileInfoPayload *)((my_data)->return_data))->fileName, ((fileInfoPayload *)((my_data)->return_data))->noOfSplits);
    		fileInfo = (fileInfoPayload *)my_data->return_data;
    		*file_thread = calloc(1, sizeof(dfs_thread_info *) * fileInfo->noOfSplits);
    		threads = calloc(1, sizeof(pthread_t) * fileInfo->noOfSplits);
    		for (i = 0; i < fileInfo->noOfSplits; i++ ) {
    			file_thread[i] = calloc(1, sizeof(dfs_thread_info));
    			//threads[i] = calloc(1, sizeof(pthread_t));
                sprintf(file_thread[i]->destFileName, "%s.%d", fileInfo->fileName, i+1);
    			file_thread[i]->numOfAddresses = fileInfo->noOfReplicas;
    			memcpy(file_thread[i]->ip, fileInfo->ipAddr[i][0], fileInfo->noOfReplicas * 16);
    			pthread_create(&threads[i], NULL, receiveFileWrapper, file_thread[i]);

    		}
    		for (i = 0; i < fileInfo->noOfSplits; i++ ) {
    			pthread_join(threads[i], NULL);
    		}
    		for (i = 0; i < fileInfo->noOfSplits; i++ ) {
    			if (file_thread[i]->rc != RC_SUCCESS) {
    				LOG(DEBUG,"Could not receive file split %d from any of the replicas", i+1);

    				break;
    			}
    		}
			if (i != fileInfo->noOfSplits) {
				printf("Failed to receive splits from nodes");
				for (i = 0; i <  fileInfo->noOfSplits; i++) {
					free(file_thread[i]);
					//free(threads[i]);
				}
				free(my_data);
				free(file_thread);
				free(threads);
				free(fileInfo);

                return RC_FAILURE;

			}


			//Merge the files
			merge_file_splits(remoteFileName , localFileName, fileInfo->noOfSplits);

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
    FILE *fp;
    LOG(DEBUG, "Trying to fetch file  %s", dfs_data->destFileName);
    if ((fp =fopen(dfs_data->destFileName, "r")) != NULL) {
    	fclose(fp);
    	LOG(DEBUG, "File %s present locally. Hence, not fetching from distributed file system", dfs_data->destFileName);
    	dfs_data->rc = RC_SUCCESS;//File present locally , return
        return RC_SUCCESS;
    }
    for ( i = 0; i < dfs_data->numOfAddresses ; i++, IP++) {
    	LOG(DEBUG, "Trying to fetch file  %s from %s", dfs_data->destFileName, IP);
    	if ( createConnection(&nodeAddress, IP, &sock) == RC_SUCCESS) {

        	if (sendFileInfoRequest(sock, dfs_data->destFileName) == RC_SUCCESS) {
                while ((rc = message_decode(sock, &packet)) == RC_SUCCESS) {
                        processPacket(socket, packet, &data);

                }
                if (data) {
                	if (*((RC_t *)data) == RC_SUCCESS ) {
                		LOG(DEBUG, "Fetched file  %s from %s successfully", dfs_data->destFileName, IP);
                		break;
                	}
                }
            }

        }
    }
    if ( i < dfs_data->numOfAddresses) {
	    dfs_data->rc = RC_SUCCESS;
         }else {
        dfs_data->rc = rc;
    }
    if (data) {
        free(data);
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
    int i = 0;
    fp = fopen(dfs_data->fileName, "r");
	if (fp == NULL) {
	    return RC_FILE_NOT_FOUND;
	}
    for ( i = 0; i < dfs_data->numOfAddresses ; i++, IP++) {
        if (!strcmp(dfs_data->ip[i], myself->IP)) {
        	continue; //File already present on local IP
        }
    	if ( createConnection(&nodeAddress, IP, &sock) == RC_SUCCESS) {
            printf("\nTrying to send file %s to %s\n", dfs_data->fileName, IP);
        	if (sendFile(sock, dfs_data->fileName, dfs_data->destFileName) == RC_SUCCESS) {
        		printf("\nSent file %s to %s successfully\n", dfs_data->fileName, IP);
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

RC_t createConnection(struct sockaddr_in *nodeAddress, char *IP, int *sock)
{

    memset(nodeAddress, 0, sizeof(struct sockaddr_in));
    nodeAddress->sin_family         = AF_INET;
    nodeAddress->sin_addr.s_addr   = inet_addr(IP);
    nodeAddress->sin_port          = htons(TCP_LISTEN_PORT);
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

/*RC_t dfs_replicate_chunk(void *thread_data) {

	RC_t rc;
	dfs_thread_info *info = (dfs_thread_info *)(thread_data);
    char *IP = info->ip;
    void *data = NULL;
    payloadBuf *packet;
    struct sockaddr_in nodeAddress;
    int sock;

    if ( createConnection(&nodeAddress, IP, &sock) == RC_SUCCESS) {

    }
    return rc;


}*/

RC_t create_file_splits(char *fileName , int numOfSplits)
{
     RC_t rc;
     FILE *fp = fopen(fileName, "r");
     FILE *fp1;
     char chunkName[500];
     char ch ;
     int i =0;
     long numOfBytes = 0;
     while(!feof(fp)) {
         if (numOfBytes == CHUNK_SIZE_IN_MB * 1024 * 1024) {
             if (fp1 != NULL) {
                 fclose(fp1);
                 sprintf(chunkName, "%s.%d", fileName, i );
                 fp1 = fopen(chunkName, "w");
                 i ++;
                 numOfBytes = 0;
             }
         }
    	 ch = fgetc(fp);
    	 numOfBytes++;
    	 fputc(ch, fp1);
     }
     if (fp1 != NULL) {
    	 fclose(fp1);
     }
     return RC_SUCCESS;


}

RC_t merge_file_splits(char *fileName ,char *localFileName, int numOfSplits)
{
     RC_t rc;
     FILE *fp = fopen(localFileName, "w");
     FILE *fp1;
     char chunkName[500];
     char ch ;
     int i =0;
     long numOfBytes = 0;
     if (fp == NULL) {
    	 printf("\nFailed to create file for writing");
    	 return RC_FAILURE;
     }
     while(i < numOfSplits) {
         sprintf(chunkName, "%s.%d", fileName, i+1);
    	 fp1 = fopen(chunkName, "r");
    	 if (fp1 == NULL) {
    		 return RC_FAILURE;
    	 }
    	 while(!feof(fp1)) {
    		 ch = fgetc(fp1);
    		 fputc(ch, fp);


    	 }
    	 fclose(fp1);
     }
     fclose(fp);
     return RC_SUCCESS;


}

RC_t make_master(char *prevMasterIP)
{
    RC_t rc;
    rc = create_metadata_from_file();
    if (rc != RC_SUCCESS) {
        return rc;
    }
    rc = dfs_replicate_files_of_crashed_node(prevMasterIP);

    return rc;

}

RC_t create_metadata_from_file() {
    return RC_SUCCESS;
}

//This is to send reply to the client with the list of IPs where it can put the file into
RC_t populateFileInfoPayload(fileInfoPayload **infoPayload, fileOperationRequestPayload *request) {
    //Check if a file with this name already exists
    //fileInfoPayload->flags |= FILE_ALREADY_PRESENT; Error Case
	FileMetadata *ptr = NULL;
	int i, j, numChunks;

	ptr = getFileMetadataPtr(request->fileName);


	if(( request->flags & FILE_GET )) {
		if( ptr == NULL ) {
			(*infoPayload) = (fileInfoPayload*)calloc(1, sizeof(fileInfoPayload));
			(*infoPayload)->flags |= FILE_NOT_FOUND;
		} else {
			(*infoPayload) = (fileInfoPayload*)calloc(1, sizeof(fileInfoPayload) + 16 * ptr->numReplicas * ptr->numberOfChunks);
			(*infoPayload)->flags |= ptr->flags;
			(*infoPayload)->noOfReplicas = ptr->numReplicas;
			(*infoPayload)->noOfSplits = ptr->numberOfChunks;

			ChunkInfo *chunkPtr = ptr->chunkInfo;
			for(i = 0; i < ptr->numberOfChunks; i++, chunkPtr = chunkPtr->next) {
				for(j = 0; j < ptr->numReplicas; j++ ) {
					strcpy((*infoPayload)->ipAddr[i][j], chunkPtr->IP[j]);
				}
			}
		}
	} else if ( request->flags & FILE_PUT ) {
		if( ptr == NULL) {

			numChunks = ceil((double)request->fileSize / CHUNK_SIZE_IN_MB);

			addFileMetaInfo(request->fileName, request->fileSize, request->flags, numChunks, request->requesterIP);

			ptr = getFileMetadataPtr(request->fileName);

			if(ptr == NULL) {
				printf("ERROR : Could not find fileMetadata ptr even after inserting");
			}

			(*infoPayload) = (fileInfoPayload*)calloc(1, sizeof(fileInfoPayload) + 16 * ptr->numReplicas * ptr->numberOfChunks);
			(*infoPayload)->flags |= ptr->flags;
			(*infoPayload)->noOfReplicas = ptr->numReplicas;
			(*infoPayload)->noOfSplits = ptr->numberOfChunks;

			ChunkInfo *chunkPtr = ptr->chunkInfo;
			for(i = 0; i < ptr->numberOfChunks; i++, chunkPtr = chunkPtr->next) {
				for(j = 0; j < ptr->numReplicas; j++ ) {
					strcpy((*infoPayload)->ipAddr[i][j], chunkPtr->IP[j]);
				}
			}
		} else {
			(*infoPayload) = (fileInfoPayload*)calloc(1, sizeof(fileInfoPayload));
			(*infoPayload)->flags |= FILE_ALREADY_PRESENT;
		}
	}

	strcpy((*infoPayload)->fileName, request->fileName);

	return RC_SUCCESS;
}

//Some node has sent this payload to us to update a file entry, maybe to update the entry, or, delete it
RC_t processFileInfoUpdatePayload(fileInfoPayload *infoPayload) {
    RC_t rc;
    int found = 0;
    int i = 0;
    FileMetadata *tmp = gFileMetaData;


    while (tmp != NULL) {
    	if (!strcmp(infoPayload->fileName, tmp->fileName)) { //Entry found
    		found = 1;

    		if (infoPayload->flags & FILE_CHUNKS_PLACED_SUCCESSFULLY) {
    			//Chunks placed successfully. Mark entry as valid

    				infoPayload->flags |= ENTRY_VALID;
    				LOG(DEBUG,"Marking entry for %s as active", infoPayload->fileName);

    			}else {
    				infoPayload->flags &= ~ENTRY_VALID;
    				LOG(DEBUG,"Marking entry for %s as active", infoPayload->fileName);
    			}
    	}
    	tmp = tmp->next;


    }
    if (!found) {
    	LOG(DEBUG,"Received file info update payload for %s which was not found", infoPayload->fileName);
    }
    return rc;

}


RC_t dfs_delete_file(char *fileName) {

	RC_t rc = RC_FAILURE;

	return rc;

}


