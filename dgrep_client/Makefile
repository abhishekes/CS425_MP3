$(CC) = gcc

all :  dgrep_client

debug : CC += -DDEBUG_ENABLE -g
debug : dgrep_client

dgrep_client : file_map.o message_decode.o message_encode.o packet_process.o send_file.o
	$(CC) ../commons/file_map.o ../commons/message_decode.o ../commons/message_encode.o ../commons/send_file.o ../commons/packet_process.o -o dgrep_client dgrep_client.c -pthread

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

clean: 
	rm -rf ../commons/*.o dgrep_client 
