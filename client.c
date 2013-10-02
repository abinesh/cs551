#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include "common.h"

#include <netdb.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// get sockaddr, IPv4 or IPv6:
//todo: remove duplication, possibly create new header file and move common functions
static void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

static int connect_to(char *ip_address,char *port,char *caller){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    fprintf(stderr,"In %s: attempting to connect_to %s:%s\n",caller,ip_address,port);

    if ((rv = getaddrinfo(ip_address, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }
    for (p = servinfo; p != NULL ; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            fprintf(stderr,"In %s,",caller);
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
//            close(sockfd);
            fprintf(stderr,"In %s,",caller);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL ) {
        fprintf(stderr, "In %s : client: failed to connect\n",caller);
        exit(-1);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);

    struct sockaddr_in serv_addr;
    int addrlen = sizeof(serv_addr);

    getsockname(sockfd,(struct sockaddr *)&serv_addr, (socklen_t *)&addrlen);

    int portno =  ntohs(serv_addr.sin_port);
    printf("client port no: %d\n", portno);

    fprintf(stderr,"In %s : client: connected successfully to %s:%s(Ignore previous errors for this)\n", caller,s,port);
    freeaddrinfo(servinfo);
    return sockfd;
}


void client_fun(int id, int manager_port_no){
    int pid = getpid(), nonce;
    printf("client pid is %d: should connect to %d\n", pid ,manager_port_no);
    char manager_port_no_str[MSG_SIZE];
    sprintf(manager_port_no_str,"%d",manager_port_no);
    int sockfd = connect_to("localhost",manager_port_no_str,"client_fun");

    char buf[MSG_SIZE];
    memset(buf, 0, MSG_SIZE);

    recv(sockfd, buf, MSG_SIZE, 0);
    printf("client received %s\n", buf);
    sscanf(buf,"%d", &nonce);
    memset(buf, 0, MSG_SIZE);
    sprintf(buf, "%d %d", nonce+pid,pid);
    send(sockfd, buf, strlen(buf), 0);
    printf("client sent: %s\n", buf);

    close(sockfd);
    exit(0);
}