#include "header.h"
void handle_readReq_client(int sockfd,const struct sockaddr_in sa, socklen_t sa_len){


	// Setting Timer facility

	fd_set read_fds; // create a set of fds
	FD_ZERO(&read_fds);// fill the set to empty
	FD_SET(sockfd, &read_fds);// adding socket fd to the set

	struct timeval timeout; 
	timeout.tv_sec  = 3; // sec
	timeout.tv_usec = 0; // micro-sec

	// int ready = select(sockfd+1,&read_fds,NULL,NULL,&timeout);
	// if ready == -1 error | ready == 0 timeout | else data available at sockfd before timeout
//////////////////////////////////////////////////////////////////////////////////////////////////

	//declaring variables
	unsigned char buffer[MAXSIZE];// content of the file | read from socket
	char fileN[MAXSIZE];// file name
	const char* mode = "octet";
	unsigned char * packet=NULL;
	int blk = 0; // intially block 0 // expected blk number
	int ready;//o/p of select
	size_t bytes_recv,bytes_write;
	
	// struct variables for extracted data
	ACK_Packet ack; // recev block number
	
	DATA_Packet  dp ;
	
//////////////////////////////////////////////////////////////////////////////////////////

		
		printf("Enter Filename(present in server) to read from: ");
		fgets(fileN,MAXSIZE,stdin);
		fileN[strlen(fileN) - 1]='\0';
		
		FILE * fp = fopen(fileN,"wb");
		if(fp == NULL){
			perror("fopen while write fail");
			exit(1);
		}
// build the read packet
printf("build (read req) packet \n");
packet = build_rrq_wrq_packet(fileN, mode, 1);
// send the packet
printf("send (read req) packet \n");
sendRdPacket:
sendto(sockfd,packet,(strlen(fileN)+strlen(mode)+4), MSG_CONFIRM, (struct sockaddr *)&sa , sa_len);			
// wait for data packet as ack for read req
printf("Wait for data packet as ack for read req\n");

FD_ZERO(&read_fds);// fill the set to empty
FD_SET(sockfd, &read_fds);// adding socket fd to the set
ready = select(sockfd+1,&read_fds,NULL,NULL,&timeout);

blk=1;

if(ready == -1 ){
	perror("selct failed");
	exit(1);
}
else if (ready == 0){// timeout occur => resend
	printf("Timout for data blk %d as ack for read req\n",blk);
	goto sendRdPacket;
}
// else data received within timeout
data_recv:
bzero(buffer,MAXSIZE);
bytes_recv=recvfrom(sockfd, buffer, MAXSIZE, MSG_WAITALL, (struct sockaddr *)&sa , &sa_len);

//recv packet is an err mssg
if((int)buffer[1] == 5){
	printf("Err Mssg Recv = %s",(buffer+4));
	fclose(fp);
	close(sockfd);	
	return ;
}

printf("Received data blk=%d(expected)\n",blk);
printf("Extracting the data block blk =%d(expected)\n",blk);
dp = extract_data_packet(buffer, bytes_recv);
printf("Extracted data block no %d and size = %dB\n",dp.block_number,dp.data_size);


if(dp.block_number == blk -1 ){//dp.blk ==> recv blk ||| blk ==> should be blk 
	free(dp.data);
	goto buildAck2;
// this case occurs when 2 consectutive data blks are same as send ack went miss server reach
//timeout resend data
}


	// store the data block
	printf("Writing data blk %d into file\n",dp.block_number);
	fwrite(dp.data,1,dp.data_size,fp);
	
	//build ack
	buildAck2:
	printf("Build Ack for blk = %d\n",dp.block_number);
	packet = build_ack_packet(dp.block_number);
	
	//send ack
	sendAck2:
	printf("Send Ack for blk = %d\n",dp.block_number);
	sendto(sockfd, packet,4, MSG_CONFIRM, (struct sockaddr *)&sa , sa_len);
	
	if(dp.data_size <512){// the last packet has been received
		//wait for a timeout if the last ack is lost
			// waiting for data packet if ack for blk =dp.block_number received 
		FD_ZERO(&read_fds);// fill the set to empty
		FD_SET(sockfd, &read_fds);// adding socket fd to the set
		ready = select(sockfd+1,&read_fds,NULL,NULL,&timeout);
		
		if(ready == -1 ){
			perror("selct failed");
			exit(1);
		}
		else if (ready == 0){// timeout occur => end
			printf("File recv complete\n");
			//close
			printf("socket and file closed\n");
			free(dp.data);
			fclose(fp);
			close(sockfd);	
			return ;

		}
		//else no timeout=> data recv		
		printf("Last Ack lost thus resend it\n");
		goto sendAck2;
		
	}
	
	// waiting for data packet blk =dp.block_number+1 
	printf("Waiting for data blk %d \n",dp.block_number+1);
		FD_ZERO(&read_fds);// fill the set to empty
		FD_SET(sockfd, &read_fds);// adding socket fd to the set
		ready = select(sockfd+1,&read_fds,NULL,NULL,&timeout);
		
	if(ready == -1 ){
		perror("selct failed");
		exit(1);
	}
	else if (ready == 0){// timeout occur => resend
		printf("Timout for data blk %d\n",dp.block_number+1);
		printf("Resending Ack %d\n",dp.block_number);
		goto sendAck2;
	}
	// if curr data size is 512 go to recv next data block
	if (dp.data_size == 512){
		blk++;
		free(dp.data);
		free(packet);
		packet=NULL;
		goto data_recv;
	}
			


}
