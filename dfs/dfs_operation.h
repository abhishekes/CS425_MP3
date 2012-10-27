#ifndef __DFS_OPERATION_H__
#define __DFS_OPERATION_H__
#include "../commons/message_type.h"
#include "../node/node_send_info.h"
#include <stdint.h>
#include <stdlib.h>

#define IP_ADDR_LENGTH  16
#define NUM_OF_SPLITS   4
#define NUM_OF_REPLICAS 2

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

 RC_t dfs_file_transfer (fileOperation op, char *localFileName, char *destinationFileName);
#endif
