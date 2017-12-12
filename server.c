/*
    Irbene meteo socket server, handles multiple clients using threads
*/
 
#include <stdio.h>
#include <string.h>   	//strlen
#include <stdlib.h>   	//strlen
#include <sys/socket.h>
#include <arpa/inet.h> 	//inet_addr
#include <unistd.h>    	//write
#include <pthread.h> 	//for threading , link with lpthread
#include <signal.h>		//for catching ctrl+c
#include <fcntl.h>		//for unblocking socket
#include <errno.h>		//error names

#include "defines.h"
#include "handlers.h"
#include "com.h"
#include "list.h"

//flag to keep main loop running
int keepRunning = 1;
//Thread array index
short int clientCount = 0;

int fd_set_blocking(int fd, int blocking) {
    /* Save the current flags */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return 0;

    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) != -1;
}

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);

    if (socket_desc == -1)
    {
        perror("could not create socket");
		return -1;
    }
    printf("Socket created\n");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8887);
	
	//Set socket reusable
	if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");
	
	struct timeval tv;
	tv.tv_sec = 1;  /* 20 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	//set read socket timeout as 10 s
	setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	//send write to socket timeout as 10 s
	setsockopt(socket_desc, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
     
	//Set socket as nonblocking
	fd_set_blocking(socket_desc, 0);
	
    //Bind
    if (bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
        //print the error message
        perror("bind failed. Error");
        return -2;
    }
    printf("Bind done\n");
   
    //Listen
    if (listen(socket_desc , 3) != 0) {
		perror("listen failed\n");
		return -3;
	}
     
    //Accept and incoming connection
    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);
	
	pthread_t serverThread;
	pthread_t comThread;

	//Attach handler for SIGINT 
	signal(SIGINT, intHandler);
	signal(SIGPIPE ,pipeHandler);
	
	if (pthread_create( &comThread , NULL ,  comHandler , NULL) < 0) {
			perror("could not create thread");
			return -4;
	}
	
	if (pthread_create( &serverThread , NULL ,  cientHandler , NULL) < 0) {
			perror("could not create thread");
			return -4;
	}
		
	while(keepRunning) {
		sleep(1);
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		//Catch errno if any
		int errsv = errno;
		if (client_sock < 0) {
			//if no client - continue
			if (errsv == EAGAIN || errsv == EWOULDBLOCK) {
				continue;
			}
			//if another unexpected error - break
			else if (errsv < 0){
				perror("accept failed");
				break;
			}
		}
		//get IP of client
		struct sockaddr_in addr;
		socklen_t addr_size = sizeof(struct sockaddr_in);
		int res = getpeername(client_sock, (struct sockaddr *)&addr, &addr_size);
		char *clientip = malloc(sizeof(char)*20);
		if (clientip != NULL) {
			strcpy(clientip, inet_ntoa(addr.sin_addr));
			printf("Client IP is %s\n", clientip);
			free(clientip);
		} else
			perror("malloc at clientip");
		
		//Check if MAX_CLIENTS limit will be reached
		if (clientCount + 1 > MAX_CLIENTS) {
			printf("No handler assigned as max connection count (%d) reached\n", MAX_CLIENTS);
			//Do not forget to close acepted socket on leave!
			close(client_sock);
			continue;
		}
		
		if (setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed at client handler");
		struct timeval tv;
		tv.tv_sec = 120;  /* 120 Secs Timeout */
		tv.tv_usec = 0;  // Not init'ing this can cause strange errors
		//set read socket timeout as 120 s
		setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
		//send write to socket timeout as 120 s
		setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
	
		printf("Connection accepted\n");
		 
		insertFirst(client_sock);
		printList();

		printf("Handler No. %d with fd %d assigned\n", clientCount, client_sock);
		clientCount++;
	}
	
	//Wait till all threads are stopped
    int i;
	printf("Waiting server thread to stop\n", i);
	if (pthread_join(serverThread , NULL) != 0)
			perror("error on joining thread");
	
	deleteList();
	printList();
	pthread_kill(comThread, SIGSEGV);
	close(socket_desc);
    return 0;
}
