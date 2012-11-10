#include "send_file.h"
extern char *myIP;
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
    char fileBuf[1000]={0};                                //File buffer
    
    fileTransferPayload *ftpBuf = (fileTransferPayload *) malloc(1000);
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
    	 memset(ftpBuf->filePayload,0,790);
         rc = read(fp, fileBuf,sizeof(char) * 790);
	 //LOG("\nSendfile: RC = %d\n", rc);
         if (rc <= 0) {
             return RC_FILE_NOT_FOUND;
         }
         if (!seqNo) {
             ftpBuf->statusFlag |= FTP_START;
         }
         if (rc < sizeof(char) * 790) {
             ftpBuf->statusFlag |= FTP_STOP;
         }
	 ftpBuf->statusFlag = htons(ftpBuf->statusFlag);
	 memcpy(ftpBuf->filePayload, fileBuf, rc);
	 rc = sendPayload(socket, MSG_FILE_TRANSFER, ftpBuf, sizeof(fileTransferPayload) + rc);
         if (rc != RC_SUCCESS) {
             DEBUG(("\nCould not send script file\n"));
             free(ftpBuf);
             return rc;
         }

         ftpBuf->statusFlag = 0;
         seqNo++;
      }while(rc == 790);
     free(ftpBuf);
     close(fp);
     //LOG(DEBUG, "SendFile:Just before returning %s\n", destFileName);
     return RC_SUCCESS;

}
