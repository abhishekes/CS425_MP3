#ifndef __DFS_PAYLOAD_PROCESS_H__
#define __DFS_PAYLOAD_PROCESS_H__
#include "../commons/message_type.h"
#include "../commons/list.h"
#include "../dfs/dfs_operation.h"
#include "../commons/file_metadata.h"
#include "../logging/log.h"



RC_t processNodeFileInfoRequest(int socket, requestNodeFileInfo *payload);
RC_t processChunkReplicationPayload(int socket, chunkReplicatePayload *payload);
RC_t processFileInfoPayload(fileInfoPayload *infoPayload, void ** return_data);
RC_t processFileInfoUpdatePayload(fileInfoPayload *);
RC_t sendFileInfoRequest(int socket, char *fileName);
RC_t sendFileRequest(int socket, char *fileName);
RC_t processFileOperationRequest(int socket, fileOperationRequestPayload *payload);

#endif

