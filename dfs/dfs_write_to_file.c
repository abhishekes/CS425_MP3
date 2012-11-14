#include "dfs_write_to_file.h"

extern FileMetadata *gFileMetaData;
extern IPtoFileInfo *gIPToFileInfo;

#define METADATA_FILE "./metadata_file"

RC_t dfs_write_to_file() {
	FileMetadata *fileMetaPtr;
	IPtoFileInfo *ipToFilePtr;
	FileList *fileListPtr;
	ChunkInfo *chunkInfo;
	FILE *fPtr;
	int i, j;

	system("rm -rf metadata_file");
	fPtr = fopen(METADATA_FILE, "w");
	fileMetaPtr = gFileMetaData;

	while(fileMetaPtr != NULL) {

		fprintf(fPtr, "FILENAME:%s\n", fileMetaPtr->fileName);
		fprintf(fPtr, "SIZE:%u\n", fileMetaPtr->size);
		fprintf(fPtr, "FLAGS:%u\n", fileMetaPtr->flags);
		fprintf(fPtr, "NCHUNKS:%u\n", fileMetaPtr->numberOfChunks);
		fprintf(fPtr, "NREPLICAS:%u\n", fileMetaPtr->numReplicas);

		chunkInfo = fileMetaPtr->chunkInfo;

		while(chunkInfo != NULL) {
			fprintf(fPtr, "CHUNKNUM:%u\n", chunkInfo->chunkNumber);
			for(i = 0; i < fileMetaPtr->numReplicas; i++) {
				fprintf(fPtr, "IP%d:%s\n", i, chunkInfo->IP[i]);
			}

			chunkInfo = chunkInfo->next;
		}

		fileMetaPtr = fileMetaPtr->next;
	}

	ipToFilePtr = gIPToFileInfo;

	if(ipToFilePtr != NULL) {
		fprintf(fPtr, "SKIP:NEXT\n");
	}

	while(ipToFilePtr != NULL) {
		fprintf(fPtr, "IP:%s\n", ipToFilePtr->IP);
		fprintf(fPtr, "NFILES:%u\n", ipToFilePtr->numberOfFiles);
		fileListPtr = ipToFilePtr->metadataPtr;
		for(j=0; j < ipToFilePtr->numberOfFiles && fileListPtr && fileListPtr->fileMetaPtr != NULL; j++ ) {
			fprintf(fPtr, "FNAME:%s\n", fileListPtr->fileMetaPtr->fileName);
			fileListPtr = fileListPtr->next;
		}

		ipToFilePtr = ipToFilePtr->next;
	}

	fclose(fPtr);

	sendMetadataToNeighbour();

	return RC_SUCCESS;
}
