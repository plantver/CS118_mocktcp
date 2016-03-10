#ifndef CONNECTION118
#define CONNECTION118

#include <stdio.h>	/* defines printf */
#include <stdlib.h>	/* defines exit and other sys calls */
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <netdb.h>      // define structures like hostent
#include <string.h>	/* for memcpy */
#include <sys/time.h>


class Connection{
	int socketfd;
	struct sockaddr_in myaddr;
	int myport;
	struct sockaddr_in remaddr;
	socklen_t remaddrlen = sizeof(remaddr); // NEED TO INITIALIZE!!!!!!
	int WINDOW;

	float PL = 0.1;
	float PC = 0.1;

	const int HEADSIZE = 4;
	const int PLSIZE = 996; // payload length
	const int DGBSIZE = 1024;

	void setsocket( int to){
		if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("cannot create socket\n");
		}

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = to;
		if (setsockopt (socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0){
        	perror("setsockopt failed\n");
		}

		if (bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
			perror("bind socket failed\n");
		}
	}

	char gettype(char* dg){return dg[1];}

	int16_t getseqnum(char* dg){return ((int16_t*)dg)[1];}

	char* getmessage(char* dg){return dg + HEADSIZE; }

	bool islost(){
		if(PL > ((double)rand() / (double)RAND_MAX))
			return true;
		return false;
	}

	bool iscorrupt(){
		if(PC > ((double)rand() / (double)RAND_MAX))
			return true;
		return false;
	}

public:
	Connection( int myportno, int windowsize, float pl, float pc){ // serverside connection init
		PL = pl;
		PC = pc;
		WINDOW = windowsize/(HEADSIZE + PLSIZE);

		memset(&myaddr, 0, sizeof(myaddr));
		myaddr.sin_family = AF_INET;
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		myaddr.sin_port = htons(myportno); //0 means any, in case of client
		myport = myportno;
		this->setsocket( 20000 );
	}

	Connection( char* remaddrname, int remport, float pl, float pc, char c){ //clientside  connection init
		PL = pl;
		PC = pc;
		//set mysocket
		memset(&myaddr, 0, sizeof(myaddr));
		myaddr.sin_family = AF_INET;
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		myaddr.sin_port = htons(0); //any port?
		myport = 0; //not zero!
		this->setsocket( 500000 );

		// set remote socket
		struct hostent *rem;
		if(!(rem = gethostbyname(remaddrname))){ //takes a string like "www.yahoo.com", and returns a struct hostent which contains information, as IP address, address type, the length of the addresses...
    		printf("error getting hostend \n");
    		exit(1);
    	}
    	memset(&remaddr, 0, sizeof(remaddr));
    	remaddr.sin_family = AF_INET;
    	memcpy((void *)&remaddr.sin_addr, rem->h_addr_list[0], rem->h_length);
    	remaddr.sin_port = htons(remport);
	}

	//send header + payload/message
	// mlen has to be less than PLSIZE
	int senddg(char type, int num, char* message, int mlen);

	//receive the entire packet into the given buffer
	//buffer has to be as least than DGSIZE + 10, just in case
	//an zero is appended
	//returns the length of the payload inside
	int recvdg(char* buffer);

	int request(char* filename, int len);

	//returns a stack malloced char* containning the filename
	//need tobe freed later, or just forget about it
	char* waitforreq(); //server waits for request

	//read til the end or as much as the buffer can hold
	//returns then number of bytes read into the buffer
	int read(char* buffer, int len);

	// write the entirebuffer in datagrams.
	int write(char* buffer, int len);
};


#endif 