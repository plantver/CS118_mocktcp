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

int getfile(Connection& con){
	int BUFSIZE = 99999;
	char *buffer = (char*)malloc(BUFSIZE);
	con.read(buffer, BUFSIZE);
}

int client( char* server, int servportno ){
	Connection con(server, servportno);
	char* mes = "t.txt";
    // send name of request file, along with the window size
	con.request(mes, strlen(mes), 5000);
	(void)getfile(con);
}

int sendfile( Connection& con, char* filename){ //con should always be passed by reference
	int file_fd;
	if(( file_fd = open(filename,O_RDONLY)) == -1) {
        printf("ERROR cannot open resquested file");
        exit(1);
    }
    int filelen = lseek(file_fd, (off_t)0, SEEK_END);
    (void)lseek(file_fd, (off_t)0, SEEK_SET); /* reposition file offset to start of file */

    // send file
   	char *buffer = (char*)malloc(filelen);
   	(void)read(file_fd, buffer, filelen);
   	con.write(buffer, filelen);
}

int server(int portno){
	Connection con(portno);
	char* request = con.waitforreq();
	//debug
	char* test = "debugging";
	//con.senddg('T', 0, test, strlen(test));
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
    	client( argv[2], portno );
    }
    else{
    	printf("Unsupported arguments \n");
    	exit(0);
    }


}