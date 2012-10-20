#ifndef __LOG_H__
#define __LOG_H__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define MACHINE_ID 1

extern FILE *log_fp;
extern char fbuf[500];
extern char fbuf1[200];
extern char fileName[500];

extern time_t timer;
extern char fbuffer[25];
extern struct tm* tm_info;

extern pthread_mutex_t log_mutex;

void log_init();
void log_exit();

typedef enum {
    DEBUG,
    TRACE,
    INFO,
    ERROR
}level;

extern level log_level;
#define LOG(level,str, ...)  do { \
  if (level >= log_level) { \
      pthread_mutex_lock(&log_mutex); \
      time(&timer); \
      tm_info = localtime(&timer); \
      strftime(fbuffer, 35, "[%Y-%m-%d  %H:%M:%S ] ",tm_info); \
      snprintf(fbuf,27,"\n < %s > ", myIP); \
      strncat(fbuf,fbuffer, 70); \
      strcpy(fbuf1, __FILE__); \
      strcat(fbuf, fbuf1); \
      sprintf(fbuf1,"(%d) ", __LINE__); \
      strcat(fbuf, fbuf1); \
      strcat(fbuf," : "); \
      strcpy(fbuf1, str); \
      strcat(fbuf, fbuf1); \
      log_fp = fopen(fileName, "a"); \
      if (log_fp) { \
          fprintf(log_fp, fbuf, __VA_ARGS__); \
          fclose(log_fp); \
      } \
      pthread_mutex_unlock(&log_mutex); \
  } \
}while(0) 

void log_init();
void log_exit();

#endif
