#ifndef __MESSAGE_ENCODE__
#define __MESSAGE_ENCODE__
#include "message_type.h"
#include "debug.h"
char * encodePayload(messageType type, char* payload, uint16_t length);

#endif
