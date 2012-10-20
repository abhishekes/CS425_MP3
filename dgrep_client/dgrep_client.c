#include "dgrep_client.h"

/*********************************************************
** This program is the query logging application. 
** The log query application connects to all the remote
** machines whose IP addresses are present in the serverIP
** list and executes the grep command on them.
***********************************************************/

char command[1000];

/* ********************** IP ADDRESS ARRAY (hardcoded) **************************  
The IP addresses of the remote machines are hardcoded here. The IP address of he
machine where the log query application is runing also needs to be added in the array
if results are  grep needs to be executed on those machines as well/
* ******************************************************************************/

char serverIP[4][16] = { "192.17.251.144", "192.168.2.45", "192.168.2.5", "192.168.2.8"};

void displayFile(char *fileName);

void *doDGrep(void *tdata) {
	
	struct threadData *myData = (struct threadData *)tdata;
	struct sockaddr_in serverAddress;
	char *serverIP = myData->ip;
	int myID = myData->threadID;
	int sock;
        int rc = FAILURE;
        int responseReceived = 0;
        int retryCount = -1;
        payloadBuf *packet;
        char fileName[]="grep_script";	
      
        memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family 	= AF_INET;
	serverAddress.sin_addr.s_addr	= inet_addr(serverIP);
	serverAddress.sin_port		= htons(SERVER_PORT);
	
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("Thread %d: Unable to create TCP Socket. Dying...\n", myID);
		myData->statusFlag = (FAILURE | SOCKET_CREATION_FAILURE);
                pthread_exit(NULL);	
	}
	
	if((connect(sock, (struct sockaddr *) &serverAddress, 	sizeof(serverAddress))) < 0) { 
		printf("Thread %d: Unable to connect with server %s . Dying ...\n", myID, serverIP);	
                myData->statusFlag = (FAILURE | SOCKET_CONNECT_FAILURE);
                pthread_exit(NULL);
	}
        DEBUG(("Thread 	%d : Connection Established\n\n", myID));
	//We are now connected. Let the send receive begin... 
        do {
            DEBUG(("\nTrying to send\n"));
            rc = sendScriptFile(sock, myData->command);
            retryCount++;
        }while(((rc != RC_SUCCESS) && (retryCount < 5)) && (rc != RC_SOCKET_WRITE_FAILURE));
        DEBUG(("\n *** Send Script file succeeded \n"));
        if (rc != RC_SUCCESS) {
            printf("Grep on machine %d could not be executed because the script file could not"
                   " transferred to the remote system. Error :%d ", myID, rc);
            myData->statusFlag = (FAILURE | FILE_TRANSFER_FAILURE );
            if (rc == RC_SOCKET_WRITE_FAILURE) {
                myData->statusFlag |= SOCKET_WRITE_FAILURE;
            }
            pthread_exit(NULL);
        }
        
        retryCount = -1;
        do {
            DEBUG(("\n In execute command***i\n"));
            rc = sendExecuteScriptCommand(sock, fileName);
            retryCount++;
        }while(((rc != RC_SUCCESS) && (retryCount < 5) && ((rc != RC_SOCKET_WRITE_FAILURE))));

        if (rc != RC_SUCCESS) {
            printf("Grep on machine %d could not be executed because the script file on the system could not be executed"
                   " Error :%d ", myID, rc);
            myData->statusFlag = (FAILURE | FILE_EXECUTION_FAILURE );
            if (rc == SOCKET_WRITE_FAILURE) {
                myData->statusFlag |= SOCKET_WRITE_FAILURE;
            }

            pthread_exit(NULL);
        }
        DEBUG(("****** Sent execute script ***** "));
        do {
            rc = message_decode(sock, &packet);
            if (rc != RC_SUCCESS) {
                break;
            }
            processPacket(sock, packet);        
        }while(1);
         
        myData->statusFlag = SUCCESS;        	
	printf("\n"); //This is to flush all in the buffer.. 
	
	pthread_exit(NULL);
	
}

