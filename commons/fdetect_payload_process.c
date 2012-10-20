#include "fdetect_payload_process.h"

neighbourHeartbeat savedHeartbeat[NUM_HEARTBEAT_NEIGHBOURS];

extern struct Head_Node *server_topology;
char myIP[16];
struct Node *myself = NULL;
int topology_version = 0;

pthread_mutex_t node_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t heartbeat_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timestamp_mutex = PTHREAD_MUTEX_INITIALIZER;
//nodeData *head = NULL;
//nodeData *thisNode = NULL;
/*********************************************************
** This is the heartbeat process function. 
** It updates the heartbeat timestamp when the heartbeat 
** is received. 
** 
** Arguments:
** ipAddr : IP Address.
***********************************************************/

void processHeartbeatPayload(heartbeatPayload *payload) 
{
    int i; 
    pthread_mutex_lock(&timestamp_mutex);
    for (i = 0; i < NUM_HEARTBEAT_NEIGHBOURS; i++) {
              //printf("Heartbeat Received Outside\n");
              LOG(INFO,"Received heartbeat from %s", payload->ip_addr); 
              if (!(strcmp(savedHeartbeat[i].ipAddr, payload->ip_addr))) {
                  time(&savedHeartbeat[i].latestTimeStamp);
		 // printf("Heartbeat Received from %s\n", payload->ip_addr);
              }
    }
    pthread_mutex_unlock(&timestamp_mutex); 
}


/*********************************************************
** This is the node add process function. 
** It adds the node to the current list of nodes. 
** 
** 
** Arguments:
** ipAddr : IP Address.
***********************************************************/

void processNodeAddDeletePayload(addDeleteNodePayload *payload, int payload_size) 
{
    int i;
    thread_data *tdata;
    char *buf;
    char *dup_payload;
    char IP[16];
    pthread_t thread;
    pthread_mutex_lock(&node_list_mutex);   
    DEBUG(("\n********--------------------In AddDeletePayload-----------------------------************\n"));

    for(i=0;i<sizeof(*payload);i++) {	
    	DEBUG(("%0x,", *(payload + i)));
    }
    
    //getchar();

    if ((payload->flags & ADD_PAYLOAD) && (payload->flags & COMPLETE_PAYLOAD)) {
        delete_all_nodes(&server_topology);
    }
  
	   

    for (i = 0; i < payload->numOfNodes; i++) {
          DEBUG(("Received node update for %s ", payload->ID[0] + 4));

          if (payload->flags & DELETE_PAYLOAD) {
              printf("Node %s is being deleted from the group membership", payload->ID[0] + 4);
              remove_from_list(&server_topology, payload->ID[i]);
  
          if (payload->flags & LEAVE_NOTIFICATION) {
              LOG(INFO, "Node %s is voluntarily leaving the group", payload->ID[i]);
          }
          else if (payload->flags & LEAVE_NOTIFICATION) {
              LOG(INFO, "Node %s has failed. Removing its entry", payload->ID[i]);
 
               
          }
       }else if (payload->flags & ADD_PAYLOAD) {
           LOG(INFO, "Node %s is being added as a member in the group", payload->ID[i]);
           printf("Node %s is being added as a member in the group", payload->ID[0] + 4);
           add_to_list(&server_topology, payload->ID[i]);             
           pthread_mutex_lock(&timestamp_mutex);
           strcpy(savedHeartbeat[0].ipAddr, payload->ID[i] + 4); 
           pthread_mutex_unlock(&timestamp_mutex);

       }
    }
    
    if(myself != NULL) {
        memcpy(IP, myself->next->IP, 16); 
        pthread_mutex_lock(&timestamp_mutex);
        strcpy(savedHeartbeat[0].ipAddr, myself->prev->IP); 
        pthread_mutex_unlock(&timestamp_mutex);
    }
           
    pthread_mutex_unlock(&node_list_mutex); 
    if (payload->ttl > 0 && server_topology->num_of_nodes > 0) {
        payload->ttl--;
        buf =  calloc(1,payload_size);
        memcpy(buf, payload, payload_size);
        tdata = calloc(1,sizeof(thread_data));
        memcpy(tdata->ip, IP, 16);
        tdata->payload_size = payload_size;
        tdata->payload = buf;
        (void)pthread_create(&thread, NULL, send_node_update_payload, (void*)tdata);
         
    }
}
/*********************************************************
** This is the topology request function. 
** Can be called by both the master and the node willing
** to join the ring
** If the ADD_NODE_REQUEST bit is set, it means that this 
** node wants to join the topology. 
** Arguments:
** 
***********************************************************/

