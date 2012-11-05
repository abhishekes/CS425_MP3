#ifndef __DFS_PAYLOAD_PROCESS_H__
#define __DFS_PAYLOAD_PROCESS_H__
#include "../commons/message_type.h"
#include "../commons/list.h"
#include "../dfs/dfs_operation.h"
RC_t processFileInfoPayload(fileInfoPayload* file_info);
RC_t processNodeFileInfoRequest(int socket, requestNodeFileInfo *payload);
RC_t processChunkReplicationPayload(int socket, chunkReplicatePayload *payload);
#endif