int main(int argc, char *argv[]) {

	int i;
	pthread_t threads[MAX_THREADS];	
	struct threadData tData[MAX_THREADS];
	
	if (argc < 2 ) {	
		printf("Usage: ./dgrep [options] <grep pattern> \n");
		return 0;
	}
	
	//Populate te command to be sent to the server
        //ELLORES2
        strcpy(command, "grep -n ");
        for (i = 1; i < argc; i++) {
            if(strlen(command) + strlen(argv[i]) > 900) {
                printf("\nArguments list too long. Exiting\n\n ");
                return -1;
            }
            
            strcat(command, argv[i]);
            strcat(command, " ");
        }
        

	for(i = 0; i < NO_OF_CONNECTIONS ; i++) {
		tData[i].threadID = i + 1;
		tData[i].ip = serverIP[i];
                sprintf(tData[i].command, "##command_output_%d.txt\n%s machine.%d.log &> command_output_%d.txt",i+1, command, i+1, i+1); 
		puts(tData[i].command);
                (void)pthread_create(&threads[i], NULL, doDGrep, (void*)&tData[i]);
	}
        
        for (i=0; i < NO_OF_CONNECTIONS; i++) {
            (void)pthread_join(threads[i], NULL);
        }
        for (i =0; i < NO_OF_CONNECTIONS; i++) {
            if (tData[i].statusFlag & SUCCESS) {
                 printf("\nSTATUS: grep command executed on Machine with IP %s", serverIP[i]);
                 sprintf(command, "command_output_%d.txt", i+1);
                 displayFile(command);
            }else {
                 printf("\nSTATUS: grep command failed to execute on Machine with IP %s\n", serverIP[i]);
            }    

        }

	return 0;
}


//Assumes socket conection is established
int sendScriptFile(int socket, char *command )
{
    FILE *fp = fopen("bash_template.txt", "r");
    char fileBuf[1000]={0};                                //File buffer
    char msgBuf[1000] = {0} ;                              //Buffer to hold strings as and when they are read.
    char *filename = "execute_script";
    int msgLength = 0;
    int fileLength = 0;
    int seqNo = 0;
    int rc;
    char fileName[] = "grep_script";
    fileTransferPayload *ftpBuf = (fileTransferPayload *) malloc(1024);
     
    if (fp == NULL) {
        return RC_FILE_NOT_FOUND;
    }
    memset(ftpBuf,0,1024);
    ftpBuf->statusFlag |= FTP_REQUEST; 
    puts("Trying to send script file");
    do {
       fgets(msgBuf, 1000, fp);
       puts(msgBuf);
       strncpy(ftpBuf->fileName, fileName, FILE_PATH_LENGTH);

       if (!strcmp(msgBuf, "##REPLACE USER COMMAND\n"))  {
         puts("Comparison success");
         strncpy(msgBuf, command, 1000);
       } 
         if ((strlen(msgBuf) + msgLength) > 1000) {
             if (seqNo == 0) {
                 ftpBuf->statusFlag |= FTP_START;   
             }
             ftpBuf->statusFlag = htons(ftpBuf->statusFlag);
             memcpy(ftpBuf->filePayload, fileBuf, msgLength);
             DEBUG(("\n1.Sending %s\n", fileBuf));
             rc = sendPayload(socket, MSG_FILE_TRANSFER, ftpBuf, msgLength + sizeof(fileTransferPayload));
             if (rc != RC_SUCCESS) {
                 printf("\nCould not send script file\n");
                 free(ftpBuf);
                 return rc;
             }
             msgLength = 0;    
             ftpBuf->statusFlag = 0;
             seqNo++; 
         }  
         memcpy(fileBuf + msgLength, msgBuf, strlen(msgBuf));
         msgLength += strlen(msgBuf);         
              
    }while(!feof(fp));
    if (seqNo == 0) {
        ftpBuf->statusFlag |= FTP_START;
    }
    ftpBuf->statusFlag |= FTP_STOP;
    DEBUG(("\nBefore = %0x\n", ftpBuf->statusFlag));
    ftpBuf->statusFlag = htons(ftpBuf->statusFlag);
    DEBUG(("\nAfter = %0x\n", ftpBuf->statusFlag));
    memcpy(ftpBuf->filePayload, fileBuf, msgLength);
    DEBUG(("\n2.Sending %s :: %s, flag = %0x \n", fileBuf, ftpBuf->filePayload, ftpBuf->statusFlag));
    rc = sendPayload(socket, MSG_FILE_TRANSFER, ftpBuf, msgLength+sizeof(fileTransferPayload));
    free(ftpBuf);
    if (rc != RC_SUCCESS) {
        DEBUG(("\nCould not send script file\n"));
        return rc;
    }
    DEBUG(("\nReturning++++++\n"));
    return RC_SUCCESS;
 
}

int sendExecuteScriptCommand(int socket, char *fileName)
{
    DEBUG(("\nCrashing Allocating memory\n"));
    executeScriptPayload *payloadBuf = (executeScriptPayload *)malloc(sizeof(executeScriptPayload));
    int rc;
    strncpy(payloadBuf->scriptName, fileName, FILE_PATH_LENGTH);
    rc = sendPayload(socket, MSG_EXECUTE_SCRIPT, payloadBuf, sizeof(executeScriptPayload));
    free(payloadBuf); 
    return rc;
}

void displayFile(char *fileName) {
    FILE *fp = fopen(fileName, "r");
    char line[1000];
    if (fp == NULL) {
        return;
    }
    do {
       fgets(line, 1000,fp );
       puts(line);
       
    }while(!feof(fp));
} 
