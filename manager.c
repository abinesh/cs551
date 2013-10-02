#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include "common.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define TOTAL_NO_OF_PENDING_CONNECTIONS 2048

static int receive_connection_from_client(int server_sock_fd, int *new_fd){
    struct sockaddr_in their_addr; // connector's address information
    socklen_t sin_size;

    sin_size = sizeof their_addr;
    *new_fd = accept(server_sock_fd, (struct sockaddr *) &their_addr, &sin_size);
    if (*new_fd == -1) {
        perror("accept");
        exit(-1);
    }

    int portno = ntohs(their_addr.sin_port);
    printf("client port no in manager: %d\n", portno);
    return portno;
}

void manager_fun(config c, int manager_sock_fd, FILE *log_file){
	int new_fd;

	if (listen(manager_sock_fd, TOTAL_NO_OF_PENDING_CONNECTIONS) == -1) {
		perror("listen");
		exit(1);
	}

	int client_id = 1;
	while (client_id <= c.num_nodes) { // main accept() loop
	    fprintf(stderr, "waiting for connection no %d of %d\n", client_id, c.num_nodes);
	    int client_port_no = receive_connection_from_client(manager_sock_fd, &new_fd);
	    fprintf(stderr, "received connection from client\n");
	    fprintf(log_file, "client %d port: %d\n", client_id, client_port_no);

        char buf[MSG_SIZE];
        memset(buf, 0, MSG_SIZE);
        sprintf(buf, "%d\n", c.nonce);
        printf("sending to client no %d: %s\n", client_id, buf);
        send(new_fd, buf, strlen(buf), 0);
        memset(buf, 0, MSG_SIZE);
        recv(new_fd, buf, MSG_SIZE, 0);
        printf("received from client %d: %s\n", client_id, buf);
        fprintf(log_file, "client %d says: %s\n", client_id, buf);

	    close(new_fd);
	    client_id++;
    }
    printf("manager fun exiting\n");
}