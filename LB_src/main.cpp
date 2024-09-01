#include "Headers/imports.hpp"
#include "Headers/globals.hpp"
#include "Headers/functions.hpp"

// Global maps for client UIDs and server assignments
std::unordered_map<int, ClientInfo> clients;           // UID -> ClientInfo
std::unordered_map<int, ServerInfo> servers;           // Server ID -> ServerInfo
std::unordered_map<int, int> client_to_server;         // Client UID -> Server ID
std::unordered_map<int, std::vector<int>> server_to_clients; // Server ID -> vector of Client IDs
std::unordered_map<int, std::string> client_file_map;  // Client UID -> FileName

std::mutex map_mutex; // Mutex to protect shared data structures

int next_client_uid = 1;   // UID counter for clients
int next_server_id = 0;    // Server ID counter

std::queue<PacketInfo> packet_queue; // Queue for packets to be processed by Thread 2
std::mutex queue_mutex;              // Mutex to protect packet queue
std::condition_variable queue_cv;    // Condition variable to notify Thread 2


int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <LB IP> <LB Port>" << std::endl;
        return -1;
    }

    std::string dest_ip = argv[1];
    int dest_port = std::stoi(argv[2]);

    // Initialize UDP socket for sending packets
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd == -1) {
        perror("UDP socket failed");
        return -1;
    }

    // Initialize raw socket for reading packets
    int raw_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_sockfd == -1) {
        perror("Raw socket failed");
        close(udp_sockfd);
        return -1;
    }

    // Set promiscuous mode on the raw socket
    struct ifreq ifr;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1); // Replace "eth0" with your interface
    if (ioctl(raw_sockfd, SIOCGIFFLAGS, &ifr) == -1) {
        perror("ioctl SIOCGIFFLAGS failed");
        close(raw_sockfd);
        close(udp_sockfd);
        return -1;
    }
    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(raw_sockfd, SIOCSIFFLAGS, &ifr) == -1) {
        perror("ioctl SIOCSIFFLAGS failed");
        close(raw_sockfd);
        close(udp_sockfd);
        return -1;
    }

    // Start threads
    std::thread receive_thread_instance(receive_thread, raw_sockfd, dest_ip, dest_port);
    std::thread forward_thread_instance(forward_thread, udp_sockfd);

    // Join threads
    receive_thread_instance.join();
    forward_thread_instance.join();

    close(raw_sockfd);
    close(udp_sockfd);
    return 0;
}
