#include "../Headers/functions.hpp"

// Function to handle packet forwarding based on the received packet info
void forward_thread(int udp_sockfd) {
    while (true) {
        PacketInfo packet_info;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait(lock, [] { return !packet_queue.empty(); });

            packet_info = packet_queue.front();
            packet_queue.pop();
        }

        // Sender is client ==> forward Packet to server
        if (packet_info.sender_type == 1 or packet_info.sender_type == 2) {

            int client_uid , server_id;

            if(packet_info.sender_type == 1){
                // Assign UID and server for the new client
                client_uid = assign_uid_to_client(packet_info.addr);
                server_id = assign_server_to_client(client_uid);
            }
            else{//old client
                client_uid = packet_info.client_uid;
                server_id = packet_info.server_id;
            }

            // Append the filename and client UID to the packet data before forward to server
            std::string file_name = client_file_map[client_uid];
            append_To_Tftp_packet(packet_info,file_name,client_uid);

            // Forward the packet to the assigned server
            const ServerInfo &server_info = servers[server_id];
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(server_info.port);
            inet_pton(AF_INET, server_info.ip.c_str(), &server_addr.sin_addr);

            send_packet(udp_sockfd, packet_info.data.data(), packet_info.data.size(), server_addr);
            std::cout << "Forwarded packet from client to server " << server_info.ip << ":" << server_info.port << std::endl;
        } 
        else {// sender is Server forward to designated client

            const ClientInfo &client_info = clients[packet_info.client_uid];
            struct sockaddr_in client_addr;
            client_addr.sin_family = AF_INET;
            client_addr.sin_port = htons(client_info.port);
            inet_pton(AF_INET, client_info.ip.c_str(), &client_addr.sin_addr);

            send_packet(udp_sockfd, packet_info.data.data(), packet_info.data.size(), client_addr);
            std::cout << "Forwarded packet from server to client " << client_info.ip << ":" << client_info.port << std::endl;
        
        }
    }
}
