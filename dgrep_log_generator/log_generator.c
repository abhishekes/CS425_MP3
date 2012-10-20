#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>

/*This is the pattern that is going to be inserted in log files for machines */
char *pattern[] = {
    "FREQUENT",
    "FREQUENT_MACHINE_",
    "FEW_MACHINE",
    "SCARCE",
    "SCARCE_MACHINE_"    
};

#define FREQUENT_DIVISOR  2
#define SCARCE_DIVISOR   17


/*****************************************************************************************
** This program generates the random machie logs by accepting the machine number and      
** size of the file in MB. It expects files by th name syslog.<machine id>  to be present 
** in the location where this file is executed from. These files are used o generate the  
** random lines in the log files.                                                         
**                                                                                        
** Patterns by the name FREQUENT_MACHINE_<i> and SCARCE_MACHINE_<i> will be generated 
** in logs of machine i.
** Pattern by name FEW_MACHINE will be generated in machines with even id only                                                                         *****************************************************************************************/



int  main(int argc, char *argv[])
{
    FILE *fp;
    FILE *log;
    int length = 0;
    int file_length =0;
    char line[1000];    
    char fileName[1000];
    char patternLine[1000];
    int num;
    int counter;
    int selector;
    int flip = 0;
    int rotate = 0;
    int rotate1 = 0;
    int running_counter =0;
    char temp[2];

    if (argc != 3 ) {
        printf("\nUsage: log_generator <machine-id> <Log Size in MB>\n");
        return -1;
    }
    length = atoi(argv[2]);
    
    if ( length < 0 || length > 1024) {
        printf("\nInvalid file size specified. Maximum allowed size is 1024 MB");
        return -1;  
    
    }
    length *= 1024 * 1024; //Convert to bytes 
    sprintf(fileName, "syslog.%s", argv[1]); // File for geting random data
    fp = fopen (fileName, "r");    

    sprintf(fileName, "machine.%s.log", argv[1]); //Output log file 
    log = fopen(fileName, "w");

    if ((fp == NULL) || (log == NULL)) {
       printf("\nError in file processing\n");
       return -1;
    }
     
    while(file_length < length) {
       srand(num);
       num =  rand();
       counter = (num % 300);
       num = running_counter++;
       while (counter > 0) { 
           if (feof(fp)) {
               fseek(fp, 0L, SEEK_SET);
           }
           fgets(line, 1000, fp);
           fputs(line, log);
           file_length += strlen(line);
           counter--; 
       }
       if ((num % FREQUENT_DIVISOR) == 0)  {
          strcpy(temp, "");
          if (((rotate%3 - (atoi(argv[1]))%2) % 3) == 1) {
              sprintf(temp, "%s", argv[1] );
          }
          sprintf(patternLine, "##TEST_LOGGER: The pattern is %s%s\n", pattern[(rotate % 3) - (atoi(argv[1]))%2], temp); 
          rotate++;
       }
       if ((num % SCARCE_DIVISOR) == 0) {
           strcpy(temp, "");
           if (rotate1) {
              sprintf(temp, "%s", argv[1]);
           } 
           sprintf(patternLine, "##TEST_LOGGER: The pattern is %s%s\n", pattern[rotate1+3], temp);
           rotate1 = !rotate1;
           
       }
       fputs(patternLine, log);
       file_length += strlen(patternLine);    
    }
    fclose(log);
    fclose(fp);    
}
