#include "dfs_payload_process.h"

extern fileInfoPayload fileInfo;

RC_t processFileInfoPayload(fileInfoPayload* file_info) {
     memcpy(fileInfo, file_info, sizeof(fileInfoPayload));
}
