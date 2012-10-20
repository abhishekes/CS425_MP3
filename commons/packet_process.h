#ifndef __PACKET_PROCESS__
#define __PACKET_PROCESS__
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include"message_type.h"
#include"file_map.h"
#include"debug.h"

void processPacket(int, payloadBuf *);
#endif
