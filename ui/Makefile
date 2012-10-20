
$(CC) = gcc

all :  ui

debug : CC += -DDEBUG_ENABLE -g
debug : ui

ui : file_map.o fdetect_payload_process.o message_decode.o message_encode.o packet_process.o failure_detector.o list.o log.o node_send_info.o heartbeat_send.o heartbeat_receive.o get_topology.o send_file.o topology_update.o
	$(CC) ui.c -o ui ../commons/fdetect_payload_process.o ../commons/message_decode.o ../commons/message_encode.o ../commons/packet_process.o -pthread  ../commons/list.o ../logging/log.o ../node/node_send_info.o ../failure_detector/heartbeat_send.o ../failure_detector/heartbeat_receive.o ../failure_detector/get_topology.o ../commons/send_file.o ../failure_detector/failure_detector.o ../failure_detector/topology_update.o ../commons/file_map.o

fdetect_payload_process.o : ../commons/fdetect_payload_process.c
	$(CC) -c ../commons/fdetect_payload_process.c -o ../commons/fdetect_payload_process.o

packet_process.o : ../commons/packet_process.c 
	$(CC) -c ../commons/packet_process.c -o ../commons/packet_process.o

message_decode.o : ../commons/message_decode.c
	$(CC) -c ../commons/message_decode.c -o ../commons/message_decode.o

message_encode.o : ../commons/message_encode.c
	$(CC) -c ../commons/message_encode.c -o ../commons/message_encode.o

list.o : ../commons/list.c
	$(CC) -c ../commons/list.c -o ../commons/list.o

log.o : ../logging/log.c
	$(CC) -c ../logging/log.c -o ../logging/log.o

node_send_info.o : ../node/node_send_info.c
	$(CC) -c ../node/node_send_info.c -o ../node/node_send_info.o

failure_detector.o : ../failure_detector/failure_detector.c 
	$(CC) -c ../failure_detector/failure_detector.c -o ../failure_detector/failure_detector.o 

heartbeat_send.o :
	$(CC) -c ../failure_detector/heartbeat_send.c -o ../failure_detector/heartbeat_send.o

heartbeat_receive.o :
	$(CC) -c ../failure_detector/heartbeat_receive.c -o ../failure_detector/heartbeat_receive.o

get_topology.o :
	$(CC) -c ../failure_detector/get_topology.c -o ../failure_detector/get_topology.o

send_file.o : ../commons/send_file.c
	$(CC) -c ../commons/send_file.c -o ../commons/send_file.o

file_map.o : ../commons/file_map.c
	$(CC) -c ../commons/file_map.c -o ../commons/file_map.o

topology_update.o : ../failure_detector/topology_update.c
	$(CC) -c ../failure_detector/topology_update.c -o ../failure_detector/topology_update.o
clean: 
	rm -rf core ../commons/*.o ui ../failure_detector/*.o ../logging/*.o ../node/*.o


