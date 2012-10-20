#include "file_map.h"

/*********************************************************************
** Helper function for File Transfer.
** Thus file maintains a structure of pointers to all the files
** that are being received through file transfer.
** A new entry is added when FTP_START bit is set and 
** a entry is deleted when FTP_STOP is received.
*********************************************************************/

struct FileNameMap *map = NULL;

struct FileNameMap* add_entry(char fileName[FILE_PATH_LENGTH]) {
	struct FileNameMap* newEntry;

	newEntry = (struct FileNameMap*)malloc(sizeof(struct FileNameMap));
		
	strcpy(newEntry->fileName, fileName);	
	DEBUG(("\nIn add_entry. Filename = %s\n",newEntry->fileName));
	newEntry->fd = open(fileName, O_CREAT | O_RDWR);
	
	DEBUG(("\nadd_entry. FD = %d\n", newEntry->fd));	

	newEntry->state = OPEN;
					
	newEntry->next = NULL;
	if (map == NULL)
		map = newEntry;
	else {
		newEntry->next = map;
		map = newEntry;
	
	}
	return newEntry;
}

struct FileNameMap* get_entry(char fileName[FILE_PATH_LENGTH]) {
	struct FileNameMap * ptr;
	
	ptr = map;
	
	while(ptr != NULL) {
		if(strcmp(ptr->fileName, fileName))	
			ptr = ptr->next;
		else
			break;
	}
	return ptr;

}

int delete_entry(int fd) {
	struct FileNameMap *ptr, *prev;
	ptr = map;
	prev = NULL;
	
	while((ptr!= NULL ) && (ptr->fd != fd)) {
		prev = ptr;
		ptr = ptr->next;
	}
	
	if(ptr == NULL)
		return -1;
	else {
		DEBUG(("\nEntry found!!\n"));
		if(ptr == map) {
			map = map->next;
			free(ptr);
		} else {
			prev->next = ptr->next;
			free(ptr);
		}
		
		return 0;
	}
}
