
// how to use -->prithi@HP$$>./client.out <ServerIP> <ServerPort> <choice>
// choice = r (read to server)/ w(write toserever)
#include "header.h"

void check_err(int fd,char * mssg){
	if(fd <0){
		perror(mssg);
		exit(1);
	}	
}

int main(int argc, char **argv){
	// checking command line args
	if (argc <4){
		printf("Insufficient command line argument\n");
		exit(1);
	}
			
	//creating the socket
	int sockfd= socket(AF_INET,SOCK_DGRAM,0);
	check_err(sockfd,"Error in opening UDP socket");
	
	// setting server details
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);
	bzero(&sa,sa_len);	
	sa.sin_family=AF_INET;//setting address family to IPv4
	sa.sin_port=htons(atoi(argv[2]));
	//converting from presentable format to networkformat and storing in sa.sin_addr
	int status = inet_pton(AF_INET,argv[1],&sa.sin_addr);
	check_err(status,"Error in client ip conversion format");
	
///////////////////////////////////////////////////////////////////////////////////////////////////
	
	// User Menu
	
	if(argv[3][0] == 'r'){
		handle_readReq_client(sockfd,sa,sa_len); // reads data from server (Cread.c)
	}
	else if(argv[3][0] == 'w') {
		handle_writeReq_client(sockfd,sa,sa_len); // sends data to server (Cwrite.c)
	}	
	else{
		printf("Wrong choice of %s \n",argv[3]);
		close(sockfd);
		return 0;
	}
 		
	return 0;
}
