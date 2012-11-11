#include "packet_process.h"

extern char myIP[16];
//Refer message_type.h for structure of the payload

/*********************************************************
** This is the payload process function. 
** It decodes the packet type and processes it accodingly. 
** 1
** Arguments:
** socket : socket on which the message was rceived
** packet : Packet to be processed.
***********************************************************/
extern topology_version;

void processPacket(int socket, payloadBuf *packet, void ** return_data) {
	uint16_t packetType, packetLength, bytesToWrite, bytesWritten = 0;
	payloadBuf *packet_ptr = packet;
	uint16_t statusFlag;
	struct FileNameMap *ptr;
	int wfp = -1;
	char fileName[FILE_PATH_LENGTH + 1];
	char outputFileName[FILE_PATH_LENGTH];
        fileTransferPayload *ftpBuf;
	executeScriptPayload *eSPayload;
	char command[200];
        int result;
        char *buf;
        FILE *fp;      
  	
	packetLength = packet->length;
	packetType = packet->type;
    if (packetType != 0) {
    	DEBUG(("\nIn processpacket. Packet type : %0x\n", packetType));
    }
	perform_marshalling(packetType, packet->payload);
        switch(packetType) {
		case MSG_HEARTBEAT :
                     processHeartbeatPayload(packet->payload);                     
                     break;
		case MSG_ADD_DELETE_NODE:
		        printf("\n************processPacket***************\n");
                     processNodeAddDeletePayload(packet->payload, packet->length - sizeof(payloadBuf));
                     topology_version++;
                     close(socket);
                     break;
		case MSG_TOPOLOGY_REQUEST:
                     processTopologyRequest(socket, packet->payload);
                     close(socket);
                     break;
		case MSG_FILE_TRANSFER :
                     ftpBuf = (fileTransferPayload *)(packet->payload);				
                     DEBUG(("Status flag before = %0x\n", ftpBuf->statusFlag));
                     statusFlag = ntohs(ftpBuf->statusFlag);

                     if((statusFlag & FTP_START)) {
                    	 DEBUG(("\nFilename : %s\n", ftpBuf->fileName));

                    	 sprintf(command, "rm -rf %s", ftpBuf->fileName);                      //Delete old entries
                    	 system(command);
                    	 wfp = open(ftpBuf->fileName, O_WRONLY | O_CREAT, 0666);
                     } else {
                    	 wfp = open(ftpBuf->fileName, O_WRONLY | O_APPEND, 0666);
                     }


                     while(wfp < 0) {
                    	 wfp = open(ftpBuf->fileName, O_WRONLY | O_APPEND, 0666);
                    	 DEBUG(("\n****************** $$$$$$$$$$$ processPacket : Get Entry Failed for %s \n", ftpBuf->fileName));
                    	 LOG(DEBUG,"****************** $$$$$$$$$$$ processPacket : Get Entry Failed for %s \n", ftpBuf->fileName);
                    	 //getchar();

                     }/*else */{
                    	 DEBUG(("\nprocessPacket : Got Entry for %s : %x\n", ftpBuf->fileName, wfp));
                     }

                     buf = (char*)(ftpBuf->filePayload);
                     bytesToWrite =  packetLength - sizeof(packetLength) - sizeof(packetType) - sizeof(fileTransferPayload);
                     while(bytesToWrite != 0) {
                    	 bytesWritten = write(wfp, buf, bytesToWrite);
                    	 bytesToWrite -= bytesWritten;
                    	 buf += bytesWritten;
                     }
                     close(wfp);
                     if (statusFlag & FTP_STOP) {
                         LOG(DEBUG, "Received stop for %s. Flags : %0x ", ftpBuf->fileName, statusFlag);
                    	 sprintf(command, "chmod 777 %s", ftpBuf->fileName);
                    	 result = system(command);

                    	 if (result == -1) {
                    		 DEBUG(("\nprocessPacket : Delete entry failed\n"));
                    	 }
                    	 if (return_data) {
                    		 *return_data = (RC_t *)malloc(sizeof(RC_t));
                    	 	 *((RC_t *)(*return_data)) = RC_SUCCESS;
                    	 }
                         LOG(DEBUG, "Received file %s", ftpBuf->fileName);


                         close(socket); //Close connection since last chunk is received
                     }

		break;
			
		case MSG_EXECUTE_SCRIPT :
		     eSPayload = (executeScriptPayload *)(packet->payload);
		     int result=-1;
			
		     printf("Am I coming here??\n");	
 	   	     sprintf(command, "ls %s", eSPayload->scriptName);
             result = system(command);
             strcpy(fileName, "error_file.txt");
             //The script file is present.
             puts(command);
             DEBUG(("Here 0 \n"));
             if (result == 0) {
                   sprintf(command, "chmod 777 %s", eSPayload->scriptName);
                   system(command);
                   fp = fopen(eSPayload->scriptName, "r");
                   fgets(outputFileName, FILE_PATH_LENGTH, fp);
                   fgets(outputFileName, FILE_PATH_LENGTH, fp);
                   fclose(fp);
                   DEBUG(("Here 1\n"));
                   puts(command);
                   outputFileName[strlen(outputFileName)-1] = 0;

		           sprintf(command, "rm -rf %s", outputFileName + 2);
		           system(command);
		         
		    
		           sprintf(command, "./%s", eSPayload->scriptName);
		           puts(command);
                   DEBUG(("Here 2\n"));
                   result = system(command);
		           DEBUG(("\nHere 3: %d %d %d %d\n", command[0], command[1], command[2], command[3]));
		           strcpy(fileName, outputFileName + 2);
             }
             sendFile(socket, fileName, NULL);
			
		     close(socket);	
 		     break;
		case MSG_FILE_REQUEST: //This is to tell Master that a client wants a file
			processFileRequest(socket, packet->payload);
			break;
		case MSG_FILE_OPERATION_REQUEST: //to tell the Master that a client has a file to put
            processFileOperationRequest(socket, packet->payload);
			break;
		case MSG_FILE_INFO: //This is where the Master decides where does a particular chunk-replica needs to go
			processFileInfoPayload(packet->payload, return_data);
			break;
		case MSG_NODE_FILE_INFO_REQEST: //Does nothing for now.
			//processNodeFileInfoRequest(socket, packet->payload);
			break;
		case MSG_CHUNK_OPERATION: //This is to tell failed node's neighbor to replicate content
			processChunkOperationPayload(socket, packet->payload);
			break;
        default	:
			    printf("\nIn process packet.. but unknown type\n");
			    break;
	}
	//DEBUG(("*** Freeing memory \n\n"));
        free(packet_ptr); // Freeing
}
