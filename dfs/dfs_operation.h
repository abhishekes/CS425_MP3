#ifndef __DFS_OPERATION_H__
#define __DFS_OPERATION_H__
#include "../commons/message_type.h"
#include "../node/node_send_info.h"
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "../logging/log.h"
#include "../commons/file_metadata.h"
#include <math.h>

#define IP_ADDR_LENGTH    16
#define NUM_OF_SPLITS      4
#define CHUNK_SIZE_IN_MB  64

typedef struct {
	char fileName[255];
    char splitInfo[NUM_OF_SPLITS][NUM_OF_REPLICAS][IP_ADDR_LENGTH];
    uint16_t flags;
	#define ENTRY_VALID 0x01

}file_info;

typedef enum {
	FILE_PUT,
	FILE_GET
}fileOperation;

typedef struct {
    int numOfAddresses;
    char (*ip)[16];
    char fileName[500];
    char destFileName[500];
    RC_t rc;

}dfs_thread_info;

RC_t dfs_file_transfer (fileOperation op, char *localFileName, char *destinationFileName);
RC_t dfs_replicate_chunk(char *fileName, char *ip);
RC_t create_file_splits(char *, char *fileName , int numOfSplits);
RC_t make_master(char *prevMasterIP);
RC_t populateFileInfoPayload(fileInfoPayload **payload, fileOperationRequestPayload *request);
RC_t merge_file_splits(char *fileName , char *localFileName, int numOfSplits);
RC_t createConnection(struct sockaddr_in *nodeAddress, char *IP, int *sock);
RC_t receiveFileWrapper(void *tdata);
RC_t sendFileWrapper(void *tdata);
RC_t create_metadata_from_file(void);
RC_t dfs_delete_file(char *fileName);
RC_t dfs_replicate_files_of_crashed_node(char *ip);
#endif
