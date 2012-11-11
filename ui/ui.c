#include "ui.h"
extern struct Head_Node *server_topology;
extern pthread_mutex_t node_list_mutex;
//extern pthread_mutex_t timestamp_mutex;
state_machine current_state = INIT;   
pthread_mutex_t state_machine_mutex = PTHREAD_MUTEX_INITIALIZER;
extern struct Node *myself;
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
    char command[500];
 
    log_init();
    //pthread_mutex_t node_list_mutex = PTHREAD_MUTEX_INITIALIZER;
    //timestamp_mutex = PTHREAD_MUTEX_INITIALIZER;
    do {
       system("clear");
       printf("\n***************************\nOne-Data-To-Rule-Them-All Inc\nSDFS Interface\n***************************\n\n");
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
               if (server_topology && server_topology->node == myself) {
            	   strcat(buf, "  **** Leader **** ");
               }
               strcpy(display_buf,"\n1. Send Membership Quit notification\n");   
               case1function = node_exit;
               break;
           
       }
       strcat(display_buf, "2. Display membership list\n3. Enter File System Command\n4. Quit");
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
                        /*if (current_state == INIT) {
                             printf("\nOperation invalid in this state\n");
                             getchar();
                        } else */{
                            printf("Enter file system command : ");
                            fflush(stdin);
                            gets(command);
                            puts(command);
                            parse_command(command);
                            getchar();
                        }
                        break;
                   case 4:
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
    }while((choice-'0') != 4);
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
         printf("%s\t %lu", tmp->IP, tmp->timestamp);
         if (tmp == server_topology->node) {
        	printf("%s", "\t(Leader) ");

         }
         printf("\n");

     }
     pthread_mutex_unlock(&node_list_mutex); 
     printf("\nPlease press any key to continue ...\n");
     getchar(); 
}

void parse_command(char *command)
{
     char args[3][255];
     char buffer[255];
     int index = 0;
     int i = 0;
     int string_index = 0;
     int length = 0;
     int valid = 1;
     puts(command);
     FILE *inputFile = NULL;
     while(command[index]) {
         if (command[index] == ' ' || command[index] == 0 )
         {
        	 /*if(length) {
                args[i] = malloc(length + 1);
        	 }*/
        	 //memcpy(args[i], buffer, length);
        	 args[i][length] = 0;
        	 while(command[index++] == ' ');
        	 index--;
        	 string_index = 0;
        	 length = 0;
        	 i++;
         }
    	 args[i][string_index++] = command[index];
    	 index++;
    	 length ++;
    	 if (i > 2 || (i == 0 && length > 3)) {
    		 valid = 0;
    		 break;
    	 }

     }
     if (!valid || (!strcmp(args[1],"del") && i < 1) || (!strcmp(args[1],"del") && i < 2 )) {
    	 printf("\nInvalid Input. Command Format : <get/put/del> <local-file-name> <destination-file-name>\n");

    	 return;

     }
     args[i][length] = 0;
     printf("Local File Name : %s Destination File Name: %s. Length = %d", args[1], args[2], length );
     inputFile = fopen(args[1], "r");
     if (!strcmp(args[0], "put")) {
         if (!inputFile) {
        	 printf("\nCould not find source file");
        	 valid = 0;
         }
         else {
        	 fclose(inputFile);
         }
         if (!valid) {
             return;
         }
         dfs_file_transfer(FILE_PUT, args[1], args[2]);
     }else if (!strcmp(args[0], "get")) {
    	 dfs_file_receive(args[2], args[1]);
     }else if (!strcmp(args[0], "del")) {//Delete operation
         dfs_delete_file(args[1]);
     }else {
    	 printf("\nInvalid Input. Command Format : <get/put/del> <local-file-name> <destination-file-name>\n");
     }


}

