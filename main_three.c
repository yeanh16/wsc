#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "main_three.h"

#define MAX_THREADS 5
#define BUFLEN 9999

typedef struct thread_control_block {
	int client;
	struct sockaddr_in6 their_address;
	socklen_t their_address_size;
} thread_control_block_t;

struct {
	char *ext;
	char *filetype;
} extensions [] = {
	{"gif", "image/gif" },  
	{"jpg", "image/jpeg"}, 
	{"jpeg","image/jpeg"},
	{"png", "image/png" },  
	{"zip", "image/zip" },  
	{"gz",  "image/gz"  },  
	{"tar", "image/tar" },  
	{"htm", "text/html" },  
	{"html","text/html" },  
	{"php", "image/php" },  
	{"cgi", "text/cgi"  },  
	{"asp","text/asp"   },  
	{"jsp", "image/jsp" },  
	{"xml", "text/xml"  },  
	{"js","text/js"     },
   	{"css","test/css"   }, 
   	{"ico","image/x-icon"},
	{0,0} };

void error(char *msg){
	perror(msg);
	//exit(1);
}

/*
 Alternative to write
 */
void write_new(int sock, char *msg) {
 int len = strlen(msg);
 if (send(sock, msg, len, 0) == -1) {
  printf("Error in writing\n");
 }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in6 serv_addr, cli_addr;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    //handle control c quits
    int one = 1;
    int reuse = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
    
    bzero((char *) &serv_addr, sizeof (serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin6_port = htons(portno);
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_family = AF_INET6;
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        error("ERROR on binding");
    }
    listen(sockfd, 5);
    clilen = sizeof (cli_addr);
    struct sockaddr_in6 their_addr;
    socklen_t size = sizeof (their_addr);
    while (1) {

        
        
        thread_control_block_t *tcb_p = malloc(sizeof (thread_control_block_t));
        if (tcb_p == 0) {
            perror("malloc");
            exit(1);
        }


        tcb_p->their_address_size = sizeof (tcb_p->their_address);
        if ((tcb_p->client = accept(sockfd, (struct sockaddr *) &(tcb_p->their_address), &(tcb_p->their_address_size))) >= 0) {
            pthread_t thread;
            if (pthread_create(&thread, 0, handle, (void *) tcb_p) != 0) {
                exit(1);
                //error
                error("Error creating new thread");
            }
            
        }
        
	//int newsock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	//pthread_t thread;
	//if (pthread_create(&thread, 0, handle, &newsock)) error("Error creating thread");
        /*
	if (newsock == -1) {
            perror("accept");
        } else {
            printf("Got a connection\n");
            handle(&newsock);
        }
	*/
    }

    return 0;
}

void *handle(void *param) {
    	
    	int j, file_fd, buflen, len;
	long i, ret;
	char * fstr;
	static char buffer[BUFLEN+1];
	thread_control_block_t * tcb_p = param;
	int fd = tcb_p->client;
	ret =read(fd,buffer,BUFLEN); 
	
	//print request buffer
	printf("Request:\r\n%s\n", buffer);
	
	/*
	//check that the contents are within range for the buffer and add terminator to the end
	if(ret > 0 && ret < BUFLEN)	
		buffer[ret]=0;	
	else buffer[0]=0;
	printf("buffer after 0's:\r\n%s\r\nEND\r\n",buffer);
	
	//replace returns and newlines with '*'
	for(i=0;i<ret;i++)	
		if(buffer[i] == '\r' || buffer[i] == '\n')
			buffer[i]='*';
	printf("buffer after 0's:\r\n%s\r\nEND\r\n",buffer);
	*/
	
	
	//add terminator to end of the request part of the buffer as we are only interested in this (and not the headers)
	for(i=4;i<BUFLEN;i++) { 
		if(buffer[i] == ' ') { 
			buffer[i] = 0;
			break;
		}
	}
	
	//printf("buffer after 0's:\r\n%s\r\nEND\r\n",buffer);
	
	
	//null url handling
	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) 
		(void)strcpy(buffer,"GET /index.html");
	
	//GET requests
	if( strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ){ 
		buflen=strlen(buffer);
		fstr = (char *)0;
		for(i=0;extensions[i].ext != 0;i++) {
			len = strlen(extensions[i].ext);
			if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
				fstr =extensions[i].filetype;
				break;
			}
		}
	
		//see if file is available
		if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) 
			error("File not supported");

		//send response
		int length = get_file_size(file_fd);
		(void)sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
		(void)write(fd,buffer,strlen(buffer));
	
		//print response buffer
		printf("Response:\r\n%s\n", buffer);
	
		//write contents of the requested file
		while (	(ret = read(file_fd, buffer, BUFLEN)) > 0 ) {
			(void)write(fd,buffer,ret);
		}
	}
	//HEAD requests
	else if (strncmp(&buffer[0], "HEAD", 4)){
		buflen = strlen(buffer);
		fstr = (char *)0;
		for(i=0;extensions[i].ext != 0;i++) {
			len = strlen(extensions[i].ext);
			if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
				fstr =extensions[i].filetype;
				break;
			}
		}
		
		//see if file is available
		if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) 
			error("File not supported");

		//send response
		int length = get_file_size(file_fd);
		(void)sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent Length: %d\r\n", fstr,length);
		(void)write(fd,buffer,strlen(buffer));
	
		//print response buffer
		printf("Response:\r\n%s\n", buffer);
	}
	
	//close connection
	close(tcb_p->client);
	free(tcb_p);
	pthread_exit(0);
}

int get_file_size(int fd) {
	struct stat stat_struct;
	if (fstat(fd, &stat_struct) == -1)
  		return (1);
 	return (int) stat_struct.st_size;
}


int parse(const char* line){
    /* Find out where everything is */
    const char *start_of_path = strchr(line, ' ') + 1;
    const char *start_of_query = strchr(start_of_path, '?');
    const char *end_of_query = strchr(start_of_query, ' ');

    /* Get the right amount of memory */
    char path[start_of_query - start_of_path];
    char query[end_of_query - start_of_query];

    /* Copy the strings into our memory */
    strncpy(path, start_of_path,  start_of_query - start_of_path);
    strncpy(query, start_of_query, end_of_query - start_of_query);

    /* Null terminators (because strncpy does not provide them) */
    path[sizeof(path)] = 0;
    query[sizeof(query)] = 0;

    /*Print */
    printf("%s\n", query);
    printf("%s\n", path);
}
