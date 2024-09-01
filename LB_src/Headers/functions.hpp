#pragma once
#include "imports.hpp"
#include "globals.hpp"

// Inside Threads folder
void forward_thread(int udp_sockfd) ;
void receive_thread(int raw_sockfd, const std::string &dest_ip, int dest_port);

//Inside Helper/utility
int assign_server_to_client(int client_uid) ; // Load Balancing Algorithm
int assign_uid_to_client(const struct sockaddr_in &client_addr) ;
int send_packet(int sockfd, const char *buffer, int buffer_size, const struct sockaddr_in &addr) ;
int read_packet(int sockfd, struct sockaddr_in &addr, char *buffer, int buffer_size) ;

//Inside Helper/tftpPacketHandle
bool is_tftp_request(const std::vector<char> &data);
std::string extract_tftp_filename(const std::vector<char> &data);
void append_To_Tftp_packet(PacketInfo &packet_info, std::string &file_name, int client_uid);
void truncate_extract_Tftp_packet(PacketInfo &packet_info , int recv_len);

