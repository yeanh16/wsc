#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include "main.h"
#define MAX_THREADS 5
typedef struct thread_control_block {
	int client;
	struct sockaddr_in6 their_address;
	socklen_t their_address_size;
} thread_control_block_t;

void error(char *msg){
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    //handle control c quits
    int one = 1;
    int reuse = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
    
    //the function bzero sets all values in a buffer to zero. It takes two arguments, the first is a pointer to the buffer and the second is the size of the buffer.
    bzero((char *) &serv_addr, sizeof (serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_port = htons(portno);
    //the third field of sockaddr_in is a structure of type struct in_addr which contains only a single field unsigned long s_addr. This field contains the IP address of the host. For server code, this will alwyas be the IP address of the machine on which the server is running, and there is a symbolic constant INADDR_ANY which gets this address.
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        error("ERROR on binding");
    }
    listen(sockfd, 5);
    clilen = sizeof (cli_addr);
    
    while (1) {
        struct sockaddr_in6 their_addr;
        socklen_t size = sizeof (their_addr);
        
        /*
        thread_control_block_t *tcb_p = malloc(sizeof (*tcb_p->their_address);
        if (tcb_p == 0) {
            perror("malloc");
            exit(1);
        }


        //int newsock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        tcb_p->their_address_size = sizeof (tcb_p->theiraddress);
        if ((tcb_p->client = accept(sockfd, (struct sockaddr *) &(tcb_p->their_address), &(tcb_p->their_address_size))) < 0) {
            pthread_t thread;
            if (pthread_create(&thread, 0, handle, (void *) tcb_p) != 0) {
                exit(1);
                //error
            }
            
        }
         */
	int newsock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsock == -1) {
            perror("accept");
        } else {
            printf("Got a connection\n");
            handle(&newsock);
        }
    }

    return 0;
}

void *handle(void *param) {
    int socket = *(int*) param;
    char buffer[256];
    if (socket < 0) {
        error("ERROR on accept");
    }
    while (1) {
        bzero(buffer, 256);
        int n = read(socket, buffer, 255);
        if (n < 0) error("ERROR reading from socket");
        printf("Here is the message: %s\n", buffer);
        n = write(socket, "I got your message", 18);
        if (n < 0) error("ERROR writing to socket");
    }
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   int n;
   char buffer[256];
      
   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) error("ERROR reading from socket");
   printf("Here is the message: %s\n",buffer);
   n = write(sock,"I got your message",18);
   if (n < 0) error("ERROR writing to socket");
}
