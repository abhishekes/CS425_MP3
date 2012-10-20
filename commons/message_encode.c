#include<stdio.h>
#include"message_type.h"
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>

#define MAX_BUFFER_SIZE 1000                       //Maximum application payload we support as of now is 1000 bytes. 
#define MAX_PAYLOAD_SIZE 1024

/*Data structure used for marshalling */
int marshalInfo[NUM_OF_PAYLOADS][MAX_ELEMENTS_PER_PAYLOAD][2] = 
{ {
      {marshal_field_info(heartbeatPayload, heartbeatTimeStamp)}
      /*(int)(&(((heartbeatPayload*)0)->heartbeatTimeStamp)), sizeof(((heartbeatPayload*)0)->heartbeatTimeStamp)*/ ,
      {0,0}
  },/*HEARTBEAT PAYLOAD*/
  {
  //  {addDeleteNodePayload, nodeAddDelete}
      {0,0},
  }, /*ADD DELETE NODE PAYLOAD*/
  {
      {marshal_field_info(topologyRequestPayload, timestamp)
      /*(int)(&(((addNodeRequest*)0)->timestamp)), sizeof(((addNodeRequest*)0)->timestamp) */},
      {0,0} 
  }/* ADD NODE REQUEST*/
};


/************************************************************
** This is the message marshalling function. 
** It marshals the data as per the type.
** 
** Arguments:
** msgType  : Message type as defined in message_type.h
** payload : Payload from application layer
** 
** Return Type: void
** Returns Marshalled data in the payload
**************************************************************/

void perform_marshalling(messageType msgType, char *payload) 
{
   int i=0;
   uint32_t val32;
   uint16_t val16;
   while(marshalInfo[msgType][i][1] != 0 ) {
      switch(marshalInfo[msgType][i][1]) {
           case 2 :
                val16 = *((uint16_t *)(payload + marshalInfo[msgType][i][0]));
                *((uint16_t *)(payload + marshalInfo[msgType][i][0])) = htons(val16);
                break;
           case 4 :
                val32 = *((uint32_t *)(payload + marshalInfo[msgType][i][0]));           
                *((uint32_t *)(payload + marshalInfo[msgType][i][0])) = htonl(val32);
                break;
           default :
               printf("\nBreaking\n");
               break;
                    
       }
       i++;
   } 
}
/************************************************************
** This is the message sending function. 
** It populates the relevant fields of the payload and sends.
** it over the socket.
** Arguments:
** socket  : socket on which the message has to be received
** msgType : Type of message
** payload : Payload from application layer
** length  : Length of application payload
**************************************************************/


int sendPayload(int socket, messageType msgType, char* payload, uint16_t length) {

    payloadBuf *buf;                                //Maximum payload we support as of now is 1000 bytes. 
    int bytesSent = 0;
    int rc = 0;
    int type = msgType;
    if (length > MAX_BUFFER_SIZE) {
        DEBUG(("\nWarning :: Payload lentgth > 1024 bytes. Truncating to 1000 bytes\n")); //Anything above 1000 bytes will be truncated.
        length = MAX_BUFFER_SIZE;
    } 
    buf = malloc(sizeof(payloadBuf) + length);
    if (buf == NULL) {
        return RC_FAILURE;
    }
    buf->type =  htons(type);
    buf->length = htons(length + sizeof(payloadBuf));
    //printf("\n Before marshalling \n");
    //printf("\n After marshalling \n");
    perform_marshalling(msgType, payload);
    memcpy(buf->payload, payload, length);
    //DEBUG(("\n Printing below\n"));
    //puts(payload);
    //puts(buf->payload);
    
    int i;
    for(i=0;i<length;i++) {
      DEBUG(("%c,",*((char*)(buf + i))));
    }
    while (bytesSent < sizeof(payloadBuf) + length) {
        rc = write(socket, (buf+bytesSent), (sizeof(payloadBuf) + length) - bytesSent);
        if (rc < 0) {
            DEBUG(("Socket connection closed while trying to write"));
            return RC_SOCKET_WRITE_FAILURE;
        }
        else {
            bytesSent += rc;
        }
    }
    DEBUG(("\nFreeing memory here\n"));
    free(buf);
    return RC_SUCCESS;    
}

int sendPayloadUDP(int socket, messageType msgType, char* payload, uint16_t length, const struct sockaddr *dest_addr) {

    payloadBuf *buf;                                //Maximum payload we support as of now is 1000 bytes. 
    int bytesSent = 0;
    int rc = 0;
    int type = msgType;
    if (length > MAX_BUFFER_SIZE) {
        DEBUG(("\nWarning :: Payload lentgth > 1024 bytes. Truncating to 1000 bytes\n")); //Anything above 1000 bytes will be truncated.
        length = MAX_BUFFER_SIZE;
    } 
    buf = malloc(sizeof(payloadBuf) + length);
    if (buf == NULL) {
        return RC_FAILURE;
    }
    //printf("\nLength in UDP Send : %d ", length);
    buf->type =  htons(type);
    buf->length = htons(length + sizeof(payloadBuf));
    perform_marshalling(msgType, payload);
    memcpy(buf->payload, payload, length);
    //DEBUG(("\n Printing below 1:%0x , 2:%0x : 3 :%0x \n", buf->payload[0], buf->payload[1], buf->payload[2]));
    //puts(payload);
    //puts(buf->payload);
    
    if((bytesSent = sendto(socket, buf, ntohs(buf->length), 0, dest_addr, sizeof(*dest_addr))) < 0)
    {
	//printf("Send to failed");
    }else{
	//printf("bytes sent = %d\n", bytesSent);
    } 

    //DEBUG(("\nFreeing memory here\n"));
    free(buf);
    return RC_SUCCESS;    
}
