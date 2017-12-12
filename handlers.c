#include "handlers.h"
#include <stdio.h>
#include <stdlib.h>   	
#include <fcntl.h>		//for unblocking socket
#include <sys/socket.h>
#include <errno.h>

#include "list.h"
#include "com.h"

int keepRunning;
short int ServerThreadsIndex;
char wxBuff[wxBuffSize];
int newData;
short int clientCount;

/*
 * This will handle connection for each client
 * */
void *cientHandler()
{
    int read_size;
    char *message , client_message[2000];
     
	int status, errsv;
	struct node *ptr;
	
    //Receive a message from client
    while(keepRunning) {
		sleep(1);
		//If no new messages - skip
		if (!newData) 
			continue;
        //Send the message to clients
		ptr = head;	
		//start from the beginning
		while(ptr != NULL) {
			//printf("write to socket %d %d bytes\n", ptr->fd, newData);
			status = write(ptr->fd , wxBuff , newData);
			errsv = errno;
			if (status < 0) {
					printf("write to socket %d returned %d, errno is %d\n", ptr->fd, status, errsv);
					perror("write to socket");
					close(ptr->fd);
					if (delete(ptr->fd) == NULL)
						continue;
					else
						if (clientCount > 0) {
							clientCount--;
							printf("Now client count is %d\n", clientCount);
						}
			}
			ptr = ptr->next;
		}
		
		//Wait till all threads receive the message
		newData = 0;
    }
     
	if (keepRunning == 0) 
		printf("Forced stop of thread\n");
 
    //Free the socket pointer
	//close(sock);
    //free(socket_desc);
	
    
	printf("End of thread\n");
    return 0;
}

void intHandler(int dummy) {
	printf("Server will be stopped\n");
    keepRunning = 0;
}

void pipeHandler(int dummy) {
	return;
}