void processTopologyRequest(int socket, topologyRequestPayload *payload) 
{
    int i;
    int offset = 0;
    struct Node* tmp=NULL;
    struct Node* found = NULL;
    char *buf=NULL;
    char *ipAddrList = NULL;
    char ID[ID_SIZE];
    int offset1 = 0;
    int numNodestoSend = 0;
    uint32_t timestamp =0;
    int index = 0;
    int nodeDelete = 0;
    int total_nodes = 0;
    int nodes_to_send_to = 0;
    
    pthread_mutex_lock(&node_list_mutex);
    if(server_topology ) {
        tmp = server_topology->node;
    }      
    LOG(INFO, "Received topology request from  %s ", payload->ipAddr);

    for (index = 0; server_topology && index < server_topology->num_of_nodes; index++) {
        DEBUG(("\n\nLooping through : %s , searching for :%s", tmp->IP, payload->ipAddr));
        if (!memcmp(payload->ipAddr, tmp->IP, 15)) {
            found = tmp;
            //printf("\n3: Found ***********, IP : %s",payload->ipAddr);
     
            
        }
    }
    //printf("\n4\n");
    if (payload->flags & ADD_NODE_REQUEST) {
        timestamp = htonl(payload->timestamp);
        //printf("\n5\n");

        memcpy(ID, &(timestamp), sizeof(timestamp)); 
        memcpy(ID + 4, payload->ipAddr, 16);
        //printf("\n6\n");
        add_to_list(&server_topology, ID); 
   }
   populate_ipAddrList(&buf, &total_nodes, &ipAddrList, &nodes_to_send_to, 0);
   pthread_mutex_unlock(&node_list_mutex);
   if (total_nodes) {
        sendTopologyResponse(socket, total_nodes , buf);  
   }
   if ((payload->flags & ADD_NODE_REQUEST) && (nodes_to_send_to)) {
        //printf("\n7 Nodes : %d, IP List : %s \n", server_topology->num_of_nodes, ipAddrList);
        sendAddNodePayload(ipAddrList, nodes_to_send_to , ID);
   }
   if (ipAddrList) {
        free(ipAddrList);
   }
   if (buf) {
        free(buf);
   }
   //printf("\n8\n");
    
}


/*********************************************************
** This is the IP address get function 
** It gets the system's IP address
** 
** 
** Arguments:
** ipAddr : IP Address.
***********************************************************/
RC_t getIpAddr()
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    char addressBuffer[INET_ADDRSTRLEN];
    struct in_addr *tmpAddrPtr;
    RC_t rc = RC_FAILURE;
    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strncmp(addressBuffer, "127.0.0.1", INET_ADDRSTRLEN)) {
                LOG(INFO, "Node IP Address Obtained : %s", addressBuffer); 
                //printf("\nIP : %s\n", addressBuffer);
                strcpy(myIP, addressBuffer);        
                rc = RC_SUCCESS;
                break;
            }
        }     
   }
   if (ifAddrStruct!=NULL) 
       freeifaddrs(ifAddrStruct);
   
   
   return rc;
    
}

/*********************************************************
** This is function used to send request for topology 
** request
** 
** 
** Arguments:
** socket     : socket on which the payload was received.
** numOfNodes : Num of nodes in topology.
** buf        : buffer having IP addresses of nodes.
***********************************************************/
void sendTopologyResponse(int socket, int numOfNodes, char *buf)
{
     int size = (sizeof(addDeleteNodePayload) + (numOfNodes * ID_SIZE));
 
     addDeleteNodePayload *payloadBuf = malloc(size);
     payloadBuf->numOfNodes = numOfNodes;
     //printf("\n Sending here 1\n"); 

     payloadBuf->flags = ADD_PAYLOAD | COMPLETE_PAYLOAD;
     payloadBuf->ttl = 0;          //No need to propogate
     memcpy(payloadBuf->ID, buf, numOfNodes * ID_SIZE);
     //printf("\n Sending here 2\n"); 
     //getchar();
 
    //printf("\n Sending here 3\n"); 
    sendPayload(socket, MSG_ADD_DELETE_NODE, payloadBuf, size);
    //printf("\n Sending response \n"); 
    free(payloadBuf); 
}

