#ifndef __FILE_MAP__
#define __FILE_MAP__
#include"message_type.h"
#include"debug.h"
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>


struct FileNameMap{
	char fileName[FILE_PATH_LENGTH];
	int fd;
	int state;
	#define OPEN 0x01
	#define CLOSED 0x2
	struct FileNameMap *next;
};

struct FileNameMap* add_entry(char fileName[FILE_PATH_LENGTH]);
struct FileNameMap* get_entry(char filename[FILE_PATH_LENGTH]);
int delete_entry(int fd);
	
#endif // __FILE_MAP__
