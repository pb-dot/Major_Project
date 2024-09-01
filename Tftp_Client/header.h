#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXSIZE 1024
/////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    char* filename;
    char* mode;
} RRQ_WRQ_Packet;

typedef struct {
    int block_number;
    char* data;
    int data_size;
} DATA_Packet;

typedef struct {
    int block_number;
} ACK_Packet;

typedef struct {
    int error_code;
    char* error_message;
} ERROR_Packet;

//////////////////////////////////////////////////////////////////////////////////////////////////

//read_write packet
unsigned char* build_rrq_wrq_packet(const char* filename, const char* mode, int opcode);
RRQ_WRQ_Packet extract_rrq_wrq_packet(const unsigned char* packet);

//data packet
unsigned char* build_data_packet(int block_number, const unsigned char* data, int data_size);
DATA_Packet extract_data_packet(const unsigned char* packet, int packet_size);

//ack packet
unsigned char* build_ack_packet(int block_number);
ACK_Packet extract_ack_packet(const unsigned char* packet);

//err packet
unsigned char* build_error_packet(int error_code, const char* error_message);
ERROR_Packet extract_error_packet(const unsigned char* packet, int packet_size);


///////////////////////////////////////////////////////////////////////////////////////////

//send data from client to server (wrt req)
void handle_writeReq_client(int sockfd,const struct sockaddr_in sa, socklen_t sa_len);
void handle_writeReq_server(int sockfd,const struct sockaddr_in ca, socklen_t ca_len,unsigned char *buffer,RRQ_WRQ_Packet rd_wr);

//send data from server to client (read req)
void handle_readReq_client(int sockfd,const struct sockaddr_in sa, socklen_t sa_len);
void handle_readReq_server(int sockfd,const struct sockaddr_in ca, socklen_t ca_len,unsigned char *buffer,RRQ_WRQ_Packet rd_wr);




