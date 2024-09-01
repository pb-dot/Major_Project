#include "../Headers/functions.hpp"

// Function to check if a packet is a TFTP request (Read/Write)
bool is_tftp_request(const std::vector<char> &data) {
    // Simple check for TFTP RRQ/WRQ opcode (1 for RRQ, 2 for WRQ)
    return (data.size() >= 2 && (data[0] == 0 && (data[1] == 1 || data[1] == 2)));
}

// Function to extract the filename from a TFTP RD/WR request
std::string extract_tftp_filename(const std::vector<char> &data) {
    auto end_iter = std::find(data.begin() + 2, data.end(), '\0');
    return std::string(data.begin() + 2, end_iter);
}

// Call this function before forwarding data sent by client to server
void append_To_Tftp_packet(PacketInfo &packet_info, std::string &file_name, int client_uid){
    packet_info.data.push_back(0);
    packet_info.data.insert(packet_info.data.end(), file_name.begin(), file_name.end());
    packet_info.data.push_back(0);
    packet_info.data.push_back(static_cast<char>((client_uid >> 24) & 0xFF));
    packet_info.data.push_back(static_cast<char>((client_uid >> 16) & 0xFF));
    packet_info.data.push_back(static_cast<char>((client_uid >> 8) & 0xFF));
    packet_info.data.push_back(static_cast<char>(client_uid & 0xFF));
}

// Call this function before forwarding data sent by server to client
void truncate_extract_Tftp_packet(PacketInfo &packet_info , int recv_len){
    //The packet sent by server has last 4B as client uid
    packet_info.client_uid = *(int*)(packet_info.data.data() + recv_len - 4); // Extract last 4 bytes for client ID
    // Remove the last 4 bytes (client UID) before forwarding to the client
    packet_info.data.resize(packet_info.data.size() - 4);
}
