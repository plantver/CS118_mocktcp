// the server of mock

#include <stdio.h>	/* defines printf */
#include <stdlib.h>	/* defines exit and other sys calls */
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <netdb.h>      // define structures like hostent
#include <string.h>	/* for memcpy */


int server(int portno){
	int socketfd;
	if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket");
		return 0;
	}

	//my socket
	struct sockaddr_in myaddr;
	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(portno); //0 means any, in case of client

	if (bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("server bind socket failed");
		return 0;
	}

	//client socket
	struct sockaddr_in cliaddr;
	memset(&myaddr, 0, sizeof(cliaddr));
	socklen_t cliaddrlen;

	const int BUFSIZE = 1024;
	unsigned char buf[BUFSIZE];
	int recvlen = 0;
	while(1){
		printf("Waiting on port %d\n", portno);
		recvlen = recvfrom( socketfd, buf, BUFSIZE, 0, (struct sockaddr *)&cliaddr, &cliaddrlen );
        printf("received %d bytes\n", recvlen);
	    if (recvlen > 0) {
	            buf[recvlen] = 0;
	            printf("received message: \"%s\"\n", buf);
	    }
	}
}


int client( struct hostent* server, int servportno ){

	//server socket
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	memcpy((void *)&servaddr.sin_addr, server->h_addr_list[0], server->h_length);
	servaddr.sin_port = htons(servportno);

	//my socket
	struct sockaddr_in myaddr;
	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0); //0 means any, in case of client
	int socketfd;
	if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket");
		return 0;
	}
	if (bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("client bind socket failed");
		return 0;
	}

	char* message = "test message";
	if (sendto(socketfd, message, strlen(message), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("client sendto failed");
		return 0;
	}

}


int main(int argc, char *argv[]){
	//open up server
	int portno;
	if (argc == 2) { 
        portno = atoi(argv[1]);
        server(portno);
    }
    else if (argc == 3) { //open up client
    	struct hostent *server; //contains tons of information, including the server's IP address
    	if(!(server = gethostbyname(argv[1]))){ //takes a string like "www.yahoo.com", and returns a struct hostent which contains information, as IP address, address type, the length of the addresses...
    		printf("error getting hostend \n");
    		exit(0);
    	}
    	portno = atoi(argv[2]);
    	client( server, portno );
    }
    else{
    	printf("enter port for server, or server address and port for client \n");
    	exit(0);
    }


}