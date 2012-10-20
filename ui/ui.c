#include "ui.h"
extern struct Head_Node *server_topology;
extern pthread_mutex_t node_list_mutex;
extern pthread_mutex_t timestamp_mutex;
state_machine current_state = INIT;   
pthread_mutex_t state_machine_mutex = PTHREAD_MUTEX_INITIALIZER;

void main()/*interact_with_user()*/
{
    char choice = 0;
    int valid = 0;
    char buf[100];
    char display_buf[600];
    char dislay_buf2[300];
    char user_input[10]; 
    int (*case1function)(void);    
    int test = 0;
 
    log_init();
    //pthread_mutex_t node_list_mutex = PTHREAD_MUTEX_INITIALIZER;
    //timestamp_mutex = PTHREAD_MUTEX_INITIALIZER;
    do {
       system("clear");
       printf("\n***************************\nGroups-RUs Inc\nNode Membership Interface\n***************************\n\n");
       buf[0] = 0;
       display_buf[0] = 0;
       //printf("Current Membership Status : ");
       switch(current_state) {
           case INIT:
               strcpy(buf,"Not a member");
               strcpy(display_buf,"\n1. Send Membership Join Request\n");
               case1function = node_init;
               break;
           /*case JOIN_REQUEST_SENT:
               strcpy(buf, "Join request sent to admission contact");
               break;
           case TOPOLOGY_FORMED:
               strcpy(buf, "Membership List Received. Updating other nodes");
               break;
           */
           case TOPOLOGY_FORMED:
               strcpy(buf, "Membership Active");
               strcpy(display_buf,"\n1. Send Membership Quit notification\n");   
               case1function = node_exit;
               break;
           
       }
       strcat(display_buf, "2. Display membership list\n3. Quit");
       printf("Current membership Status : %s\n\n%s\n\n", buf, display_buf);
       valid = 0;
       do {
           printf("\nEnter choice : "); 
           user_input[0] = 0;
	   gets(user_input); test = 1;
           
	   choice = user_input[0];
           //printf("\nUser Input : %0x\n", choice);
           valid = 1;
           switch(choice - '0') {
                   case 1: 
                       system("clear");
                       printf("\n\n Processing request. Please wait\n\n");
                       (*case1function)();
                       getchar();
                       break;
                   case 2:
                        if (current_state == INIT) {
                             printf("\nOperation invalid in this state\n");
                             getchar();
                        } else {
                            display_membership_list();
                        }
                        //display_membership_list();
                        break;
                   case 3:
                        if (current_state == TOPOLOGY_FORMED) {
                            node_exit();
                        }
                        exit(0);
                        break;
                   default:
                        printf("\nInvalid entry. Please try again\n");
                        valid = 0;
                        break;

          }
            
       }while(valid == 0);
       //getchar();
       system("clear"); 
    }while((choice-'0') != 3);
    //}while(0); 
    //while(1);
}


void display_membership_list()
{
     int i = 0;
     system("clear");
     struct Node *tmp;
     printf("\nNode IP Address\t Node Timestamp\n*****************************************************************\n");
     pthread_mutex_lock(&node_list_mutex);    
     for (tmp = server_topology->node; i < server_topology->num_of_nodes ; i++, tmp=tmp->next) {
         printf("%s\t %lu\n", tmp->IP, tmp->timestamp);
     }
     pthread_mutex_unlock(&node_list_mutex); 
     printf("\nPlease press any key to continue ...\n");
     getchar(); 
}


