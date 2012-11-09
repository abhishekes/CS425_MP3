#include "dfs_read_from_file.h"
#define METADATA_FILE "./metadata_file"

extern FileMetadata *gFileMetaData;
extern IPtoFileInfo *gIPToFileInfo;


RC_t dfs_read_from_file() {

	FileMetadata *fileMetaPtr, *dummyPtr;
	IPtoFileInfo *ipToFilePtr;
	ChunkInfo *chunkInfo;
	FileList *fileListPtr;
	FILE *fPtr;
	char fname[255];

	char line[129];
	int i = 0;

	fPtr = fopen(METADATA_FILE, "r");
	fileMetaPtr = gFileMetaData;

	while(fgets(line, 128, fPtr) != feof) {

		if(strstr(line, "FILENAME") != NULL) { //New FileMetaPtr;
			fileMetaPtr = (FileMetadata*)calloc(1, sizeof(FileMetadata));
			fileMetaPtr->next = gFileMetaData;
			gFileMetaData = fileMetaPtr;
			sscanf(line, "FILENAME : %s", fileMetaPtr->fileName);
		} else if(strstr(line, "SIZE") != NULL) {
			sscanf(line, "SIZE : %u", &fileMetaPtr->size);
		} else if(strstr(line, "FLAGS") != NULL) {
			sscanf(line, "FLAGS : %u", &fileMetaPtr->flags);
		} else if(strstr(line, "NCHUNKS") != NULL) {
			sscanf(line, "NCHUNKS : %u", &fileMetaPtr->numberOfChunks);
		} else if(strstr(line, "NREPLICAS") != NULL) {
			sscanf(line, "NREPLICAS : %u", &fileMetaPtr->numReplicas);
		} else if(strstr(line, "CHUNKNUM")) { //It is a chunk number belonging to the current fileMetaPtr
			chunkInfo = (ChunkInfo*)calloc(1, sizeof(ChunkInfo));
			chunkInfo->next = fileMetaPtr->chunkInfo;
			fileMetaPtr->chunkInfo = chunkInfo;
			sscanf(line, "CHUNKNUM : %u", chunkInfo->chunkNumber);
		}else if((strstr(line, "IP") != NULL) && (strstr(line, "IP :") == NULL)) {
		}else if(strstr(line, "SKIP : NEXT") != NULL) {
			continue;
		}else if((strstr(line, "IP") != NULL) && (strstr(line, "IP :") != NULL)) { // new IP entry
			ipToFilePtr = (IPtoFileInfo*)calloc(1, sizeof(IPtoFileInfo));
			ipToFilePtr->next = gIPToFileInfo;
			gIPToFileInfo = ipToFilePtr;
			sscanf(line, "IP : %s", ipToFilePtr->IP);
		}else if(strstr(line, "NFILES") != NULL) {
			sscanf(line, "NFILES : %u", &ipToFilePtr->numberOfFiles);
		}else if(strstr(line, "FNAME") != NULL) { // new File list
			fileListPtr = (FileList*)calloc(1, sizeof(FileList));
			fileListPtr->next = ipToFilePtr->metadataPtr;
			ipToFilePtr->metadataPtr = fileListPtr;
			sscanf(line, "FNAME : %s", fname);
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