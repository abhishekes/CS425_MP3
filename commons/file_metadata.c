#include "file_metadata.h"

#define FileMetaType 0x01
#define IPtoFileType 0x02
#define FileListType 0x03
#define ChunkInfoType 0x04

FileMetadata *gFileMetaData = NULL; //This is the global File Metadata Pointer
IPtoFileInfo *gIPToFileInfo = NULL; //This is the global IP to File info Pointer

extern char clientIP[16];
extern struct Head_Node *server_topology;


void* init_list_node(int type) {
	FileMetadata *temp1 = NULL;
	IPtoFileInfo *temp2 = NULL;
	FileList	 *temp3 = NULL;
	ChunkInfo	 *temp4 = NULL;

	if(type == FileMetaType) {
		temp1 = (FileMetadata*) calloc (1, sizeof(FileMetadata));
		return temp1;
	}
	else if (type == IPtoFileType) {
		temp2 = (IPtoFileInfo*) calloc (1, sizeof(IPtoFileInfo));
		return temp2;
	}
	else if (type == FileListType) {
		temp3 = (FileList*) calloc (1, sizeof(FileList));
		return temp3;
	}else if (type == ChunkInfoType){
		temp4 = (ChunkInfo*) calloc (1, sizeof(ChunkInfo));
		return temp4;
	}

	return NULL;
}

RC_t addChunkInfo(unsigned chunkNumber, char replicaIPs[MAXREPLICAS][16], FileMetadata *ptr) {
	ChunkInfo *temp = (ChunkInfo *)init_list_node(ChunkInfoType);
	int i = 0;
	if( temp == NULL) return RC_FAILURE;

	temp->next = ptr->chunkInfo;
	ptr->chunkInfo = temp;

	temp->chunkNumber = chunkNumber;
	for(i = 0; i < ptr->numReplicas; i++) {
		strcpy(temp->IP[i], replicaIPs[i]);
		updateIPtoFileInfo(temp->IP[i], ptr);
	}

	return RC_SUCCESS;
}



RC_t addFileMetaInfo(char fileName[NAMEMAX], uint32_t size, uint32_t flags, uint32_t numberOfChunks) {
	int i;
	FileMetadata *temp = NULL;
	char anotherIP[16];
	char replicaIPs[MAXREPLICAS][16];

	if(getFileMetadataPtr(fileName) != NULL) {
		printf("addFileMetaInfo : File already present, cannot add again");
		return RC_FAILURE;
	}

	temp = (FileMetadata*) init_list_node(FileMetaType);


	if(temp == NULL) return RC_FAILURE;

	strcpy(temp->fileName, fileName);
	temp->next = gFileMetaData;
	gFileMetaData = temp;
	temp->flags = flags;
	temp->numberOfChunks = numberOfChunks;
	temp->size = size;

	temp->numReplicas = (server_topology->num_of_nodes < MAXREPLICAS ? server_topology->num_of_nodes: MAXREPLICAS);


	for(i=0; i<numberOfChunks; i++) {
		for(i=0; i < temp->numReplicas; i++) {
			if(i==0) {
				strcpy(replicaIPs[0], clientIP);
			} else {
				getNextIP_RR(anotherIP);
				if(!strcmp(anotherIP, clientIP)) {
					getNextIP_RR(anotherIP);
				}

				strcpy(replicaIPs[i], anotherIP);
			}
		}

		addChunkInfo(i+1, replicaIPs, temp);
	}

	return RC_SUCCESS;
}

FileMetadata *getFileMetadataPtr(char fileName[NAMEMAX]) {
	FileMetadata *ptr = gFileMetaData;

	while(ptr != NULL && strcmp(ptr->fileName, fileName))
		ptr = ptr->next;

	return ptr;
}

IPtoFileInfo *getIPtoFileInfo(char IP[16]) {
	IPtoFileInfo *ptr = NULL;

	ptr = gIPToFileInfo;

	while(ptr != NULL && strcmp(ptr->IP, IP))
		ptr = ptr->next;

	return ptr;
}


RC_t removeFileMetaEntry(char IP[16], FileMetadata *filePtr) {
	IPtoFileInfo *ipToFileInfoPtr = NULL;
	FileList *temp, *prev;

	ipToFileInfoPtr = getIPtoFileInfo(IP);

	if(ipToFileInfoPtr == NULL)
		return RC_FAILURE;

	temp = ipToFileInfoPtr->metadataPtr;
	if(ipToFileInfoPtr->metadataPtr->fileMetaPtr == filePtr) {
		ipToFileInfoPtr->metadataPtr = ipToFileInfoPtr->metadataPtr->next;
	} else {
		while(temp != NULL && temp->fileMetaPtr != filePtr) {
			prev = temp;
			temp = temp->next;
		}
		if(temp != NULL) {
			ipToFileInfoPtr->numberOfFiles--;
			prev->next = temp->next;
			temp->fileMetaPtr = NULL;
			free(temp);
		}
	}

	return RC_SUCCESS;
}

RC_t deleteAllChunkInfo(ChunkInfo *chunkPtr, FileMetadata *fileMetaPtr) {
	int i=0;

	if( chunkPtr == NULL) {
		return RC_SUCCESS;
	} else {
		deleteAllChunkInfo(chunkPtr->next, fileMetaPtr);

		for(i=0; i<fileMetaPtr->numReplicas; i++) {
			removeFileMetaEntry(chunkPtr->IP[i], fileMetaPtr);
		}

		free(chunkPtr);

		return RC_SUCCESS;
	}
}

RC_t deleteFileMetaInfo(FileMetadata *fileMetaPtr) {

	(void)deleteAllChunkInfo(fileMetaPtr->chunkInfo, fileMetaPtr);
	free(fileMetaPtr);

	return RC_SUCCESS;
}

//This is the function called to delete a file
RC_t removeFileMetaInfo(char fileName[NAMEMAX]) {
	FileMetadata *ptr = gFileMetaData;
	FileMetadata *prev = NULL;

	ptr = getFileMetadataPtr(fileName);

	if(ptr == NULL) return RC_FAILURE;

	if(ptr == gFileMetaData) {
		gFileMetaData = gFileMetaData->next;
	}else{
		prev = gFileMetaData;
		while( prev != NULL && prev->next!=ptr )
			prev = prev->next;

		prev->next = ptr->next;
	}

	return (deleteFileMetaInfo(ptr));
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
	IPtoFileInfo *ptr = NULL;

	ptr = getIPtoFileInfo(IP);

	if(ptr == NULL) { //The required IPtoFile struct for this IP is not present.
		(void)addIPtoFileInfo(IP);
		ptr = getIPtoFileInfo(IP);
	}

	return (updateFileList(ptr, fileMetaPtr));
}


void getNextIP_RR(char IP[16]) {
	static int i = 0;
	int j;
	struct Node *myNodePtr;

	myNodePtr = server_topology->node;

	j = i;
	while(j--)
		myNodePtr = myNodePtr->next;

	strcpy(IP, myNodePtr->IP);

	i++;

	return;
}
