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
	printf("sent message bytes %d\n", mlen);
	free(dg);
	return 0;
}

int Connection::getdg(char* buffer){
	int recvlen = recvfrom( socketfd, buffer, DGBSIZE, 0, (struct sockaddr *)&remaddr, &remaddrlen );
	buffer[recvlen] = 0; //if want to read as strings
	return recvlen - HEADSIZE;
}

int Connection::request(char* filename, int len){
	return senddg('R', 0, filename, len);
}


char* Connection::waitforreq(){
	printf("Waiting on port %d\n", myport);
	char* buf = (char*)malloc(DGBSIZE); //mem leak careful
	while(1){
		int meslen = getdg(buf);
	    if(gettype(buf) == 'R'){
		    printf("received %d bytes\n", meslen);
		    printf("received seq %d\n", getseqnum(buf));
		    printf("received request: \"%s\"\n", getmessage(buf));
		    return getmessage(buf);
		}
		else{
			printf("Received non-request packet\n" );
		}
	}
}

int Connection::read(char* base, int len){
	int offset = 0;
	int pllen = 0;
	char dgbuf[DGBSIZE];
	while(offset <= len - PLSIZE){
		pllen = getdg(dgbuf);
		memcpy(base + offset, getmessage(dgbuf), pllen);
		printf("%s\n", base + offset);
		offset += pllen;
	}

}

int Connection::write(char* base, int len){
	int offset = 0;
	int seqnum = 0;
	while(offset < len){
		if(len - offset >= PLSIZE){
			senddg('S',seqnum, base + offset, PLSIZE);
		}
		else{
			senddg('S',seqnum, base + offset, len - offset);
		}
		offset += PLSIZE;
	}
	return 0;
}