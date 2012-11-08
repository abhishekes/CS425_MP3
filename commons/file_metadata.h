#ifndef __FILE_METADATA__
#define __FILE_METADATA__

#define NAMEMAX 255

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "message_type.h"
struct ChunkInfoStruct;
struct FileMetadataStruct;
struct IPtoFileInfoStruct;
struct FileListStruct;
struct ChunkInfoStruct{
	unsigned chunkNumber;
	char IP[2][16];
	struct ChunkInfoStruct *next;
};
typedef struct ChunkInfoStruct ChunkInfo;
struct FileMetadataStruct{
	char fileName[NAMEMAX];
	uint32_t size;
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
#endif //__FILE_METADATA__
