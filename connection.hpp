#ifndef CONNECTION118
#define CONNECTION118

#include <stdio.h>	/* defines printf */
#include <stdlib.h>	/* defines exit and other sys calls */
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <netdb.h>      // define structures like hostent
#include <string.h>	/* for memcpy */


class Connection{
	int socketfd;
	struct sockaddr_in myaddr;
	int myport;
	struct sockaddr_in remaddr;
	socklen_t remaddrlen;

	void setsocket(){
		if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("cannot create socket");
		}
		if (bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
			perror("bind socket failed");
		}
	}

	char gettype(char* dg){return dg[1];}

	int16_t getseqnum(char* dg){return ((int16_t*)dg)[1];}

	char* getmessage(char* dg){return dg + 4; }

public:
	Connection(){
		memset(&myaddr, 0, sizeof(myaddr));
		myaddr.sin_family = AF_INET;
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	void setmyport(int portno ){
		myport = portno;
		myaddr.sin_port = htons(portno); //0 means any, in case of client
		this->setsocket();
	}

	Connection( int myportno ){ // serverside connection init
		memset(&myaddr, 0, sizeof(myaddr));
		myaddr.sin_family = AF_INET;
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		myaddr.sin_port = htons(myportno); //0 means any, in case of client
		myport = myportno;
		this->setsocket();
	}

	Connection( char* remaddrname, int remport ){ //clientside  connection init
		//set mysocket
		memset(&myaddr, 0, sizeof(myaddr));
		myaddr.sin_family = AF_INET;
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		myaddr.sin_port = htons(0); //any port?
		myport = 0; //not zero!
		this->setsocket();

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


	int senddg(char type, int num, char* message, int mlen);

	char* waitforreq(); //server waits for request

};


#endif 