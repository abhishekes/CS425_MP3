#include "log.h"
 
FILE *log_fp;
char fileName[500] = {0};
//char buf1[200] = {0};

time_t timer;
char fbuffer[25] = {0};
struct tm* tm_info;
level log_level = DEBUG;
pthread_mutex_t log_mutex;
char fbuf[500];
char fbuf1[200];

void log_init()
{
   sprintf(fileName, "/tmp/machine.%d.log", MACHINE_ID);
   
   pthread_mutex_init(&log_mutex, NULL);
}

void log_exit()
{
    //fclose(log_fp);
}

/*void main() {
    char *myIP = "120.122.333.111";
    log_init();
    LOG(INFO, "Hello %s", "world");
}*/
