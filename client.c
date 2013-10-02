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

static int connect_to(char *ip_address,char *port){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    printf("attempting to connect_to %s:%s\n",ip_address,port);

    if (getaddrinfo(ip_address, port, &hints, &servinfo) != 0) {
        return -1;
    }
    for (p = servinfo; p != NULL ; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            printf("error in client creating socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            printf("error in client connect");
            continue;
        }
        break;
    }

    if (p == NULL ) {
        printf( "client: failed to connect\n");
        exit(-1);
    }

    printf("client: connected successfully to %s:%s(Ignore previous errors for this)\n",s,port);
    freeaddrinfo(servinfo);
    return sockfd;
}


void client_fun(int manager_port_no){
    int pid = getpid(), nonce;
    printf("client pid is %d: should connect to %d\n", pid ,manager_port_no);
    char manager_port_no_str[MSG_SIZE];
    sprintf(manager_port_no_str,"%d",manager_port_no);
    int sockfd = connect_to("localhost",manager_port_no_str);

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