/*********************************************************
** This is the function used to send request for topology 
** 
** 
** 
** Arguments:
** socket     : socket on which the payload was received.
** numOfNodes : Num of nodes in topology.
** buf        : buffer having IP addresses of nodes.
***********************************************************/
void sendTopologyJoinRequest(int socket)
{
    int size = (sizeof(topologyRequestPayload));
    time_t t;
    uint32_t timestamp = time(&t);
    //printf("\n\nSize = %d",size);
    LOG(INFO, "Sending Join Request to Admission %s:", "Contact");
    //printf("\nSending Join Request \n");
    topologyRequestPayload *payloadBuf = 
                        calloc(1,sizeof(topologyRequestPayload));
   
    payloadBuf->flags = ADD_NODE_REQUEST;
    payloadBuf->timestamp = timestamp;
    memcpy(payloadBuf->ipAddr, myIP, 16);
    //printf("\nSending Join Request %s\n", myIP);
    sendPayload(socket, MSG_TOPOLOGY_REQUEST, payloadBuf, size);
    free(payloadBuf); 
}

/********************************************************************
** This is function used to send Add Node Payload 
** 
** 
** 
** Arguments:
** socket     : socket on which the payload was received.
** numOfNodes : Num of nodes in topology.
** buf        : buffer having IP addresses of nodes.
**********************************************************************/
void sendAddNodePayload(char *ipAddrList, int numOfNodesToSend, char ID[ID_SIZE] )
{
    char (*IP)[16];
    IP = ipAddrList;
    int index = 0;
    int i;
    thread_data *my_data[5];
    pthread_t   thread[5];
    int threads_created = 0;
    addDeleteNodePayload *payloadBuf = calloc(1,sizeof(addDeleteNodePayload) + ID_SIZE);  
    //printf("\nIn here \n");
    payloadBuf->numOfNodes = 1;
    payloadBuf->flags |= (ADD_PAYLOAD | DELTA_PAYLOAD);
    payloadBuf->ttl = 0;
    memcpy(payloadBuf->ID, ID, ID_SIZE);
    //payloadBuf->ID[0][47] = 0;
    while(index < numOfNodesToSend ) {
        threads_created = 0;
        for (i=0; i<5 && index < numOfNodesToSend; i++, index ++, threads_created++) {
	    
 	    //printf("Num nodes to send = %d", numOfNodesToSend);
            //my_data[i].ip[15] = 0;
            my_data[i] = calloc(1, sizeof(thread_data) + sizeof(addDeleteNodePayload) + ID_SIZE);
            (*my_data[i]).payload = calloc(1, sizeof(addDeleteNodePayload) + ID_SIZE);
            memcpy((*my_data[i]).ip, IP, 16);
            IP++;
            (*my_data[i]).payload_size = sizeof(addDeleteNodePayload) + ID_SIZE;  
            (*my_data[i]).msg_type = MSG_ADD_DELETE_NODE;
            memcpy((*my_data[i]).payload, payloadBuf, sizeof(addDeleteNodePayload) + ID_SIZE);
            pthread_create(&thread[i], NULL, send_node_update_payload, (my_data[i])); 
        }
        for (i=0 ; i < threads_created; i++) {
            pthread_join(thread[i],NULL);
        } 
    }
    free(payloadBuf);
}


