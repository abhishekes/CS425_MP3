#This is make file for admission contact

$(CC) = gcc

all :  admission_contact

debug : CC += -DDEBUG_ENABLE -ggdb
debug : admission_contact

admission_contact : file_map.o message_decode.o message_encode.o packet_process.o send_file.o fdetect_payload_process.o list.o log.o node_send_info.o list.o node_send_info.o
	$(CC) ../commons/file_map.o ../commons/message_decode.o ../commons/message_encode.o ../commons/send_file.o ../commons/packet_process.o ../commons/fdetect_payload_process.o ../commons/list.o -o admission_contact ../logging/log.o ../node/node_send_info.o admission_contact.c -pthread 

file_map.o : ../commons/file_map.c
	$(CC) -c ../commons/file_map.c -o ../commons/file_map.o

packet_process.o : ../commons/packet_process.c
	$(CC) -c ../commons/packet_process.c -o ../commons/packet_process.o

message_decode.o : ../commons/message_decode.c
	$(CC) -c ../commons/message_decode.c -o ../commons/message_decode.o

message_encode.o : ../commons/message_encode.c
	$(CC) -c ../commons/message_encode.c -o ../commons/message_encode.o

send_file.o : ../commons/send_file.c
	$(CC) -c ../commons/send_file.c -o ../commons/send_file.o

fdetect_payload_process.o : ../commons/fdetect_payload_process.c
	$(CC) -c ../commons/fdetect_payload_process.c -o ../commons/fdetect_payload_process.o

list.o : ../commons/list.c
	$(CC) -c ../commons/list.c -o ../commons/list.o

log.o : ../logging/log.c
	 $(CC) -c ../logging/log.c -o ../logging/log.o

node_send_info.o : ../node/node_send_info.c
	$(CC) -c ../node/node_send_info.c -o ../node/node_send_info.o


clean: 
	rm -rf core ../commons/*.o admission_contact 
