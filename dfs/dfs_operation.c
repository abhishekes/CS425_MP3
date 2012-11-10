#include "dfs_operation.h"
#include "dfs_write_to_file.h"

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
	fileInfoPayload *payloadBuf1 = NULL;
	fileInfoPayload *fileInfo = NULL;
	int z=0;
	char *p;

	dfs_thread_info **file_thread;
	pthread_t       *threads;
	int size = 0;
	int i = 0;
	char suffix[] = "0000";
	char suffix_buf[5];

	if (inputFile == NULL) {
		     printf("\nCould not find source file");
	    	 return RC_INPUT_FILE_NOT_FOUND;
	}else {
	        fseek(inputFile, 0L, SEEK_END);
	        size = ftell(inputFile);
	        size = ceil(size / (1024 * 1024));
	        printf("\n****** File Length : %d********* \n" , size);
		    fclose(inputFile);
	}
    if (server_topology && server_topology->node ) {


    	my_data = calloc(1, sizeof(thread_data) + sizeof(fileOperationRequestPayload));
        (*my_data).payload = calloc(1, sizeof(fileOperationRequestPayload));

        memcpy((*my_data).ip, server_topology->node->IP, 16);

        (*my_data).payload_size = sizeof(fileOperationRequestPayload);
        (*my_data).msg_type = MSG_FILE_OPERATION_REQUEST;
        (*my_data).flags = WAIT_FOR_RESPONSE;
        payloadBuf = my_data->payload;
        payloadBuf->fileSize = size;
        //memcpy((*my_data).payload, payloadBuf, sizeof(fileOperationRequestPayload));
        strcpy(payloadBuf->fileName, destinationFileName);
        memcpy(payloadBuf->requesterIP, myself->IP, 16);
        payloadBuf->flags |= PUT_FILE_REQUEST;

        LOG(DEBUG,"Sending File Operation Request to %s for %s", my_data->ip, payloadBuf->fileName);
        pthread_create(&thread, NULL, send_node_update_payload, (my_data));
        pthread_join(thread, NULL);

        if (my_data->status == RC_SUCCESS) {
        	//Sachin's function to do splitting of files.
        	//Call all send file wrappers
        	if (my_data->return_data == NULL) {
        		my_data->status = RC_NO_RESPONSE_RECEIVED;
        		//free(thread);

        		free(my_data);
        		return RC_NO_RESPONSE_RECEIVED;
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
        		create_file_splits(localFileName, ((fileInfoPayload *)((my_data)->return_data))->fileName, ((fileInfoPayload *)((my_data)->return_data))->noOfSplits);
        		fileInfo = (fileInfoPayload *)my_data->return_data;
        		file_thread = (dfs_thread_info **)calloc(1, (sizeof(dfs_thread_info *) * fileInfo->noOfSplits));
        		threads = calloc(1, sizeof(pthread_t ) * fileInfo->noOfSplits);
        		for (i = 0; i < fileInfo->noOfSplits; i++ ) {
        			file_thread[i] = (dfs_thread_info *)calloc(1, sizeof(dfs_thread_info));
        			//threads[i] = calloc(1, sizeof(pthread_t));
        			sprintf(suffix_buf, "%d", i);
        			strcpy(suffix + 4 - strlen(suffix_buf), suffix_buf);
        			sprintf(file_thread[i]->destFileName, "%s%s", fileInfo->fileName, suffix);
        			DEBUG(("\nSending %s\n",suffix));
        			file_thread[i]->numOfAddresses = fileInfo->noOfReplicas;
        			printf("\n ******* IP::::: %s ********* \n", fileInfo->ipAddr[0][0] );
        			file_thread[i]->ip = malloc(fileInfo->noOfReplicas * 16);
        			memcpy(file_thread[i]->ip, fileInfo->ipAddr[i][0], fileInfo->noOfReplicas * 16);
        			pthread_create(&threads[i], NULL, sendFileWrapper, file_thread[i]);

        		}
        		for (i = 0; i < fileInfo->noOfSplits; i++ ) {
        			pthread_join(threads[i], NULL);
        		}
        		for (i = 0; i < fileInfo->noOfSplits; i++ ) {
        			free(file_thread[i]->ip);
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
    			free(my_data);
    			//Send finalize entry to master
        	    my_data = calloc(1, sizeof(thread_data) + sizeof(fileInfoPayload));

            	my_data = calloc(1, sizeof(thread_data) + sizeof(fileInfoPayload));
                (*my_data).payload = calloc(1, sizeof(fileInfoPayload));

                memcpy((*my_data).ip, server_topology->node->IP, 16);

                (*my_data).payload_size = sizeof(fileInfoPayload);
                (*my_data).msg_type = MSG_FILE_INFO;
                (*my_data).flags = RETURN_VALUE_REQUIRED;
                payloadBuf1 = my_data->payload;
                //payloadBuf->fileSize = size;
                //memcpy((*my_data).payload, payloadBuf, sizeof(fileOperationRequestPayload));
                strcpy(payloadBuf1->fileName, destinationFileName);

                payloadBuf1->flags |= (FILE_CHUNKS_PLACED_SUCCESSFULLY | FILE_INFO_UPDATE);

                LOG(DEBUG,"Sending File Info Update to %s for %s", my_data->ip, payloadBuf1->fileName);
                LOG(DEBUG,"Sending file info update payload for %s ", ((fileInfoPayload *)(my_data->payload))->fileName);


                pthread_create(&thread, NULL, send_node_update_payload, (my_data));
                pthread_join(thread, NULL);
                if (my_data->status != RC_SUCCESS) {
                    LOG(ERROR, "Failed to send finalize file metadata message to leader IP %s. Error = %d ", server_topology->node->IP, my_data->status);
                }

        	}

        }else {
        	LOG(ERROR,"Failed to get file information from %s", server_topology->node->IP);
        	printf("Failed to get file information from %s", server_topology->node->IP);
        }

    }
	for (i = 0; fileInfo && (i <  fileInfo->noOfSplits); i++) {
		free(file_thread[i]);
		//free(threads[i]);
	}
	//free(my_data->payload);
	//free(my_data);
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
	char suffix[] = "0000";
	char suffix_buf[5];
	int i;

    if (server_topology && server_topology->node ) {
    	my_data = calloc(1, sizeof(thread_data) + sizeof(fileOperationRequestPayload));
        (*my_data).payload = calloc(1, sizeof(fileOperationRequestPayload));

        memcpy((*my_data).ip, server_topology->node->IP, 16);

        (*my_data).payload_size = sizeof(fileOperationRequestPayload);
        (*my_data).msg_type = MSG_FILE_OPERATION_REQUEST;
        (*my_data).flags = WAIT_FOR_RESPONSE;
        payloadBuf = my_data->payload;
        //memcpy((*my_data).payload, payloadBuf, sizeof(fileOperationRequestPayload));
        strcpy(payloadBuf->fileName, remoteFileName);
        memcpy(payloadBuf->requesterIP, myself->IP, 16);
        payloadBuf->flags |= GET_FILE_REQUEST;

        LOG(DEBUG,"Sending File Operation Request to %s for %s", my_data->ip, payloadBuf->fileName);
        pthread_create(&thread, NULL, send_node_update_payload, (my_data));
        pthread_join(thread, NULL);
        printf("status : %d, my_data->return_data = %0x ", my_data->status, my_data->return_data);
        if (my_data->status == RC_SUCCESS) {

		//Check if you got the reply
        	if (my_data->return_data != NULL) {
			//Loop through all splits, creating an entry for each
			//receiveFileWrapper();
    	    //Transfer all parts to destinations
    		//create_file_splits(((fileInfoPayload *)((my_data)->return_data))->fileName, ((fileInfoPayload *)((my_data)->return_data))->noOfSplits);



        		fileInfo = (fileInfoPayload *)my_data->return_data;
    			if (fileInfo->flags & FILE_NOT_FOUND ) {
    				printf("\n File could not be found on the distributed file system");

    			} else {
    				*file_thread = calloc(1, sizeof(dfs_thread_info *) * fileInfo->noOfSplits);
    				threads = calloc(1, sizeof(pthread_t) * fileInfo->noOfSplits);
    				for (i = 0; i < fileInfo->noOfSplits; i++ ) {
    					file_thread[i] = calloc(1, sizeof(dfs_thread_info));
    					//threads[i] = cadlloc(1, sizeof(pthread_t));
    					//sprintf(file_thread[i]->destFileName, "%s.%d", fileInfo->fileName, i+1);
    		            sprintf(suffix_buf, "%d", i);
    		            strcpy(suffix + 4 - strlen(suffix_buf), suffix_buf);
    		            sprintf(file_thread[i]->destFileName, "%s%s", remoteFileName, suffix_buf);
    					//DEBUG(("\nSending %s\n",suffix));
    		            file_thread[i]->numOfAddresses = fileInfo->noOfReplicas;
    		            file_thread[i]->ip = malloc(fileInfo->noOfReplicas * 16);
    		            sprintf(file_thread[i]->destFileName, "%s%s", remoteFileName, suffix);
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
    					if (file_thread[i]->ip) {
    						free(file_thread[i]->ip);
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
	}
	return my_data->status;
    }
    return RC_FAILURE;
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

    fp = fopen(dfs_data->destFileName, "r");
	if (fp == NULL) {
        printf("File Not Found %s", dfs_data->destFileName);
		return RC_FILE_NOT_FOUND;
	}
    rc = RC_SUCCESS;
	for ( i = 0; i < dfs_data->numOfAddresses ; i++, IP++) {
		printf("IP **** %s\n\n", IP);
		if (!strcmp(dfs_data->ip[i], myself->IP)) {
        	printf("\nContinuing\n");
        	continue; //File already present on local IP
        }
    	if ( createConnection(&nodeAddress, IP, &sock) == RC_SUCCESS) {
            printf("\nTrying to send file %s to %s\n", dfs_data->fileName, IP);
        	if ((rc=sendFile(sock, dfs_data->destFileName, dfs_data->destFileName)) == RC_SUCCESS) {
        		printf("\nSent file %s to %s successfully\n", dfs_data->fileName, IP);
        		//break;
            }else {
            	printf("\nFailed to send file %s to %s. Error = %d", dfs_data->fileName, IP, rc );
            }

        } else {
        	rc = RC_FAILURE;
        }
    }
    dfs_data->rc = rc;
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

RC_t create_file_splits(char *localFileName, char *fileName , int numOfSplits)
{
     RC_t rc;
     int fp = open(localFileName, O_RDONLY /*| O_LARGE_FILE*/);
     int fp1;
     char command[1000];
     printf("\n&&&&&&&&& %s &&&&&&\n", fileName);

     if (fp == NULL) {
    	 return RC_FILE_NOT_FOUND;
     }else {
    	 close(fp);
     }
     sprintf(command, "split -d -b 67108664 -a 4 %s %s",localFileName, fileName);
     system(command);
     return RC_SUCCESS;


}

RC_t merge_file_splits(char *fileName ,char *localFileName, int numOfSplits)
{
     char command[1000];
     sprintf(command, "cat %s???? > %s", fileName, localFileName );
     system(command);

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


	if(( request->flags & GET_FILE_REQUEST )) {
		if( ptr == NULL ) {
			(*infoPayload) = (fileInfoPayload*)calloc(1, sizeof(fileInfoPayload));
			(*infoPayload)->flags |= FILE_INFO_RESPONSE;
			(*infoPayload)->flags |= FILE_NOT_FOUND;
		} else {
			(*infoPayload) = (fileInfoPayload*)calloc(1, sizeof(fileInfoPayload) + 16 * ptr->numReplicas * ptr->numberOfChunks);
			(*infoPayload)->flags |= FILE_INFO_RESPONSE;
			(*infoPayload)->noOfReplicas = ptr->numReplicas;
			(*infoPayload)->noOfSplits = ptr->numberOfChunks;

			ChunkInfo *chunkPtr = ptr->chunkInfo;
			for(i = 0; i < ptr->numberOfChunks; i++, chunkPtr = chunkPtr->next) {
				for(j = 0; j < ptr->numReplicas; j++ ) {
					strcpy((*infoPayload)->ipAddr[i][j], chunkPtr->IP[j]);
				}
			}
		}
	} else if ( request->flags & PUT_FILE_REQUEST ) {
		if( ptr == NULL) {

			numChunks = ceil((double)request->fileSize / CHUNK_SIZE_IN_MB);
			if (!numChunks) {
				numChunks = 1;
			}

			addFileMetaInfo(request->fileName, request->fileSize, request->flags, numChunks, request->requesterIP);
			dfs_write_to_file();


			ptr = getFileMetadataPtr(request->fileName);

			if(ptr == NULL) {
				printf("ERROR : Could not find fileMetadata ptr even after inserting");
			}

			(*infoPayload) = (fileInfoPayload*)calloc(1, sizeof(fileInfoPayload) + (16 * ptr->numReplicas * ptr->numberOfChunks));
			(*infoPayload)->flags |= FILE_INFO_RESPONSE | FILE_NAME_AVAILABLE;
			(*infoPayload)->noOfReplicas = ptr->numReplicas;
			(*infoPayload)->noOfSplits = ptr->numberOfChunks;

			ChunkInfo *chunkPtr = ptr->chunkInfo;
			printf("^^^^^^ Limits : %0x\n ", *infoPayload + sizeof(fileInfoPayload) + (16 * ptr->numReplicas * ptr->numberOfChunks) );
			for(i = 0; i < ptr->numberOfChunks; i++, chunkPtr = chunkPtr->next) {
				for(j = 0; j < ptr->numReplicas; j++ ) {
					printf("\n*** Copying to location %0x from %0x", (*infoPayload)->ipAddr[i][j], chunkPtr->IP[j]);
					memcpy((*infoPayload)->ipAddr[i][j], chunkPtr->IP[j], 16);
				}
			}
		} else {
			(*infoPayload) = (fileInfoPayload*)calloc(1, sizeof(fileInfoPayload));
			(*infoPayload)->flags |= FILE_INFO_RESPONSE | FILE_ALREADY_PRESENT;
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


