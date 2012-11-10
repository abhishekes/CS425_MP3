#include "send_file.h"
extern char *myIP;

#define MAX_FILE_CHUNK_PAYLOAD_SIZE 29900
#define MAX_FILE_CHUNK_SIZE 29600
/*********************************************************
** This function is used to send file 
** 
** Arguments:
** socket   : socket on which the message was received
** fileName : Name of file.
***********************************************************/

int sendFile(int socket, char *fileName, char *destFileName )
{
    int fp = open(fileName, O_RDONLY);
    int rc;
    DEBUG(("\nSendFile:Opened file %s\n", fileName));
    //char fileBuf[1000]={0};                                //File buffer
    int bytesSent = 0;
    fileTransferPayload *ftpBuf = (fileTransferPayload *) malloc(MAX_FILE_CHUNK_PAYLOAD_SIZE);
    int seqNo = 0;
    if (fp == 0) {
	
    	LOG(DEBUG, "SendFile:File %s Not found\n", fileName);
        return RC_FILE_NOT_FOUND;
    }
    DEBUG(("\nSendFile:\n"));
    if (destFileName != NULL) {
    	strcpy(ftpBuf->fileName, destFileName);
    }else {
    	strcpy(ftpBuf->fileName, fileName);
    }
    ftpBuf->statusFlag |= FTP_REQUEST;
    do {
    	 memset(ftpBuf->filePayload,0,MAX_FILE_CHUNK_SIZE);
         bytesSent = read(fp, ftpBuf->filePayload,sizeof(char) * MAX_FILE_CHUNK_SIZE);
         DEBUG(("\nSendfile: %s, RC = %d\n", ftpBuf->fileName , bytesSent));
         if (bytesSent <= 0) {
             return RC_FILE_NOT_FOUND;
         }
         if (!seqNo) {
             ftpBuf->statusFlag |= FTP_START;
         }
         if (bytesSent < (sizeof(char) * MAX_FILE_CHUNK_SIZE)) {
        	 DEBUG(("\nSending LAST CHUNK ***************************\n"));
        	 ftpBuf->statusFlag |= FTP_STOP;
         }
         ftpBuf->statusFlag = htons(ftpBuf->statusFlag);
         memcpy(ftpBuf->filePayload, ftpBuf->filePayload, bytesSent);
         rc = sendPayload(socket, MSG_FILE_TRANSFER, ftpBuf, sizeof(fileTransferPayload) + bytesSent);
         if (rc != RC_SUCCESS) {
             DEBUG(("\nCould not send script file\n"));
             free(ftpBuf);
             return rc;
         }

         ftpBuf->statusFlag = 0;
         seqNo++;
      }while(bytesSent == MAX_FILE_CHUNK_SIZE);
     free(ftpBuf);
     close(fp);
     //LOG(DEBUG, "SendFile:Just before returning %s\n", destFileName);
     return RC_SUCCESS;

}
