
// how to use -->prithi@HP$./server.out <ServerPort>
#include "header.h"

/*

Order of Headers

                                                  2 bytes
    ----------------------------------------------------------
   |  Local Medium  |  Internet  |  Datagram  |  TFTP Opcode  |
    ----------------------------------------------------------

TFTP Formats

   Type   Op #     Format without header

          2 bytes    string   1 byte     string   1 byte
          -----------------------------------------------
   RRQ/  | 01/02 |  Filename  |   0  |    Mode    |   0  |
   WRQ    -----------------------------------------------
          2 bytes    2 bytes       n bytes
          ---------------------------------
   DATA  | 03    |   Block #  |    Data    |
          ---------------------------------
          2 bytes    2 bytes
          -------------------
   ACK   | 04    |   Block #  |
          --------------------
          2 bytes  2 bytes        string    1 byte
          ----------------------------------------
   ERROR | 05    |  ErrorCode |   ErrMsg   |   0  |
   	  ----------------------------------------

Note1:    The data field is from zero to 512 bytes long.  If it is 512 bytes
   	  long, the block is not the last block of data; if it is from zero to
   	  511 bytes long, it signals the end of the transfer.
   
Note2:  a.A WRQ is acknowledged with an ACK packet having a block number of zero.
	b.The WRQ and DATA packets are acknowledged by ACK or ERROR packets
	c.The RRQ and ACK packets are acknowledged by  DATA  or ERROR packets.
	d. All  packets other than duplicate ACK's and those used for termination are acknowledged unless a timeout occurs	


*/

void check_err(int fd,char * mssg){
	if(fd <0){
		perror(mssg);
		exit(1);
	}	
}


int main(int argc, char **argv){

	// checking command line args
	if (argc <2){
		printf("Didnt enter server-port number as command line argument\n");
		exit(1);
	}
	
	//creating the socket
	int sockfd= socket(AF_INET,SOCK_DGRAM,0);
	check_err(sockfd,"Error in opening UDP socket");
	
	// setting server details
	struct sockaddr_in sa;
	bzero(&sa,sizeof(sa));
	sa.sin_family=AF_INET;//setting address family to IPv4
	sa.sin_port=htons(atoi(argv[1]));
	sa.sin_addr.s_addr=INADDR_ANY;
	
	// binding
	int status= bind(sockfd,(struct sockaddr *) &sa, sizeof(sa));
	check_err(status,"Error in binding");
	
	//optional messages
	printf("\nServer started waiting for client ...\n\n");
			
	//declare client addr
	struct sockaddr_in ca;
	socklen_t ca_len = sizeof(ca);
	bzero(&ca,ca_len);
///////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned char buffer[MAXSIZE];
	RRQ_WRQ_Packet rd_wr;
	
	// receive from client (rd/wrt packet)
	printf("Waiting for read or write packet\n");
	bzero(buffer,MAXSIZE);
        recvfrom(sockfd, buffer, MAXSIZE, MSG_WAITALL, (struct sockaddr *)&ca , &ca_len);
        printf("Extracting from read/wrt req\n");
        rd_wr = extract_rrq_wrq_packet(buffer);

        
	 if ((int)buffer[1] == 1){ // read req from client
	 	handle_readReq_server(sockfd,ca,ca_len,buffer,rd_wr);
	 }	
	 else if((int)buffer[1] == 2){ // wrt req from client
		handle_writeReq_server(sockfd,ca,ca_len,buffer,rd_wr);
	 }// end of wrt handle
	 else{
	 	// build error packet and sent to client
	 }

	
	return 0;
}













