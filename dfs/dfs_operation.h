#ifndef __DFS_OPERATION_H__
#define __DFS_OPERATION_H__

#define IP_ADDR_LENGTH  16
#define NUM_OF_SPLITS   4
#define NUM_OF_REPLICAS 2

typedef struct {
	char fileName[255];
    char splitInfo[NUM_OF_SPLITS][NUM_OF_REPLICAS][IP_ADDR_LENGTH];
}file_info;


#endif
