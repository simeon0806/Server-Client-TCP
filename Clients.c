#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT (8080)
#define IP_ADDRES ("127.0.0.1")
#define MSG_SIZE (64)
#define NAME_LEN (32)
#define END_MSG ("exit")
#define WORKERS_NUM (6)

void* workerFunc(void* arg);
int end(char* mesg,int *sockfd);

int main()
{
	pthread_t worker[WORKERS_NUM];
	int clientId[WORKERS_NUM];
	int i;

	for(i=0;i<WORKERS_NUM;i++){
		clientId[i] = i+1;
		if(pthread_create(&worker[i],NULL,workerFunc,(void*)&clientId[i])!=0)
		{
			printf("pthread_create failed...\n");
			exit(1);
		}
	}

	for(i=0;i<WORKERS_NUM;i++){
		if(pthread_join(worker[i],NULL)!=0)
		{
			printf("pthread_join failed...\n");
			exit(1);
		}
	}

	return 0;
}

void* workerFunc(void* arg)
{
	int id = *((int*)arg);
	char buff[MSG_SIZE];
	char serverMsg[MSG_SIZE];
	int sockfd;
	struct sockaddr_in servaddr;
	int value = 0;

	// Socket create
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(1);
	}
	
	bzero(&servaddr, sizeof(servaddr));

	// Assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// Connect the client socket to server socket
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(1);
	}	
	
	snprintf(buff, NAME_LEN, "Worker%d",id);

	while (1) {
		// Clear the buffer
		memset(&value, 0, sizeof(value));
		
		// Prepare the money that should be taken
		value = 0 - ((rand() % 100) + 100);
		
		// Clear the buffer field(the bytes)
		memset((buff + NAME_LEN), 0, NAME_LEN);
	
		snprintf((buff + NAME_LEN), NAME_LEN, "%d", value);

		// Send to server how much money to be taken from cash
		write(sockfd, buff, sizeof(buff));

		memset(serverMsg,0,MSG_SIZE);

		read(sockfd, serverMsg, sizeof(buff));

		printf("%s-%d\n",serverMsg,id);

		if(end(serverMsg,&sockfd))
		{
			printf("clsed - %d\n",id);
			break;
		}
		
		// Sleep from 0 to 5 seconds
		sleep(rand() % 5);
		
		// Clear the buffer
		memset((buff + NAME_LEN), 0, NAME_LEN);
		
		// Prepare the money that should be returned
		value = value * (-1);
		
		snprintf((buff + NAME_LEN), NAME_LEN, "%d", value);
				
		// Send to server how much money to be returned to cash
		write(sockfd, buff, sizeof(buff));

		memset(serverMsg,0,MSG_SIZE); 

		read(sockfd, serverMsg, sizeof(buff));

		printf("%s-%d\n",serverMsg,id);

		if(end(serverMsg,&sockfd))
		{
			printf("clsed - %d\n",id);
			break;
		}
		
		// Sleep from 0 to 5 seconds
		sleep(rand() % 5);
	}
}

int end(char* mesg,int *sockfd)
{
	if(strcmp(mesg,END_MSG)==0){
			// Close the socket
			close(*sockfd);
			return 1;
	}
	return 0;
}