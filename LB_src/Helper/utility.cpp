#include "../Headers/functions.hpp"

// Function to receive a packet (recvfrom() wrapper)
int read_packet(int sockfd, struct sockaddr_in &addr, char *buffer, int buffer_size) {
    socklen_t addr_len = sizeof(addr);
    int recv_len = recvfrom(sockfd, buffer, buffer_size, 0, (struct sockaddr *)&addr, &addr_len);
    if (recv_len == -1) {
        perror("recvfrom failed");
    }
    return recv_len;
}

// Function to send a packet (sendto() wrapper)
int send_packet(int sockfd, const char *buffer, int buffer_size, const struct sockaddr_in &addr) {
    int sent_len = sendto(sockfd, buffer, buffer_size, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (sent_len == -1) {
        perror("sendto failed");
    }
    return sent_len;
}

// Function to assign a unique UID to a client and store its IP and port
int assign_uid_to_client(const struct sockaddr_in &client_addr) {
    std::lock_guard<std::mutex> lock(map_mutex); // Protect access to shared data
    int uid = next_client_uid++;
    std::string client_ip = inet_ntoa(client_addr.sin_addr);
    int client_port = ntohs(client_addr.sin_port);
    clients[uid] = {client_ip, client_port};
    std::cout << "Assigned UID " << uid << " to client " << client_ip << ":" << client_port << std::endl;
    return uid;
}

// Function to assign a client to a server (Load Blancing Algo :- Round Robin)
int assign_server_to_client(int client_uid) {
    std::lock_guard<std::mutex> lock(map_mutex); // Protect access to shared data
    int server_id = next_server_id;
    next_server_id = (next_server_id + 1) % servers.size(); // Round-robin load balancing
    client_to_server[client_uid] = server_id;
    server_to_clients[server_id].push_back(client_uid);
    std::cout << "Assigned client UID " << client_uid << " to server ID " << server_id << std::endl;
    return server_id;
}



