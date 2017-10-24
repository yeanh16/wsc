#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

int listen_socket (const int s){
	int client;
	struct sockaddr_in6 their_address;
	socklen_t their_address_size = sizeof (their_address)
	char buffer[INET6_ADDRSTRLEN + 32];
	char *printable;
}

void construct_server_socket (){
	const char* hostname =0;/* wildcard */
	const char* portname="daytime";
	struct addrinfo hints;
	memset (&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=0;
	hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;
	struct addrinfo * res=0;
	int err=getaddrinfo(hostname,portname,&hints,&res);
	if (err!=0){
		die("failed to resolve local socket address (err=%d)",err)
	}
}

void create_server_socket(){
	int (server_fd==-1) {
		die("%s",strerror(errno));
	}
}




