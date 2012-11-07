#ifndef __FILE_METADATA__
#define __FILE_METADATA__

#define NAMEMAX 255

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	unsigned chunkNumber;
	char IP[3][16];
}ChunkInfo;

typedef struct {
	char fileName[NAMEMAX];
	uint64_t size;
	uint64_t numberOfChunks;
	uint64_t flags;
	ChunkInfo *chunkInfo;
	struct FileMetadata *next;
}FileMetadata;

typedef struct {
	FileMetadata *fileMetaPtr;
	struct FileList *next;
}FileList;

typedef struct {
	char IP[16];
	uint64_t numberOfFiles;
	FileList *metadataPtr;
	struct IPtoFileInfo *next;
}IPtoFileInfo;

#endif //__FILE_METADATA__
