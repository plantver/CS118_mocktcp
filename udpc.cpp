// the server of mock

#include <stdio.h>	/* defines printf */
#include <stdlib.h>	/* defines exit and other sys calls */
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <netdb.h>      // define structures like hostent
#include <string.h>	/* for memcpy */
#include <unistd.h>
#include <fcntl.h>

// the protocal
#include "connection.hpp"

int client( char* server, int servportno, char* filename){
	Connection con(server, servportno);
    // send name of request file
	con.request(filename, strlen(filename));
	
    int BUFSIZE = 10000000; //10 MB buffer
    char *buffer = (char*)malloc(BUFSIZE);
    int flen = con.read(buffer, BUFSIZE);
    FILE* fp = fopen( "recv.txt", "w+" );
    fwrite(buffer, 1, flen, fp);
}

int sendfile( Connection& con, char* filename){ //con should always be passed by reference
	int file_fd;
	if(( file_fd = open(filename,O_RDONLY)) == -1) {
        perror("Cannot open request file\n"); exit(1);
    }
    int filelen = lseek(file_fd, (off_t)0, SEEK_END);
    (void)lseek(file_fd, (off_t)0, SEEK_SET); /* reposition file offset to start of file */

    // send file
   	char *buffer = (char*)malloc(filelen);
   	(void)read(file_fd, buffer, filelen);
   	con.write(buffer, filelen);
}

int server(int portno){
	Connection con(portno, 5000);
	char* request = con.waitforreq();
	(void)sendfile(con, request);
}


int main(int argc, char *argv[]){
	//open up server
	int portno;
	if (*argv[1] == 's' ||*argv[1] == 'S') { 
        portno = atoi(argv[2]);
        server(portno);
    }
    else if (*argv[1] == 'r' ||*argv[1] == 'R') { //open up client
    	portno = atoi(argv[3]);
    	client( argv[2], portno, argv[4]);
    }
    else{
    	printf("Unsupported arguments \n");
    	exit(0);
    }


}