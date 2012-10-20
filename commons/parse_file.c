#include "parse_file.h"

int parse_IPs(FILE *fp, struct Head_Node **topology, int *version) {
	char line[100];
	char ip[16];
	int file_version;
	
	while(fgets(line, 100, fp) != NULL) {
		if(strstr(line, "VERSION=") != NULL) { //It is the version number line. Format VERSION=<number>
			sscanf(line, "VERSION=%d", &file_version);
			
			if(file_version <= *version) //we are up-to-date. don't parse any more
				break;
			else
				*version = file_version;
			
		} else if (strstr(line, "IP=") != NULL) { //It is the IP line. Format IP=<ip>
			sscanf(line, "IP=%s", ip);
			ip[15] = 0;
			if(add_to_list(topology, ip) != 0) {
				DEBUG(("parseIPs: Something went wrong while adding IPs"));
			} 
		}	
	}

	//Let the caller take care of closing the file	
	return 0;		
}