/********************************************************************
** This is function used to send Add Node Payload 
** 
** 
** 
** Arguments:
** socket     : socket on which the payload was received.
** numOfNodes : Num of nodes in topology.
** buf        : buffer having IP addresses of nodes.
**********************************************************************/
void sendDeleteNodePayload(char *ipAddrList, int numOfNodesToSend, char ID[ID_SIZE] , int ttl, char flags)
{
    char (*IP)[16];
    IP = ipAddrList;
    int index = 0;
    int i;
    thread_data *my_data[5];
    pthread_t   thread[5];
    int threads_created = 0;
    addDeleteNodePayload *payloadBuf = calloc(1,sizeof(addDeleteNodePayload) + ID_SIZE);  
    //printf("\nIn here \n");
    payloadBuf->numOfNodes = 1;
    payloadBuf->flags |= (DELETE_PAYLOAD | DELTA_PAYLOAD | flags);
    payloadBuf->ttl = ttl;
    memcpy(payloadBuf->ID, ID, ID_SIZE);
    //payloadBuf->ID[0][47] = 0;
    while(index < numOfNodesToSend ) {
        threads_created = 0;
        for (i=0; i<5 && index < numOfNodesToSend; i++, index ++, threads_created++) {
	    
 	    //printf("Num nodes to send = %d", numOfNodesToSend);
            //my_data[i].ip[15] = 0;
            my_data[i] = calloc(1, sizeof(thread_data) + sizeof(addDeleteNodePayload) + ID_SIZE);
            (*my_data[i]).payload = calloc(1, sizeof(addDeleteNodePayload) + ID_SIZE);
            memcpy((*my_data[i]).ip, IP, 16);
            IP++;
            (*my_data[i]).payload_size = sizeof(addDeleteNodePayload) + ID_SIZE;  
            (*my_data[i]).msg_type = MSG_ADD_DELETE_NODE;
            memcpy((*my_data[i]).payload, payloadBuf, sizeof(addDeleteNodePayload) + ID_SIZE);
            pthread_create(&thread[i], NULL, send_node_update_payload, (my_data[i])); 
        }
        for (i=0 ; i < threads_created; i++) {
            pthread_join(thread[i],NULL);
        } 
    }
    free(payloadBuf);


}


void populate_ipAddrList(char **idList, int *noOfNodes, char **nodesToSendTo, int *numOfNodesToSendTo, int flag) 
{
     int i;
     char *list_ptr = NULL;
     char *nodes_to_send_ptr = NULL;
     struct Node* tmp = NULL;
     uint32_t timestamp = 0;
     int populate_nodes = 0;

     *idList = NULL;
     *nodesToSendTo = NULL;
     *noOfNodes = 0;
     *numOfNodesToSendTo = 0;
  
     if (server_topology == NULL || server_topology->node == NULL) {
         return ;
     }
     tmp = server_topology->node;  

     *idList = malloc( ID_SIZE *  server_topology->num_of_nodes);
     list_ptr = *idList;
     *noOfNodes = server_topology->num_of_nodes;    
     if (server_topology->num_of_nodes > 1) {

	     *nodesToSendTo = calloc(1, (16  * (server_topology->num_of_nodes -1 )));
             *numOfNodesToSendTo = server_topology->num_of_nodes -1;        
              nodes_to_send_ptr = *nodesToSendTo;
              
	     if(server_topology->num_of_nodes > 2)  { 
       		   memcpy(nodes_to_send_ptr, server_topology->node->prev->prev->IP, 16);
  	           nodes_to_send_ptr += 16; 
             //Populate first entry 
             }
             populate_nodes = 1;  
     } 
     
     for (i = 0; i < server_topology->num_of_nodes; i++, tmp = tmp->next) {
          
         timestamp = htonl(tmp->timestamp);
         memcpy(list_ptr, &timestamp, 4);
         list_ptr += 4;
         memcpy(list_ptr, tmp->IP, 16);
         list_ptr += 16;
         
         if ( populate_nodes && ((server_topology->num_of_nodes == 2) || (tmp != server_topology->node->prev->prev)) && (tmp != server_topology->node->prev) )  {
             memcpy(nodes_to_send_ptr, tmp->IP, 16);
             nodes_to_send_ptr += 16;
         
         }       
          
     }
     //printf("\nPrinting ID List : %s, IP List : %s &&&&&&&&&&&&&&&&&&&& \n",(*idList)+4, *nodesToSendTo); 
}
