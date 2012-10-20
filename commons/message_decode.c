#include "message_decode.h"
/*********************************************************
** This is the message decode function. 
** It converts a stream of bytes into payloads that 
** can be handled by the application Layer.
** Arguments:
** socket : socket on which the message has to be received
** packet : Packet returned to the calling function for
**          processing. 
***********************************************************/

int message_decode(int socket, payloadBuf **packet) {
    uint16_t length = 0;
    char *ptr = (char *)(&length);
    int size = sizeof(length);
    int bytesRead = 0;
    int rc;
    //printf("Before while loop\n");
    while (size > 0) {
        rc = read(socket, ptr, size);
        if (rc <= 0) {
        DEBUG(("Read failed in message decode. Socket may have been closed."));
        return rc;
        }
        bytesRead = rc;
        size -= bytesRead;
        ptr += bytesRead;    
    }
    //printf("\nAfter while loop\n");
    length = ntohs(length);                               // Get the length of the packet
    //printf("Length = %d", length);
    //getchar();
    size = length - sizeof(length);
    *packet = (payloadBuf *)calloc(1, length);
    (*packet)->length = length;
    ptr = (char*)((char*)(*packet) + sizeof(length));
    while (size > 0) {
        rc = read(socket, ptr, size);
        if (rc < 0) {
          DEBUG(("Read failed in message decode. Socket may have been closed."));
          return rc;
        }
        bytesRead = rc;
        size -= bytesRead;
        ptr += bytesRead;
    }
    (*packet)->type = ntohs((*packet)->type);
    DEBUG(("\nPacket type = %d\n", (*packet)->type));
    int i;
    for (i=0; i<length; i++ ) {
    //DEBUG(("%d,",(*((char*)(*packet) +i))));
    DEBUG(("%0x,",*(packet + i)));
    }
    return RC_SUCCESS;
}
 
int message_decode_UDP(int socket, payloadBuf *packet, int *packetLen, struct sockaddr_in *fromAddr, ssize_t *fromAddrLen) {

    
    *packetLen = recvfrom(socket, packet, 1000, 0, (struct sockaddr * )fromAddr, fromAddrLen); 
    packet->length = ntohs(packet->length);
    packet->type = ntohs(packet->type);
    //printf("\n In message decode  length = %d \n", packet->length);

    return RC_SUCCESS;
}
