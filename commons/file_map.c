#include "file_map.h"
extern char myIP[16];
/*********************************************************************
** Helper function for File Transfer.
** Thus file maintains a structure of pointers to all the files
** that are being received through file transfer.
** A new entry is added when FTP_START bit is set and 
** a entry is deleted when FTP_STOP is received.
*********************************************************************/

struct FileNameMap *map = NULL;
pthread_mutex_t file_map_lock = PTHREAD_MUTEX_INITIALIZER;

struct FileNameMap* add_entry(char fileName[FILE_PATH_LENGTH]) {
	struct FileNameMap* newEntry;
	LOG(DEBUG, "Adding entry for %s, %0x", newEntry->fileName, newEntry);
	newEntry = (struct FileNameMap*)malloc(sizeof(struct FileNameMap));
		
	strcpy(newEntry->fileName, fileName);	
	DEBUG(("\nIn add_entry. Filename = %s\n",newEntry->fileName));
	newEntry->fd = open(fileName, O_CREAT | O_RDWR);
	
	DEBUG(("\nadd_entry. FD = %d\n", newEntry->fd));	

	newEntry->state = OPEN;
					
	newEntry->next = NULL;

	pthread_mutex_lock(&file_map_lock);
	if (map == NULL)
		map = newEntry;
	else {
		newEntry->next = map;
		map = newEntry;
	
	}
	pthread_mutex_unlock(&file_map_lock);
	return newEntry;
}

struct FileNameMap* get_entry(char fileName[FILE_PATH_LENGTH]) {
	struct FileNameMap * ptr;
	
	pthread_mutex_lock(&file_map_lock);
	ptr = map;
	
	while(ptr != NULL) {
		if(strcmp(ptr->fileName, fileName))	
			ptr = ptr->next;
		else
			break;
	}
	pthread_mutex_unlock(&file_map_lock);

	return ptr;

}

int delete_entry(int fd) {
	struct FileNameMap *ptr, *prev;

	prev = NULL;

	pthread_mutex_lock(&file_map_lock);
	ptr = map;
	while((ptr!= NULL ) && (ptr->fd != fd)) {
		prev = ptr;
		ptr = ptr->next;
	}
	
	if(ptr == NULL) {
		pthread_mutex_unlock(&file_map_lock);
		return -1;
	}
	else {
		DEBUG(("\nEntry found!!\n"));
		LOG(DEBUG, "Deleting entry for %s %0x", ptr->fileName, ptr);
		if(ptr == map) {
			map = map->next;
			free(ptr);
		} else {
			prev->next = ptr->next;
			free(ptr);
		}
		pthread_mutex_unlock(&file_map_lock);
		return 0;
	}
}
