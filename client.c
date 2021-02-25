#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <semaphore.h>

sem_t cont;
int control = 0;
int clientSocket; 
void *Receive(void * sockID){
	int clientSocket = *((int *) sockID);
	while(1){
		char rec[20000];
		//Message is received from the server
		int final = recv(clientSocket,rec,20000,0);
		rec[final] = '\0';
		printf("%s\n",rec);
		//If this message is received, the -exit command has been entered and socket is free.
		if(strcmp(rec,"You exit the program...\n")==0){
			control = 1;
			sem_post(&cont);
			break;
		}
		//If the exit command is entered but is in a room, the user is not confirmed to exit.
		else if(strcmp(rec,"You are in the room, so you cannot exit the system...\n")==0){
			sem_post(&cont);
		}
	}
	
}
int main(){
	// the cont semaphore is a control mechanism maintained for output
	sem_init(&cont,0,0);
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(3205);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	printf("Connection established ............\n");

	pthread_t thread;
	pthread_create(&thread, NULL, Receive, (void *) &clientSocket );

	while(1){
		char input[100];
		gets(input);
		send(clientSocket,input,100,0);
		if(strcmp(input,"-exit")==0){
			sem_wait(&cont);
		}
        // if the control variable is 1, the exit is successfully achieved.
		if(control == 1){
			pthread_join(thread,NULL);
			break;
		}
	}
	return 0;
}