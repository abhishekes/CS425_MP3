#include "packet_process.h"

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

void processPacket(int socket, payloadBuf *packet) {
	uint16_t packetType, packetLength, bytesToWrite, bytesWritten = 0;
	payloadBuf *packet_ptr = packet;
	uint16_t statusFlag;
	struct FileNameMap *ptr;
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
        //DEBUG(("\nIn processpacket. Packet type : %0x\n", packetType));			
	perform_marshalling(packetType, packet->payload);
        switch(packetType) {
		case MSG_HEARTBEAT :
                     processHeartbeatPayload(packet->payload);                     
                     break;
		
		case MSG_ADD_DELETE_NODE:
		     printf("\n************processPacket***************\n");
                     processNodeAddDeletePayload(packet->payload, packet->length);
                     topology_version++;
                     break;
                case MSG_TOPOLOGY_REQUEST:
                     processTopologyRequest(socket, packet->payload);
                     break;
                case MSG_FILE_TRANSFER :
                     ftpBuf = (fileTransferPayload *)(packet->payload);				
                     DEBUG(("Status flag before = %0x\n", ftpBuf->statusFlag));
                     statusFlag = ntohs(ftpBuf->statusFlag);
	   
		     if((statusFlag & FTP_START)) {
		    	    DEBUG(("\nFilename : %s\n", ftpBuf->fileName));
			    sprintf(command, "rm -rf %s", ftpBuf->fileName);                      //Delete old entries 
                            system(command);
			    ptr = add_entry(ftpBuf->fileName);
			    if (ptr == NULL) {
			         DEBUG(("\nprocessPacket : Add Entry failed\n"));	
			    }
		     }
		     else { //This has to be done for both continue and FTP_STOP
			    ptr = get_entry(ftpBuf->fileName);
			    if(ptr == NULL) {
			     	 DEBUG(("\nprocessPacket : Get Entry Failed\n"));
			  }
		     }
		
		     buf = (char*)(ftpBuf->filePayload);
                     bytesToWrite =  packetLength - sizeof(packetLength) - sizeof(packetType) - sizeof(fileTransferPayload);
		     while(bytesToWrite != 0) {
		   	    bytesWritten = write(ptr->fd, buf, bytesToWrite);
			    bytesToWrite -= bytesWritten;
                            buf += bytesWritten;
		     }
		
		     if (statusFlag & FTP_STOP) {
		   	close(ptr->fd);
			sprintf(command, "chmod 777 %s", ftpBuf->fileName);
			result = system(command);
			result = delete_entry(ptr->fd);
			if (result == -1)
				DEBUG(("\nprocessPacket : Delete entry failed\n"));
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
 	      	     sendFile(socket, fileName);
			
		     close(socket);	
 		     break;
        	default	:
			printf("\nIn process packet.. but unknown type\n");
	}
	//DEBUG(("*** Freeing memory \n\n"));
        free(packet_ptr); // Freeing
}
