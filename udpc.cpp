// the server of mock

#include <stdio.h>	/* defines printf */
#include <stdlib.h>	/* defines exit and other sys calls */
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <netdb.h>      // define structures like hostent
#include <string.h>	/* for memcpy */

// the protocal
#include "connection.hpp"


int server(int portno){
	Connection con(portno);
	(void*)con.waitforreq();
}

int client( char* server, int servportno ){
	Connection con(server, servportno);
	char* mes = "test message";
	con.senddg('R', 0, mes, strlen(mes));
}


int main(int argc, char *argv[]){
	//open up server
	int portno;
	if (argc == 2) { 
        portno = atoi(argv[1]);
        server(portno);
    }
    else if (argc == 3) { //open up client
    	portno = atoi(argv[2]);
    	client( argv[1], portno );
    }
    else{
    	printf("enter port for server, or server address and port for client \n");
    	exit(0);
    }


}