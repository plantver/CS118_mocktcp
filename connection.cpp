#include "connection.hpp"

int Connection::senddg(char type, int num, char* message, int mlen){
	int dgsize = 4 + mlen;
	char* dg = (char*)malloc(dgsize);
	dg[1] = type;
	((int16_t*)dg)[1] = (int16_t)num;
	memcpy(dg + 4, message, mlen);

	if (sendto(socketfd, dg, dgsize, 0, (struct sockaddr *)&remaddr, sizeof(remaddr)) < 0) {
		perror("Send datagram failed");
		free(dg);
		return -1;
	}
	printf("sent message bytes %d\n", strlen(message));
	free(dg);
	return 0;
}

char* Connection::waitforreq(){
	printf("Waiting on port %d\n", myport);
	int bufsize = 2048;
	char buf[bufsize];
	while(1){
		int recvlen = recvfrom( socketfd, buf, bufsize, 0, (struct sockaddr *)&remaddr, &remaddrlen );
	    buf[recvlen] = 0;
	    if(gettype(buf) == 'R'){
	    	int meslen = strlen(getmessage(buf));
		    printf("received %d bytes\n", meslen);
		    printf("received message: \"%s\"\n", getmessage(buf));
		    return getmessage(buf);
		}
		else{
			printf("Received non-request packet\n" );
		}
	}
}