#include "dfs_payload_process.h"

extern fileInfoPayload fileInfo;

RC_t processFileInfoPayload(fileInfoPayload* fileInfo, file_info * info) {
     file_info = calloc(1, sizeof(file_info));
	 memcpy(fileInfo, file_info, sizeof(fileInfoPayload));
     return RC_SUCCESS;
}
