#include "dfs_operation.h"
#include "dfs_write_to_file.h"

fileInfoPayload fileInfo ;
extern FileMetadata *gFileMetaData;


extern char myIP[16];
extern struct Head_Node *server_topology;
extern struct Node* myself;

#define MAX_DFS_THREADS 10
//ip is the IP address of the node that has crashed.
//This function is called by the leader

RC_t dfs_replicate_files_of_crashed_node(char *ip) {

	thread_data *my_data;
	pthread_t thread;
	RC_t rc;
    int i = 0;
    chunkOperationPayload *payloadBuf = NULL;

	for (i = 0 ; i < 1; i++/*Other conditions->To be updated */) {


		my_data = calloc(1, sizeof(thread_data) + sizeof(chunkOperationPayload));
		(*my_data).payload = calloc(1, sizeof(chunkOperationPayload));
		payloadBuf = (chunkOperationPayload *)((*my_data).payload);
		memcpy( payloadBuf->ip, ip, 16);
		payloadBuf->flags |= REPLICATE_INSTRUCTION;
		my_data->payload_size = sizeof(chunkOperationPayload);

		(*my_data).msg_type = MSG_CHUNK_OPERATION;


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
	int threadCount = 0;
	char *p;
	int splitsToBeTransferred = 0;
	int skip = 0;

	dfs_thread_info_new file_thread[MAX_DFS_THREADS];
	pthread_t           threads[MAX_DFS_THREADS];
	int size = 0;
	int i = 0;
	int j,k;
	char suffix[] = "0000";
	char suffix_buf[5];

	if (inputFile == NULL) {
		printf("\nCould not find source file");
		return RC_INPUT_FILE_NOT_FOUND;
	}else {
		fseek(inputFile, 0L, SEEK_END);
		size = ftell(inputFile);
		size = ceil((float)size / (1024 * 1024));
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
				free(my_data->return_data);
				free(my_data);
				return RC_FAILURE;
			}else {
				//Transfer all parts to destinations
				create_file_splits(localFileName, ((fileInfoPayload *)((my_data)->return_data))->fileName, ((fileInfoPayload *)((my_data)->return_data))->noOfSplits);

				fileInfo = (fileInfoPayload *)my_data->return_data;

				threadCount = (fileInfo->noOfSplits > MAX_DFS_THREADS) ? MAX_DFS_THREADS : fileInfo->noOfSplits ;
				splitsToBeTransferred = fileInfo->noOfSplits;

				for (i=0 ; splitsToBeTransferred > 0 && !skip ; splitsToBeTransferred -= threadCount) {
					skip = 0;
					for (j = 0; j < threadCount && j < splitsToBeTransferred; j++ , i++) {
						sprintf(suffix_buf, "%d", i);
						strcpy(suffix + 4 - strlen(suffix_buf), suffix_buf);
						sprintf(file_thread[j].destFileName, "%s%s", fileInfo->fileName, suffix);

						file_thread[j].numOfAddresses = fileInfo->noOfReplicas;
						printf("\n ******* IP::::: %s ********* \n", fileInfo->ipAddr[0][0] );
						memcpy(file_thread[j].ip, fileInfo->ipAddr[j][0], fileInfo->noOfReplicas * 16);
						pthread_create(&threads[j], NULL, sendFileWrapper, &file_thread[j]);

					}
					for (k = 0; k < j; k++ ) {
						pthread_join(threads[k], NULL);
					}
					for (k = 0; k < j; k++ ) {

						if (file_thread[k].rc != RC_SUCCESS) {
							LOG(DEBUG,"Could not place replicas for file split %d", i);
							break;
						}
					}
					if (k != j) {
						printf("Failed to Transfer splits to destination");
						skip = 1;
						break;

					}
				}
				free(fileInfo);
				free(my_data);

				//Send finalize entry to master


				my_data = calloc(1, sizeof(thread_data) + sizeof(fileInfoPayload));
				(*my_data).payload = calloc(1, sizeof(fileInfoPayload));
				payloadBuf1 = my_data->payload;

				if (skip == 1) {
					payloadBuf1->flags |= (FILE_CHUNKS_PLACED_SUCCESSFULLY | FILE_INFO_UPDATE);

				} else {
					payloadBuf1->flags |= (FILE_CHUNKS_COULD_NOT_BE_PLACED | FILE_INFO_UPDATE);
				}


				memcpy((*my_data).ip, server_topology->node->IP, 16);

				(*my_data).payload_size = sizeof(fileInfoPayload);
				(*my_data).msg_type = MSG_FILE_INFO;
				(*my_data).flags = RETURN_VALUE_REQUIRED;

				strcpy(payloadBuf1->fileName, destinationFileName);



				LOG(DEBUG,"Sending File Info Update to %s for %s", my_data->ip, payloadBuf1->fileName);
				LOG(DEBUG,"Sending file info update payload for %s ", ((fileInfoPayload *)(my_data->payload))->fileName);


				pthread_create(&thread, NULL, send_node_update_payload, (my_data));
				pthread_join(thread, NULL);
				if (my_data->status != RC_SUCCESS) {
					LOG(ERROR, "Failed to send finalize file metadata message to leader IP %s. Error = %d ", server_topology->node->IP, my_data->status);
				}

				free(my_data);
				if (skip == 1) {
					return RC_FILE_REPLICA_PLACEMENT_FAILURE;
				}else {
					return RC_SUCCESS;
				}
			}
		}else {
			LOG(ERROR,"Failed to get file information from %s", server_topology->node->IP);
			printf("Failed to get file information from %s", server_topology->node->IP);
		}



	}
	return RC_FAILURE;
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



        		fileInfo = (fileInfoPayload *)my_data->return_data;
    			if (fileInfo->flags & FILE_NOT_FOUND ) {
    				printf("\n File could not be found on the distributed file system");

    			} else {
    				file_thread = calloc(1, sizeof(dfs_thread_info *) * fileInfo->noOfSplits);
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
    						LOG(DEBUG,"Could not receive file split %d from any of the replicas", i);

    						break;
    					}
    				}
    				if (i != fileInfo->noOfSplits) {
    					printf("Failed to receive splits from nodes");
    					for (i = 0; i <  fileInfo->noOfSplits; i++) {
    						free(file_thread[i]);
    						//free(threads[i]);
    					}
    					//if (file_thread[i]->ip) {
    						//free(file_thread[i]->ip);
    					//}
    					free(my_data);
    					free(file_thread);
    					free(threads);
    					free(fileInfo);

    					return RC_FAILURE;

    				}

    				DEBUG(("********************TRYING TO MERGE******************************"));
    				//getchar();
    				//Merge the files
    				merge_file_splits(remoteFileName , localFileName, fileInfo->noOfSplits);
    				//getchar();
    				DEBUG(("********************DONE MERGING******************************"));
    			}
		}
	}
    DEBUG(("********************Returning Success******************************"));
    getchar();
    return RC_SUCCESS;
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

        	if (sendFileRequest(sock, dfs_data->destFileName) == RC_SUCCESS) {
                while (data == NULL) {
                	if((rc = message_decode(sock, &packet)) == RC_SUCCESS) {


                	    processPacket(sock, packet, &data);
                	} else {
                		DEBUG(("Breaking out because of message decode"));
                		break;
                	}

                }
                if (data) {
                	if (*((RC_t *)data) == RC_SUCCESS ) {
                		LOG(DEBUG, "Fetched file  %s from %s successfully", dfs_data->destFileName, IP);
                		LOG(DEBUG, "Exiting the thread. I better exit", "");
                		dfs_data->rc = RC_SUCCESS;
                		free(data);
                		LOG(DEBUG, "Exiting the thread. I better exit ...... ..", "");
                		pthread_exit(NULL);
                	}
                }
            }

        }
    }
    DEBUG(("Came out"));
    if ( i < dfs_data->numOfAddresses) {
	    dfs_data->rc = RC_SUCCESS;
    }else {
        dfs_data->rc = rc;
    }
    if (data) {
        free(data);
        data = NULL;
    }
    DEBUG(("Returning from this function"));
    return rc;
}



