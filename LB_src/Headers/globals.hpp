#pragma once
#include "imports.hpp"

// Structure to hold client information
struct ClientInfo {
    std::string ip;
    int port;
};

// Structure to hold server information
struct ServerInfo {
    std::string ip;
    int port;
};

// Structure to hold a packet and metadata for processing
struct PacketInfo {
    std::vector<char> data;//packet recv by LB
    sockaddr_in addr;//sender address
    short int sender_type;// 0 server | 1 new client | 2 old client
    int client_uid;
    int server_id;
};

/////////////////////////////////The below is defined in main.cpp
// Global maps for client UIDs and server assignments
extern std::unordered_map<int, ClientInfo> clients;           // Client UID -> ClientInfo
extern std::unordered_map<int, ServerInfo> servers;           // Server ID -> ServerInfo
extern std::unordered_map<int, int> client_to_server;         // Client UID -> Server ID
extern std::unordered_map<int, std::vector<int>> server_to_clients;         // Server ID -> Client UID
extern std::queue<PacketInfo> packet_queue; // Queue for packets to be processed by Thread 2
extern std::unordered_map<int, std::string> client_file_map;  // Client UID -> FileName

extern int next_client_uid;   // UID counter for clients
extern int next_server_id;    // Server ID counter

extern std::mutex map_mutex; // Mutex to protect shared data structures
extern std::mutex queue_mutex;              // Mutex to protect packet queue
extern std::condition_variable queue_cv;    // Condition variable to notify Thread 2
