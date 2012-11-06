#ifndef __MESSAGE_TYPES__
#define __MESSAGE_TYPES__

#include <stdint.h>
#include "debug.h"
#include "time.h"

#define FILE_PATH_LENGTH                   200
#define MAX_ELEMENTS_PER_PAYLOAD             8
#define ID_SIZE                             20

#define NUM_OF_REPLICAS                      3

#define my_malloc(bytes) \
do \
{ \
  memset(malloc((bytes)) , 0 , (bytes)); \
}while(0) 


/*Macros used for Marshalling */
#define marshal_field_info(data_structure, field) \
    (int)(&(((data_structure *)0)->field)), sizeof(((data_structure *)0)->field)

/*Macro for demultiplexing IP from payload */
#define getIPAddress(ID) (ID + 4)


typedef enum {
   MSG_HEARTBEAT = 0,
   MSG_ADD_DELETE_NODE,
   MSG_TOPOLOGY_REQUEST,
   MSG_FILE_TRANSFER,
   MSG_EXECUTE_SCRIPT,
   MSG_FILE_OPERATION_REQUEST,
   MSG_FILE_INFO,
   MSG_FILE_REQUEST,
   MSG_NODE_FILE_INFO_REQEST,
   MSG_CHUNK_REPLICATION,
   NUM_OF_PAYLOADS
}messageType;

typedef struct 
{
   uint16_t length;
   uint16_t type;
   char payload[0]; 
}payloadBuf;

typedef struct 
{
    uint32_t   heartbeatTimeStamp;
    char       ip_addr[16];    
}heartbeatPayload;

typedef struct
{   
    uint16_t transferId;
    uint16_t statusFlag;
    #define FTP_REQUEST  0x01
    #define FTP_RESPONSE 0x02
    #define FTP_START    0x04
    #define FTP_STOP     0x08
    #define FTP_SUCCESS  0x10
    char fileName[FILE_PATH_LENGTH];
    char filePayload[0];
}fileTransferPayload;

typedef struct
{
    char scriptName[FILE_PATH_LENGTH];
}executeScriptPayload;

typedef struct
{
  uint8_t numOfNodes;
  uint8_t flags;
  #define ADD_PAYLOAD           0x01
  #define DELETE_PAYLOAD        0x02
  #define LEAVE_NOTIFICATION    0x04
  #define NODE_FAILURE          0x08 
  #define DELTA_PAYLOAD         0x10                      //This bit is set if only the new node in the system is getting added  
  #define COMPLETE_PAYLOAD      0x20                      //This bit is set if set of all nodes is being sent. 
  uint8_t ttl;                                            //Indicates the number of nodes this message has to be forwarded to
  char    ID[0][ID_SIZE];                             
}addDeleteNodePayload;

typedef struct
{
	uint8_t flags;
	char    ID[ID_SIZE];
}leaderElectionPayload;

typedef struct
{
  uint8_t flags;
  #define ADD_NODE_REQUEST      0x01
  uint32_t timestamp;
  char ipAddr[16]; 
}topologyRequestPayload;

typedef struct
{
  uint8_t flags;
  char fileName[256];
}fileRequestPayload;

typedef struct
{
	uint8_t flags;
    #define FILE_INFO_UPDATE         0x01
    #define FILE_INFO_RESPONSE       0x02
	#define FILE_ALREADY_PRESENT     0x04
	#define FILE_NOT_FOUND           0x08
    #define FILE_NAME_AVAILABLE      0x10
	char    fileName[256];
	uint8_t noOfSplits;
	uint8_t noOfReplicas;
	char    ipAddr[0][16];
}fileInfoPayload;

typedef struct
{
	uint8_t flags;
	#define PUT_FILE_REQUEST  0x01
	#define GET_FILE_REQUEST  0x02
    uint16_t fileSize;
	char fileName[256];

}fileOperationRequestPayload;

typedef struct
{
	char fileName[256];
	char replicaIP[NUM_OF_REPLICAS][16];
}fileDetail;

typedef struct
{
	int numberOfFiles;
	fileDetail fileInfo[0];
}nodeFileInfo;

typedef struct
{
	char ip[16];
}requestNodeFileInfo;


typedef struct
{
	char chunkName[255];
	uint8_t flags;
    #define REPLICATE_INSTRUCTION     0x01
    #define REPLICATE_RESPONSE        0x02
    #define REPLICATE_SUCCESSFUL      0x04
    #define	REPLICATE_FAILURE         0x08
	char ip[16];
	uint8_t numOfReplicas;

}chunkReplicatePayload;

typedef enum {
    RC_FAILURE = 0,
    RC_SUCCESS,
    RC_FILE_NOT_FOUND,
    RC_SOCKET_WRITE_FAILURE,
    RC_SOCKET_CONNECT_FAILED,
    RC_SOCKET_CREATION_FAILED,
    RC_SOCKET_READ_FAILURE,
    RC_INPUT_FILE_NOT_FOUND,
    RC_INVALID_INPUT,
    RC_FILE_INFO_SEND_FAILURE,
    RC_NO_RESPONSE_RECEIVED
}returnCode;


typedef returnCode RC_t;

#endif
