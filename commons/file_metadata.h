#ifndef __FILE_METADATA__
#define __FILE_METADATA__

#define NAMEMAX 255
#define MAXREPLICAS 2

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "message_type.h"
#include "list.h"

struct ChunkInfoStruct;
struct FileMetadataStruct;
struct IPtoFileInfoStruct;
struct FileListStruct;

struct ChunkInfoStruct{
	unsigned chunkNumber;
	char IP[MAXREPLICAS][16];
	struct ChunkInfoStruct *next;
};
typedef struct ChunkInfoStruct ChunkInfo;

struct FileMetadataStruct{
	char fileName[NAMEMAX];
	uint32_t size;
	unsigned numReplicas;
	uint32_t numberOfChunks;
	uint32_t flags;
	struct ChunkInfoStruct *chunkInfo;
	struct FileMetadataStruct *next;
};
typedef struct  FileMetadataStruct FileMetadata;

struct FileListStruct{
	FileMetadata *fileMetaPtr;
	struct FileListStruct *next;
};
typedef struct FileListStruct FileList;

struct IPtoFileInfoStruct{
	char IP[16];
	uint32_t numberOfFiles;
	FileList *metadataPtr;
	struct IPtoFileInfoStruct *next;
};
typedef struct IPtoFileInfoStruct IPtoFileInfo;

RC_t updateIPtoFileInfo(char IP[16], FileMetadata* fileMetaPtr);
void* init_list_node(int type);
RC_t addChunkInfo(unsigned chunkNumber, char IPs[MAXREPLICAS][16], FileMetadata *ptr);
RC_t addFileMetaInfo(char fileName[NAMEMAX], uint32_t size, uint32_t flags, uint32_t numberOfChunks, char* IP);
FileMetadata *getFileMetadataPtr(char fileName[NAMEMAX]);
IPtoFileInfo *getIPtoFileInfo(char IP[16]);
RC_t removeFileMetaEntry(char IP[16], FileMetadata *filePtr);
RC_t deleteAllChunkInfo(ChunkInfo *chunkPtr, FileMetadata *fileMetaPtr);
RC_t deleteFileMetaInfo(FileMetadata *fileMetaPtr);
RC_t removeFileMetaInfo(char fileName[NAMEMAX]);
int addIPtoFileInfo(char IP[16]);
RC_t updateFileList(IPtoFileInfo *ptr,FileMetadata *fileMetaPtr);
RC_t updateIPtoFileInfo(char IP[16], FileMetadata* fileMetaPtr);
void getNextIP_RR(char IP[16]);

#endif //__FILE_METADATA__
