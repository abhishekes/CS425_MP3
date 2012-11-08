#include "file_metadata.h"

#define FileMetaType 0x01
#define IPtoFileType 0x02
#define FileListType 0x03
#define ChunkInfoType 0x04

FileMetadata *gFileMetaData = NULL; //This is the global File Metadata Pointer
IPtoFileInfo *gIPToFileInfo = NULL; //This is the global IP to File info Pointer

void* init_list_node(int type) {
	FileMetadata *temp1 = NULL;
	IPtoFileInfo *temp2 = NULL;
	FileList	 *temp3 = NULL;
	ChunkInfo	 *temp4 = NULL;

	if(type == FileMetaType) {
		temp1 = (struct FileMetadata*) calloc (1, sizeof(FileMetadata));
		return temp1;
	}
	else if (type == IPtoFileType) {
		temp2 = (struct IPtoFileInfo*) calloc (1, sizeof(IPtoFileInfo));
		return temp2;
	}
	else if (type == FileListType) {
		temp3 = (struct FileList*) calloc (1, sizeof(FileList));
		return temp3;
	}else if (type == ChunkInfoType){
		temp4 = (struct ChunkInfo*) calloc (1, sizeof(ChunkInfo));
	}

	return NULL;
}

int addChunkInfo(unsigned chunkNumber, char IP1[16], char IP2[16], char IP3[16], FileMetadata *ptr) {
	ChunkInfo *temp = (ChunkInfo *)init_list_node(ChunkInfoType);

	if( temp == NULL) return RC_FAILURE;

	temp->chunkNumber = chunkNumber;
	strcpy(temp->IP[0], IP1);
	strcpy(temp->IP[1], IP2);
	strcpy(temp->IP[2], IP3);
	temp->next = ptr->chunkInfo;
	ptr->chunkInfo = temp;

	updateIPtoFileInfo(IP1, ptr);
	updateIPtoFileInfo(IP2, ptr);
	updateIPtoFileInfo(IP3, ptr);

	return RC_SUCCESS;
}

RC_t addFileMetaInfo(char fileName[NAMEMAX], uint64_t size, uint64_t flags, uint64_t numberOfChunks) {
	int i;
	FileMetadata *temp = (FileMetadata*) init_list_node(FileMetaType);

	if(temp == NULL) return RC_FAILURE;

	temp->next = gFileMetaData;
	gFileMetaData = temp;
	temp->flags = flags;
	temp->numberOfChunks = numberOfChunks;
	temp->size = size;

	for(i=0; i<numberOfChunks; i++) {
		//TODO : Need to get the IP Address from server topology
		//addChunkInfo(i+1, NULL, NULL, NULL, temp);
	}

	return RC_SUCCESS;
}

//This initializes and empty IPtoFileInfo structure, i.e. its FileList info is NULL
int addIPtoFileInfo(char IP[16]) {
	IPtoFileInfo *ptr = NULL;

	ptr = getIPtoFileInfo(IP);

	if(ptr != NULL) {
		printf("addIPtoFileInfo: IPtoFileInfo for %s already present", IP);
		return RC_FAILURE;
	}

	ptr = init_list_node(IPtoFileType);
	strcpy(ptr->IP, IP);
	ptr->next = gIPToFileInfo;
	gIPToFileInfo = ptr;

	return RC_SUCCESS;
}

IPtoFileInfo *getIPtoFileInfo(char IP[16]) {
	IPtoFileInfo ptr = NULL;

	ptr = gIPToFileInfo;

	while(ptr != NULL && strcmp(ptr->IP, IP))
		ptr = ptr->next;

	return ptr;
}

RC_t updateFileList(IPtoFileInfo *ptr,FileMetadata *fileMetaPtr) {
	FileList *tempFileList = NULL;

	if (ptr == NULL) {
		printf("updateFileMetaPtr : ptr is NULL");
		return RC_FAILURE;
	}

	tempFileList = ptr->metadataPtr;

	while(tempFileList != NULL && tempFileList->fileMetaPtr != fileMetaPtr)
		tempFileList = tempFileList->next;

	if(tempFileList == NULL) { //Need to add a new fileList
		tempFileList = init_list_node(FileListType);
		tempFileList->fileMetaPtr = fileMetaPtr;
		ptr->numberOfFiles++;
	}

	return RC_SUCCESS;
}


RC_t updateIPtoFileInfo(char IP[16], FileMetadata* fileMetaPtr) {
	IPtoFileInfo ptr = NULL;

	ptr = getIPtoFileInfo(IP);

	if(ptr == NULL) { //The required IPtoFile struct for this IP is not present.
		(void)addIPtoFileInfo(IP);
		ptr = getIPtoFileInfo(IP);
	}

	return (updateFileList(ptr, fileMetaPtr));
}
