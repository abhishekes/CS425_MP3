#ifndef __UI_H__
#define __UI_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "../commons/message_type.h"
#include "../commons/list.h"
#include <pthread.h>
#include "../failure_detector/failure_detector.h"
typedef enum {
    INIT,
    JOIN_REQUEST_SENT,
    TOPOLOGY_FORMED,
    TOPOLOGY_FORMED_NEIGHBOURS_UPDATED,
}state_machine;


void display_membership_list();

#endif
