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
	{0,0} };

void error(char *msg){
	perror(msg);
	exit(1);
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
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        error("ERROR on binding");
    }
    listen(sockfd, 5);
    clilen = sizeof (cli_addr);
    struct sockaddr_in6 their_addr;
    socklen_t size = sizeof (their_addr);
    while (1) {

        
        
        thread_control_block_t *tcb_p = malloc(sizeof (*tcb_p->their_address);
        if (tcb_p == 0) {
            perror("malloc");
            exit(1);
        }


        int newsock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        tcb_p->their_address_size = sizeof (tcb_p->theiraddress);
        if ((tcb_p->client = accept(sockfd, (struct sockaddr *) &(tcb_p->their_address), &(tcb_p->their_address_size))) < 0) {
            pthread_t thread;
            if (pthread_create(&thread, 0, handle, (void *) newsock) != 0) {
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
    /*
    int file_fd;
    long ret;
    int socket = *(int*) param;
    char buffer[BUFLEN];
    if (socket < 0) {
        error("ERROR on accept");
    }
    while (1) {
        bzero(buffer, BUFLEN);

	//write(socket, "HTTP/1.1 200 OK\n", 16);
	//write(socket, "Content-length: 46\n", 19);
	//write(socket, "Content-Type: text/html\n\n", 25);
	//write(socket, "<html><body><H1>Hello world</H1></body></html>",46);
        
	//the request message
	//GET / HTTP/1.1
	//User-Agent: Wget/1.15 (linux-gnu)
	//Accept: ''''''''*
	//Host: localhost:9999
	//Connection: Keep-Alive
	
	int n = read(socket, buffer, BUFLEN-1);
	printf("Request:\r\n%s\n", buffer);
	if (!( strncmp(buffer,"GET ",4) && strncmp(buffer, "get ",4) )){
		//get request
		//get rid of the request type
		int i;
		for(i=4;i<BUFLEN;i++) { 
			if(buffer[i] == ' ') { 
				buffer[i] = 0;
				break;
			}
		}
		
		if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) 
			(void)strcpy(buffer,"GET /index.html");
		
		int buflen_temp, len;
		buflen_temp=strlen(buffer);
		char * fstr = (char *)0;
		for(i=0;extensions[i].ext != 0;i++) {
			len = strlen(extensions[i].ext);
			if( !strncmp(&buffer[buflen_temp-len], extensions[i].ext, len)) {
				fstr =extensions[i].filetype;
				break;
			}
		}
		
		
		if(( file_fd = open(&buffer[5],O_RDONLY)) == -1)
			error("unable to open file");
		int length = get_file_size(file_fd);
		//(void)sprintf(buffer, "HTTP/1.0 200 OK\r\nContent-length: %d\r\nContent-Type: %s\r\n",length,fstr);
		(void)sprintf(buffer, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n",fstr);
		(void)write(socket,buffer,strlen(buffer));
		
		
		//bzero(buffer,BUFLEN);
		//while( (ret=read(file_fd,buffer,BUFLEN)) > 0){
		//	(void)write(socket,buffer,ret);
		//}
		
		size_t total_bytes_sent = 0;
       		ssize_t bytes_sent;
       		while (total_bytes_sent < length) {
        	//Zero copy optimization
        		if ((bytes_sent = sendfile(socket, file_fd, 0, length - total_bytes_sent)) <= 0) {
         			if (errno == EINTR || errno == EAGAIN) {
          				continue;
         			}
         		perror("sendfile");
         		return;
        		}
        	total_bytes_sent += bytes_sent;
       		}
		
	}
		
	
        if (n < 0) error("ERROR reading from socket");
        printf("%s\n", buffer);
        //n = write(socket, "I got your message", 18);
        if (n < 0) error("ERROR writing to socket");
    }
    
    exit(1); 
    */
    	
    	int j, file_fd, buflen, len;
	long i, ret;
	char * fstr;
	static char buffer[BUFLEN+1];
	int fd =  *(int*) param;
	ret =read(fd,buffer,BUFLEN); 
	
	//print request buffer
	printf("Request:\r\n%s\n", buffer);
	
	if(ret > 0 && ret < BUFLEN)	
		buffer[ret]=0;	
	else buffer[0]=0;

	for(i=0;i<ret;i++)	
		if(buffer[i] == '\r' || buffer[i] == '\n')
			buffer[i]='*';

	for(i=4;i<BUFLEN;i++) { 
		if(buffer[i] == ' ') { 
			buffer[i] = 0;
			break;
		}
	}
	
	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) 
		(void)strcpy(buffer,"GET /index.html");

	buflen=strlen(buffer);
	fstr = (char *)0;
	for(i=0;extensions[i].ext != 0;i++) {
		len = strlen(extensions[i].ext);
		if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
			fstr =extensions[i].filetype;
			break;
		}
	}

	if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) 
		error("File not supported");


	(void)sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
	(void)write(fd,buffer,strlen(buffer));
	
	//print response buffer
	printf("Response:\r\n%s\n", buffer);
	
	while (	(ret = read(file_fd, buffer, BUFLEN)) > 0 ) {
		(void)write(fd,buffer,ret);
	}
	
	pthread_exit(pthread_self());
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