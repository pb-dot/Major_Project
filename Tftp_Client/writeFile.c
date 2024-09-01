#include "header.h"
void handle_writeReq_client(int sockfd,const struct sockaddr_in sa, socklen_t sa_len){


	// Setting Timer facility

	fd_set read_fds; // create a set of fds
	FD_ZERO(&read_fds);// fill the set to empty
	FD_SET(sockfd, &read_fds);// adding socket fd to the set

	struct timeval timeout; 
	timeout.tv_sec  = 3; // sec
	timeout.tv_usec = 0; // micro-sec

	// int ready = select(sockfd+1,&read_fds,NULL,NULL,&timeout);
	// if ready == -1 error | ready == 0 timeout | else data available at sockfd before timeout

////////////////////////////////////////////////////////////////////////////////////////////

	//declaring variables
	unsigned char buffer[MAXSIZE];// content of the file | read from socket
	char fileN[MAXSIZE];// file name
	const char* mode = "octect";
	unsigned char * packet=NULL;
	int blk = 0; // intially block 0 // expected blk number
	int ready;//o/p of select
	size_t bytes_read,bytes_write;
	
	// struct variables for extracted data
	ACK_Packet ack; // recev block number
	
//////////////////////////////////////////////////////////////////////////////////////////

		
		printf("Enter Filename(present in client) to send to server: ");
		fgets(fileN,MAXSIZE,stdin);
		fileN[strlen(fileN) - 1]='\0';
		
		FILE * fp = fopen(fileN,"rb");
		if(fp == NULL){
			perror("fopen while read fail");
			exit(1);
		}
				
// build the wrt packet
printf("build (wrt req) packet \n");
packet = build_rrq_wrq_packet(fileN, mode, 2);
// send the packet
printf("send (wrt req) packet \n");
sendWrtPacket:
sendto(sockfd,packet,(strlen(fileN)+strlen(mode)+4), MSG_CONFIRM, (struct sockaddr *)&sa , sa_len);			
// wait for ack with block zero
printf("Wait for ack for wrt req\n");

FD_ZERO(&read_fds);// fill the set to empty
FD_SET(sockfd, &read_fds);// adding socket fd to the set
ready = select(sockfd+1,&read_fds,NULL,NULL,&timeout);

if(ready == -1 ){
	perror("selct failed");
	exit(1);
}
else if (ready == 0){// timeout occur => resend
	printf("Timout for ack with block no =0\n");
	goto sendWrtPacket;
}
// else ack received within timeout
bzero(buffer,MAXSIZE);
recvfrom(sockfd, buffer, MAXSIZE, MSG_WAITALL, (struct sockaddr *)&sa , &sa_len);

if((int)buffer[1] != 4) goto quit;// recv packet is not ack

ack = extract_ack_packet(buffer);
if(ack.block_number != 0) goto sendWrtPacket;
	
printf("ACK received for block %d (wrt req)\n",ack.block_number);
free(packet);// del the wrt req packet
packet=NULL;

// try to break file into blocks of size 512B	
blk=0; // uid of each block		
bzero(buffer,MAXSIZE);
while((bytes_read = fread(buffer,1,512,fp))>0){
	blk++;
	//build data packet
	printf("building data packet with block_no = %d and size %ld\n",blk,bytes_read);
	packet = build_data_packet(blk, buffer, bytes_read);
	
	//sleep(5);
	//send data packet
	sendDataPacket:
	printf("sending data packet with block_no = %d and size %ld\n",blk,bytes_read);	
	sendto(sockfd, packet,(bytes_read+4), MSG_CONFIRM, (struct sockaddr *)&sa , sa_len);
	//wait for ack for that data packet
	printf("Waiting for ack for block_no =%d\n",blk);
	
	FD_ZERO(&read_fds);// fill the set to empty
	FD_SET(sockfd, &read_fds);// adding socket fd to the set
	ready = select(sockfd+1,&read_fds,NULL,NULL,&timeout);
	
	if(ready == -1 ){
		perror("selct failed");
		exit(1);
	}
	else if (ready == 0){// timeout occur => resend
		printf("Timout for ack with block no =%d\n",blk);
		goto sendDataPacket;
	}
	//else ack received
	bzero(buffer,MAXSIZE);
	recvfrom(sockfd, buffer, MAXSIZE, MSG_WAITALL, (struct sockaddr *)&sa , &sa_len);
	ack = extract_ack_packet(buffer);		
	printf("ACK received for block %d; when expected blk %d\n",ack.block_number,blk);
	
	if(ack.block_number == blk-1) goto sendDataPacket; // resend data
	
	//else build next packet of blk+1
	free(packet);// del the data packet
	packet=NULL;
	bzero(buffer,MAXSIZE);
}//end of while
		printf("File Sent Complete\n");	


	//close
	quit:
	printf("socket and file closed\n");
	fclose(fp);
	close(sockfd);



}
