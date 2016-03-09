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
		printf("SENT>> data packet, seq. num %d, bytes %d\n", num, mlen + HEADSIZE);
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

int Connection::getdg(char* buffer){
	int recvlen = recvfrom( socketfd, buffer, DGBSIZE, 0, (struct sockaddr *)&remaddr, &remaddrlen );
	buffer[recvlen] = 0; //if want to read as strings
	if(gettype(buffer) == 'D'){
		printf("GOT<< data packet, seq. num %d, bytes %d\n", getseqnum(buffer), recvlen);
	}
	else if(gettype(buffer) == 'A'){
		printf("GOT<< ACK, seq. num %d, bytes %d\n", getseqnum(buffer), recvlen);
	}
	else{
		printf("GOT<< packet type %c, seq. num %d, bytes %d\n", gettype(buffer), getseqnum(buffer), recvlen);
	}
	return recvlen - HEADSIZE;
}

int Connection::request(char* filename, int len, int windowsize){
	//set seqnum of request to be the window size,
	//a hack, because both party use same datagram size.
	//also set self WINDOW
	WINDOW = windowsize/(PLSIZE + HEADSIZE);
	return senddg('R', WINDOW, filename, len);
}


char* Connection::waitforreq(){
	printf("Waiting on port %d\n", myport);
	char* buf = (char*)malloc(DGBSIZE); //mem leak careful
	while(1){
		int meslen = getdg(buf);
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

int Connection::read(char* base, int len){
	char dgbuf[DGBSIZE];
	int pllen = 0;

	//get total seqnumber, in the seqnum of the transmission initializer packet
	printf("Waiting for tranmission start...\n");
	(void)getdg(dgbuf);
	senddg('A', -1, dgbuf, 0);
	int totseq = getseqnum(dgbuf);
	char* ackarr = (char*)malloc(totseq);
	memset(ackarr, 0, totseq);
	// try not use window size first...
	// NEED TO IMP LATER!!!
	
	int offset = 0;
	int seqnum = 0;
	while(offset <= len - PLSIZE){
		pllen = getdg(dgbuf);
		seqnum = getseqnum(dgbuf);
		senddg('A', seqnum, dgbuf, 0);


		//copy message into the base buffer
		memcpy(base + offset, getmessage(dgbuf), pllen);
		offset += pllen;
	}

	base[offset] = 0;
	return offset;
}

int Connection::write(char* base, int len){
	char dgbuf[DGBSIZE];

	//send total sequence number as initializer
	//THIS PACKET CANNOT BE LOST
	int totseq = len/(PLSIZE) + 1;
	if(len%PLSIZE == 0){ totseq--; }
	senddg('S', totseq, dgbuf, 0); //here dgbuf is used as a dummy

	int offset = 0;
	int seqnum = 0;
	while(offset < len){
		(void)getdg(dgbuf);

		//set the corresponding sequence, "according to" the ack
		if(len - offset >= PLSIZE){
			senddg('D',seqnum, base + offset, PLSIZE);
		}
		else{
			senddg('D',seqnum, base + offset, len - offset);
		}
		seqnum++;
		offset += PLSIZE;
	}
	return 0;
}