RC_t sendFileWrapper(void *tdata ) {
    FILE *fp;
    dfs_thread_info_new *dfs_data = (dfs_thread_info_new *)(tdata);
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
    nodeAddress->sin_port          = htons(DFS_LISTEN_PORT);
    if((*sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        LOG(ERROR, "IP : %s Unable to create TCP Socket. Dying...\n", IP);
        printf("IP : %s Unable to create TCP Socket. Dying...\n", IP);
        return RC_SOCKET_CREATION_FAILED;

    }

    if((connect(*sock, (struct sockaddr *) nodeAddress,   sizeof(*nodeAddress))) < 0) {
            LOG(ERROR, "IP : %s Unable to connect with server %s . Dying ...\n", IP);
            return RC_SOCKET_CONNECT_FAILED;

    }
    LOG(DEBUG, "IP : %s Connected for FTP operation ...Address %0x\n", IP, nodeAddress);
    DEBUG(("IP : %s Connected for FTP operation ..Address %0x \n", IP, nodeAddress));

    return RC_SUCCESS;
}


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
     sprintf(command, "split -d -b %lu -a 4 %s %s",CHUNK_SIZE_IN_MB * 1024 * 1024, localFileName, fileName);
     system(command);
     return RC_SUCCESS;


}

RC_t merge_file_splits(char *fileName ,char *localFileName, int numOfSplits)
{
     char command[1000];
     sprintf(command, "cat %s???? > %s", fileName, localFileName );
     system(command);
     //DEBUG(("********************DONE MERGING******************************"));
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
		if( (ptr == NULL) || ((ptr != NULL) && (ptr->flags & FILE_INFO_TENTATIVE))) {
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

			addFileMetaInfo(request->fileName, request->fileSize, FILE_INFO_TENTATIVE, numChunks, request->requesterIP);
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

    tmp = getFileMetadataPtr(infoPayload->fileName);

    if(tmp != NULL) {
    	if (infoPayload->flags & FILE_CHUNKS_PLACED_SUCCESSFULLY) {
    		//Chunks placed successfully. Mark entry as valid
    		tmp->flags = FILE_INFO_FINALIZED;

    		infoPayload->flags |= ENTRY_VALID;
    		LOG(DEBUG,"Marking entry for %s as active", infoPayload->fileName);
    	}else {
    		infoPayload->flags &= ~ENTRY_VALID;
    		LOG(DEBUG, "Deleting Meta Info entry for file %s ", infoPayload->fileName);
    		removeFileMetaInfo(infoPayload->fileName);
    	}
    }else {
    	LOG(DEBUG,"Received file info update payload for %s which was not found", infoPayload->fileName);
    }

    return rc;

}


RC_t sendFileDelete(char *fileName) {

	    char (*IP)[16];
	    IP = ipAddrList;//Get from Sachin
	    int numOfNodesToSend = 0 ; //Calculate
	    int index = 0;
	    int i;
	    thread_data *my_data[5];
	    pthread_t   thread[5];
	    int threads_created = 0;
	    chunkOperationPayload *payloadBuf = calloc(1,sizeof(chunkOperationPayload));

	    strcpy(payloadBuf->chunkName, fileName);
	    payloadBuf->flags |= DELETE_REPLICA;



	    while(index < numOfNodesToSend ) {
	        threads_created = 0;
	        for (i=0; i<5 && index < numOfNodesToSend; i++, index ++, threads_created++) {

	 	    //printf("Num nodes to send = %d", numOfNodesToSend);
	            //my_data[i].ip[15] = 0;
	            my_data[i] = calloc(1, sizeof(thread_data) + sizeof(chunkOperationPayload));
	            (*my_data[i]).payload = calloc(1, sizeof(chunkOperationPayload));
	            memcpy((*my_data[i]).ip, IP, 16);
	            IP++;
	            (*my_data[i]).payload_size = sizeof(chunkOperationPayload);
	            (*my_data[i]).msg_type = MSG_CHUNK_OPERATION;
	            memcpy((*my_data[i]).payload, payloadBuf, sizeof(chunkOperationPayload));
	            pthread_create(&thread[i], NULL, send_node_update_payload, (my_data[i]));
	        }
	        for (i=0 ; i < threads_created; i++) {
	            pthread_join(thread[i],NULL);
	        }
	    }
	    free(payloadBuf);

	return RC_SUCCESS;

}

RC_t dfs_delete_file(char *fileName) {

	thread_data *my_data;
	pthread_t thread;
	fileOperationRequestPayload *payloadBuf;
	fileInfoPayload *fileInfo;
    RC_t rc;


	int i;

    if (server_topology && server_topology->node ) {
    	my_data = calloc(1, sizeof(thread_data) + sizeof(fileOperationRequestPayload));
        (*my_data).payload = calloc(1, sizeof(fileOperationRequestPayload));

        memcpy((*my_data).ip, server_topology->node->IP, 16);

        (*my_data).payload_size = sizeof(fileOperationRequestPayload);
        (*my_data).msg_type = MSG_FILE_OPERATION_REQUEST;
        payloadBuf = my_data->payload;
        //memcpy((*my_data).payload, payloadBuf, sizeof(fileOperationRequestPayload));
        strcpy(payloadBuf->fileName, fileName);
        memcpy(payloadBuf->requesterIP, myself->IP, 16);
        payloadBuf->flags |= DEL_FILE_REQUEST;

        LOG(DEBUG,"Sending File Delete Operation Request to %s for %s", my_data->ip, payloadBuf->fileName);
        pthread_create(&thread, NULL, send_node_update_payload, (my_data));
        pthread_join(thread, NULL);

        if (my_data->status == RC_SUCCESS) {
        	LOG(DEBUG,"Sent File Delete Operation Request to %s for %s", my_data->ip, payloadBuf->fileName);
            rc = RC_SUCCESS;

        }
    }

    return rc;
}

