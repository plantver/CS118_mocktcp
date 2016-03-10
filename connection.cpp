#include "connection.hpp"

int Connection::senddg(char type, int num, char* message, int mlen){
	int dgsize = HEADSIZE + mlen;
	char* dg = (char*)malloc(dgsize);
	dg[1] = type;
	((int16_t*)dg)[1] = (int16_t)num;
	memcpy(dg + HEADSIZE, message, mlen);

	if (sendto(socketfd, dg, dgsize, 0, (struct sockaddr *)&remaddr, sizeof(remaddr)) < 0) {
		perror("Send datagram failed");
		free(dg);
		return -1;
	}
	if(type == 'D'){
		printf("SENT>> data, seq. num %d, bytes %d\n", num, mlen + HEADSIZE);
	}
	else if(type == 'A'){
		printf("SENT>> ACK, seq. num %d, bytes %d\n", num, mlen + HEADSIZE);
	}
	else{
		printf("SENT>> packet type %c, seq. num %d, bytes %d\n", type, num, mlen + HEADSIZE);
	}
	free(dg);
	return 0;
}

int Connection::recvdg(char* buffer){
	int recvlen = recvfrom( socketfd, buffer, DGBSIZE, 0, (struct sockaddr *)&remaddr, &remaddrlen );
	buffer[recvlen] = 0; //if want to read as strings
	if(gettype(buffer) == 'D'){
		printf("RECV<< data, seq. num %d, bytes %d\n", getseqnum(buffer), recvlen);
	}
	else if(gettype(buffer) == 'A'){
		printf("RECV<< ACK, seq. num %d, bytes %d\n", getseqnum(buffer), recvlen);
	}
	else{
		printf("RECV<< packet type %c, seq. num %d, bytes %d\n", gettype(buffer), getseqnum(buffer), recvlen);
	}
	return recvlen - HEADSIZE;
}

int Connection::request(char* filename, int len){
	// the seqnum left for congestion control
	return senddg('R', 0, filename, len);
}


char* Connection::waitforreq(){
	printf("Waiting on port %d\n", myport);
	char* buf = (char*)malloc(DGBSIZE); //mem leak careful
	while(1){
		int meslen = recvdg(buf);
	    if(gettype(buf) == 'R'){
	    	//set self WINDOW
	    	WINDOW = getseqnum(buf);
		    return getmessage(buf);
		}
		else{
			printf("Received non-request packet, keep waiting...\n" );
		}
	}
}

int Connection::write(char* base, int len){
	char dgbuf[DGBSIZE];

	//send top sequence number as initializer
	//!THIS PACKET CANNOT BE LOST!
	int topseq = len/(PLSIZE);
	senddg('S', topseq, dgbuf, 0); //here dgbuf is used as a dummy

	char* ackarr = (char*)malloc(topseq + 1);
	memset(ackarr, 0, topseq + 1);

	int basenum = 0;
	int nextnum = 0;
	int acknum = 0;
	int prevack = 0;
	int sacounter = 0;
	while(1){
		(void)recvdg(dgbuf);

		acknum = getseqnum(dgbuf);
		if(acknum == topseq + 1){
			break;
		}
		if(acknum > basenum){
			for(int i = basenum; i < acknum; i++){
				ackarr[i] = 1;
			}
			basenum = acknum;
			if(basenum > nextnum){
				nextnum = basenum;
			}
		}



		//set the corresponding sequence, "according to" the ack
		if(len - (PLSIZE*nextnum) >= PLSIZE){
			senddg('D',nextnum, base + (PLSIZE*nextnum), PLSIZE); //!MUST BE PLSIZE UNLESS END OF BUFFER!
		}
		else{
			senddg('D',nextnum, base + (PLSIZE*nextnum), len - (PLSIZE*nextnum));
		}

		//flow control
		if(nextnum - basenum < WINDOW - 1){
			nextnum++;
		}
	}


	return 0;
}

// helper
int getnextbase(char* arr, int cur, int top){
	int i = cur;
	for( i; i <= top; i++){
		if(arr[i] == 0){
			return i;
		}
	}
	return i;
}

int Connection::read(char* base, int len){
	char dgbuf[DGBSIZE];

	//get top seqnumber, in the seqnum of the transmission initializer packet
	printf("Waiting for tranmission start...\n");
	(void)recvdg(dgbuf);
	senddg('A', -1, dgbuf, 0); // -1 is just a number not in the range of seqnum
	int topseq = getseqnum(dgbuf);
	char* ackarr = (char*)malloc(topseq + 1);
	memset(ackarr, 0, topseq + 1);
	
	int recvflen = 0;
	int seqnum = 0;
	int basenum = 0;
	int pllen = 0;
	while(1){
		pllen = recvdg(dgbuf);
		seqnum = getseqnum(dgbuf);
		if(seqnum >= basenum){
			//copy message into the base buffer
			memcpy((base + (PLSIZE * seqnum)), getmessage(dgbuf), pllen);
			ackarr[seqnum] = 1;
		}
		//compute total len of file..
		if( pllen < PLSIZE || seqnum == topseq){
			recvflen = pllen + (PLSIZE * topseq);
		}
		basenum = getnextbase(ackarr, basenum, topseq);

		// ack for the next packet, seqnum = next packet seqnum
		if(basenum > topseq ){
			senddg('A', basenum, dgbuf, 0);
			break;
		}
		senddg('A', basenum, dgbuf, 0);
	}

	return recvflen;
}