#pragma once

#include <bits/stdc++.h>
///////////////// Ntwk Related headers ////////////////
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>   // For iphdr
#include <netinet/udp.h>  // For udphdr
#include <sys/socket.h>
#include <sys/ioctl.h>         // For ioctl()
#include <linux/if_packet.h>   // For AF_PACKET, SOCK_RAW, etc.
#include <linux/if_ether.h>    // For ETH_P_ALL, ETH_HLEN, etc.
#include <linux/if.h>          // For ifreq, IFNAMSIZ, etc.