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

int client( char* server, int servportno, char* filename, float pl, float pc){
	Connection con(server, servportno, pl, pc, 'C');
    // send name of request file
	con.request(filename, strlen(filename));
	
    int BUFSIZE = 10000000; //10 MB buffer
    char *buffer = (char*)malloc(BUFSIZE);
    int flen = con.read(buffer, BUFSIZE);
    FILE* fp = fopen( "recv.txt", "w+" );
    fwrite(buffer, 1, flen, fp);
}

int server(int portno, int windowsize, float pl, float pc){
	Connection con(portno, windowsize, pl, pc);
	char* request = con.waitforreq();

    int file_fd;
    if(( file_fd = open(request,O_RDONLY)) == -1) {
        perror("Cannot open request file\n"); exit(1);
    }
    int filelen = lseek(file_fd, (off_t)0, SEEK_END);
    (void)lseek(file_fd, (off_t)0, SEEK_SET); /* reposition file offset to start of file */

    // send file
    char *buffer = (char*)malloc(filelen);
    (void)read(file_fd, buffer, filelen);
    con.write(buffer, filelen);
}


int main(int argc, char *argv[]){
	//open up server
	if (*argv[1] == 's' ||*argv[1] == 'S') { 
                //  port       window                 pl       pc
        server(atoi(argv[2]), atoi(argv[3]), atof(argv[4]), atof(argv[5]));
    }
    else if (*argv[1] == 'r' ||*argv[1] == 'R') { //open up client
                //servname  port         filename     pl        pc
    	client( argv[2], atoi(argv[3]), argv[4], atof(argv[5]), atof(argv[6]));
    }
    else{
    	printf("Unsupported arguments \n");
    	exit(0);
    }


}