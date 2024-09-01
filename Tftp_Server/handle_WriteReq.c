#include "header.h"
void handle_writeReq_server(int sockfd,const struct sockaddr_in ca, socklen_t ca_len,unsigned char *buffer,RRQ_WRQ_Packet rd_wr){

// Setting Timer facility

fd_set read_fds; // create a set of fds
FD_ZERO(&read_fds);// fill the set to empty
FD_SET(sockfd, &read_fds);// adding socket fd to the set

struct timeval timeout; 
timeout.tv_sec  = 3; // sec
timeout.tv_usec = 0; // micro-sec

// int ready = select(sockfd+1,&read_fds,NULL,NULL,&timeout);
// if ready == -1 error | ready == 0 timeout | else data available at sockfd before timeout

///////////////////////////////////////////////////////////////////////////////////////////////////


	unsigned char * packet;	
	FILE *fp;	
	int bytes_sent,	bytes_recv, ready ,blk=0;
	
	DATA_Packet  dp ;
	
///////////////////////////////////////////////////////////////////////////////////////////////////

		
		printf("Received wrt req from client for file = %s\n",rd_wr.filename);
		
		fp = fopen(rd_wr.filename,"wb");
		if(fp == NULL){
			perror("fopen while write fail");
			exit(1);
		}
		
		// sending ack for wrt packet
		printf("Build Ack 0\n");
		packet = build_ack_packet(0);
		sendAck:
		printf("Send Ack 0\n");
		sendto(sockfd, packet,4, MSG_CONFIRM, (struct sockaddr *)&ca , ca_len);
		
		blk=1;
		// waiting for data packet if ack for blk =0 received 		
		FD_ZERO(&read_fds);// fill the set to empty
		FD_SET(sockfd, &read_fds);// adding socket fd to the set
		ready = select(sockfd+1,&read_fds,NULL,NULL,&timeout);
		
		if(ready == -1 ){
			perror("selct failed");
			exit(1);
		}
		else if (ready == 0){// timeout occur => resend
			printf("Timout for data blk %d\n",blk);
			goto sendAck;
		}
		// data recv
data_recv:
bzero(buffer,MAXSIZE);
bytes_recv = recvfrom(sockfd, buffer, MAXSIZE, MSG_WAITALL, (struct sockaddr *)&ca , &ca_len);

if((int)buffer[1] == 2) goto sendAck; // if wrt req is recv again send ack again

printf("Received data blk=%d(expected)\n",blk);
free(packet);
packet=NULL;

printf("Extracting the data block blk =%d(expected)\n",blk);
dp = extract_data_packet(buffer, bytes_recv);
printf("Extracted data block no %d and size = %dB\n",dp.block_number,dp.data_size);

if(dp.block_number == blk -1 ){//dp.blk == recv blk || blk == should be blk 
	free(dp.data);
	goto buildAck2;
// this case occurs when 2 consectutive data blks are same as send ack went miss client reach
//timeout resend data
}


// store the data block
	printf("Writing data blk %d into file\n",dp.block_number);
	fwrite(dp.data,1,dp.data_size,fp);
	//build ack
	buildAck2:
	printf("Build Ack for blk = %d\n",dp.block_number);
	packet = build_ack_packet(dp.block_number);
	
	//sleep(5);
	//send ack
	sendAck2:
	printf("Send Ack for blk = %d\n",dp.block_number);
	sendto(sockfd, packet,4, MSG_CONFIRM, (struct sockaddr *)&ca , ca_len);
	
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
		goto data_recv;
	}
			
	//close
	printf("socket closed\n");
	free(rd_wr.filename);
	free(rd_wr.mode);
	close(sockfd);
}
