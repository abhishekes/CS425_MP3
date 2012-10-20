#include "ui.h"
state_machine current_state = INIT;   
pthread_mutex_t state_machine_mutex = PTHREAD_MUTEX_INTITIALIZER;
void main()/*interact_with_user()*/
{
    char choice = 0;
    int valid = 0;
    char buf[100];
    char display_buf[600];
    char dislay_buf2[300];
    char user_input[10]; 
    void * (*case1function)(void * data);    
    pthread_t node_thread; 
    do {
       system("clear");
       printf("\n***************************\nGroups-RUs Inc\nNode Membership Interface\n***************************\n\n");
       buf[0] = 0;
       display_buf[0] = 0;
       //printf("Current Membership Status : ");
       pthread_mutex_lock(&state_machine_mutex);
       switch(current_state) {
           case INIT:
               strcpy(buf,"Not a member");
               strcpy(display_buf,"\n1. Send Membership Join Request\n");
               case1function = node_init;
               break;
           case JOIN_REQUEST_SENT:
               strcpy(buf, "Join request sent to admission contact");
               break;
           case TOPOLOGY_FORMED:
               strcpy(buf, "Membership Active");
               break;
/*           case TOPOLOGY_FORMED_NEIGHBOURS_UPDATED:
               strcpy(buf, "Membership Active");
               strcpy(display_buf,"\n1. Send Membership Quit notification\n");   
               case1function = node_exit;
               break;
*/
           case SEND_QUIT_NOTIFICATION:
               strcpy(buf, "Membership active, Sending quit notification to user nodes ");
               strcpy(display_buf,"\n1. Send Membership Quit notification \n2.Display membership list ");
               
       }
       pthread_mutex_unlock(&state_machine_mutex);
       strcat(display_buf, "\n\n0. Quit");
       printf("Current membership Status : %s\n\n%s\n\n", buf, display_buf);
       valid = 0;
       do {
           printf("\nEnter choice : "); 
           user_input[0] = 0;
           gets(user_input);
           choice = user_input[0];
           //printf("\nUser Input : %0x\n", choice);
           valid = 1;
           pthread_mutex_lock(&state_machine_mutex);
           switch(choice - '0') {
                   case 1: 
                       if (current_state == INIT) {
                           pthread_create(&node_thread,  
                       } else {
                           current_state = SEND_QUIT_NOTIFICATION;
                       }   
                       break;
                   case 2:
                        //display_membership_list();
                        break;
                   case 3:
                        exit(0);
                        break;
                   default:
                        printf("\nInvalid entry. Please try again\n");
                        valid = 0;
                        break;
           pthread_mutex_unlock(&state_machine_mutex);    

          }
            
       }while(valid == 0);
       system("clear"); 
    }while((choice-'0') != 3);
}
