#include<stdio.h>
#include<stdlib.h>
#include<regex.h>
#include<unistd.h>
#include "manager.h"
#include "client.h"

static void read_next_line(FILE *file, char *line){
    regex_t comments_line;
    regcomp(&comments_line, "#.*", REG_EXTENDED);

    do{
    //todo - fix 1024
        fgets (line,1024, file );
    }while(!regexec(&comments_line, line, 0, NULL, 0));
}

static int read_int_from_line_format(char *line, char* line_prefix){
    int stage_num;
    char regex[1024];
    sprintf(regex,"%s .*",line_prefix);

    char line_scanner[1024];
    sprintf(line_scanner,"%s %s",line_prefix, "%d");

    regex_t stage_line;
    regcomp(&stage_line, regex, REG_EXTENDED);

    if(regexec(&stage_line, line, 0, NULL, 0)){
            fprintf(stderr,"Expected to find stage line, but fount %s, exiting\n",line);
            return -1;
    };

    sscanf(line, line_scanner, &stage_num);
    return stage_num;
}

static void read_config(char *file_name, config *c){
    FILE* file = fopen(file_name, "r");
    if(file==NULL){
        fprintf(stderr,"Error: %s not found, exiting now\n",file_name);
        fclose(file);
        exit(1);
    }

    char line[1025];

    read_next_line(file, line);
    c->stage_num = read_int_from_line_format(line, "stage");

    read_next_line(file, line);
    c->num_nodes = read_int_from_line_format(line, "num_nodes");

    read_next_line(file, line);
    c->nonce = read_int_from_line_format(line, "nonce");

    fclose(file);
}

static int fork_children(config c, int manager_port_no, int manager_sock_fd){
    int i=0;
    pid_t pid;
    for(i=0;i<c.num_nodes;i++){
         pid = fork();
        if (pid < 0)
            fprintf(stderr, "Error creating a new child process\n");
        else if (pid == 0) {
            close(manager_sock_fd);
            client_fun(i, manager_port_no);
            return pid;
        }
        else if (pid > 0) {}
    }
    return pid;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

//todo: clean up this code!
static int find_port(int *sock_desc){

	struct addrinfo hints, *servinfo, *p;
		int rv,addrlen;
		int socket_descriptor=0,portno;
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = INADDR_ANY;
		struct sockaddr_in serv_addr;
	int input_portno = 0;
	char portnoString[10];
		sprintf(portnoString,"%i", input_portno);

		if ((rv = getaddrinfo(NULL, portnoString, &hints, &servinfo)) != 0) {
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
				//return 1;
		}

		// loop through all the results and bind to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((socket_descriptor = socket(p->ai_family, p->ai_socktype,
					p->ai_protocol)) == -1) {
					perror("listener: socket");
					continue;
			}
			if (bind(socket_descriptor, p->ai_addr, p->ai_addrlen) == -1) {
				close(socket_descriptor);
				perror("listener: bind");
				continue;
			}
			break;
		}
		if (p == NULL) {
			fprintf(stderr, "listener: failed to bind socket\n");
			//return 2;
		}
		addrlen = sizeof(serv_addr);
		int getsock_check=getsockname(socket_descriptor,(struct sockaddr *)&serv_addr, (socklen_t *)&addrlen) ;

		   	if (getsock_check== -1) {
		   			perror("getsockname");
		   			exit(1);
		   	}
		portno =  ntohs(serv_addr.sin_port);
		freeaddrinfo(servinfo);
		*sock_desc=socket_descriptor;
		return portno;
}

int main(int argv, char **argc){
    if(argv==1){
        printf("Usage: main <config_file_path>\n");
        return -1;
    }
    int pid;
    config c;
    read_config(argc[1], &c);
    int manager_sock_fd;
    int manager_port_no = find_port(&manager_sock_fd);

    pid = fork_children(c, manager_port_no, manager_sock_fd);

    // child process, exit.
    if(pid==0){
        return 0;
    }
    char filename[1024];
    sprintf(filename, "stage%d.manager.out",c.stage_num);
	FILE *log_file = fopen(filename, "w");
    printf("manager port: %d\n", manager_port_no);
    fprintf(log_file,"manager port: %d\n", manager_port_no);

    manager_fun(c, manager_sock_fd, log_file);

    fclose(log_file);
    close(manager_sock_fd);

    return 1;
}