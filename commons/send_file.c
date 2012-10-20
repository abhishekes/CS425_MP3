#include "send_file.h"

/*********************************************************
** This function is used to send file 
** 
** Arguments:
** socket   : socket on which the message was rceived
** fileName : Name of file.
***********************************************************/

int sendFile(int socket, char *fileName )
{
    FILE *fp = fopen(fileName, "r");
    int rc;
    DEBUG(("\nSendFile:Opened file %s\n", fileName));
    char fileBuf[1000]={0};                                //File buffer
    
    fileTransferPayload *ftpBuf = (fileTransferPayload *) malloc(1000);
    int seqNo = 0;
    if (fp == NULL) {
	
    	DEBUG(("\nSendFile:File %s Not found\n", fileName));
        return RC_FILE_NOT_FOUND;
    }
    DEBUG(("\nSendFile:\n"));
    strcpy(ftpBuf->fileName, fileName);
    ftpBuf->statusFlag |= FTP_REQUEST;
    do {
    	 memset(ftpBuf->filePayload,0,790);
         rc = fread(fileBuf,sizeof(char), 790, fp);
	 DEBUG(("\nSendfile: RC = %d\n", rc));
         if (rc < 0) {
             return RC_FILE_NOT_FOUND;
         }
         if (!seqNo) {
             ftpBuf->statusFlag |= FTP_START;
         }
         if (feof(fp)) {
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
      }while(!feof(fp));
     free(ftpBuf);
     fclose(fp);
     DEBUG(("\nSendFile:Just before returning\n"));
     return RC_SUCCESS;

}
