#include "dfs_read_from_file.h"
#define METADATA_FILE "./metadata_file"

extern FileMetadata *gFileMetaData;
extern IPtoFileInfo *gIPToFileInfo;
extern char myIP[16];

RC_t dfs_read_from_file() {

	FileMetadata *fileMetaPtr, *dummyPtr;
	IPtoFileInfo *ipToFilePtr;
	ChunkInfo *chunkInfo, *tempChunkInfo;
	FileList *fileListPtr;
	FILE *fPtr;
	char fname[255];
	char tempIP[16];

	char line[129];
	unsigned i = 0;

	fPtr = fopen(METADATA_FILE, "r");
	fileMetaPtr = gFileMetaData;
    if (fPtr == NULL) {
    	LOG(DEBUG, "Could not find metadata file. Will start in a fresh state", "");
    	return RC_FILE_NOT_FOUND;
    }
	while(fgets(line, 128, fPtr) != NULL) {

		if(strstr(line, "FILENAME") != NULL) { //New FileMetaPtr;
			fileMetaPtr = (FileMetadata*)calloc(1, sizeof(FileMetadata));
			fileMetaPtr->next = gFileMetaData;
			gFileMetaData = fileMetaPtr;
			sscanf(line, "FILENAME:%s", fileMetaPtr->fileName);
		} else if(strstr(line, "SIZE") != NULL) {
			sscanf(line, "SIZE:%u", &fileMetaPtr->size);
		} else if(strstr(line, "FLAGS") != NULL) {
			sscanf(line, "FLAGS:%u", &fileMetaPtr->flags);
		} else if(strstr(line, "NCHUNKS") != NULL) {
			sscanf(line, "NCHUNKS:%u", &fileMetaPtr->numberOfChunks);
		} else if(strstr(line, "NREPLICAS") != NULL) {
			sscanf(line, "NREPLICAS:%u", &fileMetaPtr->numReplicas);
		} else if(strstr(line, "CHUNKNUM")) { //It is a chunk number belonging to the current fileMetaPtr
			chunkInfo = (ChunkInfo*)calloc(1, sizeof(ChunkInfo));
			sscanf(line, "CHUNKNUM:%u", &chunkInfo->chunkNumber);
			tempChunkInfo = fileMetaPtr->chunkInfo;
			while(tempChunkInfo && tempChunkInfo->next != NULL) 
				tempChunkInfo = tempChunkInfo->next;

			tempChunkInfo->next = chunkInfo;

		}else if((strstr(line, "IP") != NULL) && (strstr(line, "IP:") == NULL)) {
			sscanf(line, "IP%u:%s", &i, tempIP);
			strcpy(chunkInfo->IP[i], tempIP);
			//DEBUG(("Reading i = %u, IP = %s", i, chunkInfo->IP[i]));
		}else if(strstr(line, "SKIP:NEXT") != NULL) {
			continue;
		}else if((strstr(line, "IP") != NULL) && (strstr(line, "IP:") != NULL)) { // new IP entry
			ipToFilePtr = (IPtoFileInfo*)calloc(1, sizeof(IPtoFileInfo));
			ipToFilePtr->next = gIPToFileInfo;
			gIPToFileInfo = ipToFilePtr;
			sscanf(line, "IP:%s", ipToFilePtr->IP);
		}else if(strstr(line, "NFILES") != NULL) {
			sscanf(line, "NFILES:%u", &ipToFilePtr->numberOfFiles);
		}else if(strstr(line, "FNAME") != NULL) { // new File list
			fileListPtr = (FileList*)calloc(1, sizeof(FileList));
			fileListPtr->next = ipToFilePtr->metadataPtr;
			ipToFilePtr->metadataPtr = fileListPtr;
			sscanf(line, "FNAME:%s", fname);
			dummyPtr = gFileMetaData;

			while((dummyPtr != NULL) && strcmp(dummyPtr->fileName, fname))
				dummyPtr = dummyPtr->next;

			if(dummyPtr == NULL) {
				printf("\n\nERROR : This cannot be\n\n");
			}

			fileListPtr->fileMetaPtr = dummyPtr;
		}

	}

	fclose(fPtr);

	return RC_SUCCESS;
}
