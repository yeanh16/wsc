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
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "main_threeSSL.h"

#define BUFLEN 9999

typedef struct thread_control_block {
	int client;
	struct sockaddr_in6 their_address;
	socklen_t their_address_size;
	SSL *cSSL;
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

SSL_CTX* InitServerCTX(void){   
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    
    SSL_library_init();
    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    method = SSLv23_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
    /* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

void ShutdownSSL(SSL *ssl){
	SSL_shutdown(ssl);
	SSL_free(ssl);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    SSL_CTX *sslctx = InitServerCTX();
    LoadCertificates(sslctx, "server.cert", "server.key");
    SSL *cSSL;
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
    while (1) {

        
        
        thread_control_block_t *tcb_p = malloc(sizeof (thread_control_block_t));
        if (tcb_p == 0) {
            perror("malloc");
            exit(1);
        }

	
        tcb_p->their_address_size = sizeof (tcb_p->their_address);
        if ((tcb_p->client = accept(sockfd, (struct sockaddr *) &(tcb_p->their_address), &(tcb_p->their_address_size))) >= 0) {
            sslctx = SSL_CTX_new( SSLv23_server_method());
            SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);
            int use_cert = SSL_CTX_use_certificate_file(sslctx, "/mycert.pem" , SSL_FILETYPE_PEM);
	    int use_prv = SSL_CTX_use_PrivateKey_file(sslctx, "/mycert.pem", SSL_FILETYPE_PEM);
	    cSSL = SSL_new(sslctx);
	    SSL_set_fd(cSSL, tcb_p->client );
	    tcb_p->cSSL = cSSL;
	    
	    int ssl_err = SSL_accept(cSSL);
	    if(ssl_err <= 0){
	    	//Error occurred, log and close down ssl
	    	ShutdownSSL(cSSL);
	    }
	    
            pthread_t thread;
            if (pthread_create(&thread, 0, handle, (void *) tcb_p) != 0) {
                exit(1);
                //error
                error("Error creating new thread");
            }
            
        }
      
    }

    return 0;
}

void *handle(void *param) {
    	
    	int file_fd, buflen, len;
	long i, j, ret;
	char * fstr;
	static char buffer[BUFLEN+1];
	thread_control_block_t * tcb_p = param;
	SSL * cSSL = tcb_p->cSSL;
	int fd = tcb_p->client;
	ret = SSL_read(cSSL,buffer,BUFLEN); 
	if (ret <= 0){
		printf("%d",SSL_get_error(cSSL,ret));
		error("Read failed");
		exit(1);
	}
	//print request buffer
	printf("Request:\r\n%s\n", buffer);
		
	//add terminator to end of the request part of the buffer as we are only interested in this (and not the headers)
	for(i=4;i<BUFLEN;i++) {
		if(buffer[i] == '/'){
			for(j=i;j<BUFLEN;j++){
				if(buffer[j] == ' ') { 
					buffer[j] = 0;
					break;
				}
			}
			break;
		}
	}
	
	//printf("buffer after 0's:\r\n%s\r\nEND\r\n",buffer);
	
	//null uri handling
	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) 
		(void)strcpy(buffer,"GET /index.html\0");
	if( !strncmp(&buffer[0],"HEAD /\0",7) || !strncmp(&buffer[0],"head /\0",7) ) 
		(void)strcpy(buffer,"HEAD /index.html\0");
		
	//GET requests
	if( !strncmp(&buffer[0],"GET /",5) || !strncmp(&buffer[0],"get /",5) ){ 
		buflen=strlen(buffer);
		fstr = (char *)0;
		for(i=0;extensions[i].ext != 0;i++) {
			len = strlen(extensions[i].ext);
			if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
				fstr = extensions[i].filetype;
				break;
			}
		}
	
		//see if file is available
		if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) 
			error("File not supported");

		//send response
		int length = get_file_size(file_fd);
		(void)sprintf(buffer,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
		(void)write(fd,buffer,strlen(buffer));
	
		//print response buffer
		printf("Response:\r\n%s\n", buffer);
		
		//write contents of the requested file
		while (	(ret = read(file_fd, buffer, BUFLEN)) > 0 ) {
			(void)SSL_write(cSSL,buffer,ret);
		}
	}
	//HEAD requests
	else if ( !strncmp(&buffer[0], "HEAD /", 6) || !strncmp(&buffer[0], "head /", 6) ){
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
		if(( file_fd = open(&buffer[6],O_RDONLY)) == -1) 
			error("File not supported");

		//send response
		int length = get_file_size(file_fd);
		(void)sprintf(buffer,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent Length: %d\r\n", fstr,length);
		(void)SSL_write(cSSL, buffer, strlen(buffer));
	
		//print response buffer
		printf("Response:\r\n%s\n", buffer);
	}
	//not supported html request
	else{
		SSL_write(cSSL, "HTTP/1.1 501 Not Implemented\r\n", 29);
		printf("Recieved bad request or not implemented\n");
	}
	
	//close connection
	int sd = SSL_get_fd(cSSL);
	SSL_free(cSSL);
	close(sd);
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
