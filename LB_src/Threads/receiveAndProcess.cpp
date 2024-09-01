#include "../Headers/functions.hpp"

// Function to receive packets, identify the sender, and enqueue the packet for forwarding
void receive_thread(int raw_sockfd, const std::string &dest_ip, int dest_port) {
    char buffer[65536];
    struct sockaddr_in addr;

    while (true) {
        int recv_len = read_packet(raw_sockfd, addr, buffer, sizeof(buffer));
        if (recv_len > 0) {
            struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
            struct udphdr *udp_header = (struct udphdr *)(buffer + ip_header->ihl * 4);

            // Convert destination IP and port to match the filter
            struct in_addr dest_addr;
            dest_addr.s_addr = ip_header->daddr;

            //Processing only those packets who match the ip and port filter
            if (inet_ntoa(dest_addr) == dest_ip && ntohs(udp_header->dest) == dest_port) {
                PacketInfo packet_info;
                packet_info.data = std::vector<char>(buffer, buffer + recv_len);
                packet_info.addr = addr;
                packet_info.sender_type = 1;//by default consider new client
                packet_info.client_uid = -1;
                packet_info.server_id = -1;

                std::lock_guard<std::mutex> lock(map_mutex);

                // Check if the packet is from an existing client
                for (const auto &client_pair : clients) {
                    int client_uid = client_pair.first;
                    if (addr.sin_port == htons(clients[client_uid].port) &&
                        std::string(inet_ntoa(addr.sin_addr)) == clients[client_uid].ip) {
                        packet_info.client_uid = client_uid;
                        packet_info.server_id = client_to_server[client_uid];
                        packet_info.sender_type = 2;//2 means existing cleint
                        break;
                    }
                }

                // Check if the packet is from an existing server
                if (packet_info.client_uid == -1) { // If not found in clients
                    for (const auto &server_pair : servers) {
                        int server_id = server_pair.first;
                        if (addr.sin_port == htons(servers[server_id].port) &&
                            std::string(inet_ntoa(addr.sin_addr)) == servers[server_id].ip) {
                            packet_info.server_id = server_id;
                            //extracts packet_info.client_uid and truncates packet_info.data
                            truncate_extract_Tftp_packet(packet_info,recv_len);
                            packet_info.sender_type = 0;// 0 means server
                            break;
                        }
                    }
                }

                // Check if it's a TFTP read/write request
                if (is_tftp_request(packet_info.data)) {
                    std::string file_name = extract_tftp_filename(packet_info.data);
                    if (!file_name.empty()) {
                        client_file_map[packet_info.client_uid] = file_name;
                    }
                }

                // Push the packet info to the queue for processing by forward Thread
                {
                    std::lock_guard<std::mutex> qlock(queue_mutex);
                    packet_queue.push(packet_info);
                }
                queue_cv.notify_one();
            }
        }
    }
}