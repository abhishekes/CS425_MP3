#ifndef __NODE_DATA_STRUCTURES_H__
#define __NODE_DATA_STRUCTURES_H__

#define NUM_HEARBEAT_NEIGHBOURS  2

typedef struct {
    char   ipAddr[16];
    time_t latestTimeStamp;
}neigbourHeartbeat;

/*Every node is going to maintain a circular linked list of nodes present in the ring currently  */
typedef struct {
    char ipAddr[16];
    struct nodeData *next;
    struct nodeData *prev;
}nodeData;
#endif
